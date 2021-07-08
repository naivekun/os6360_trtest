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
* @file cpssDxChPortCtrlUT.c
*
* @brief Unit tests for cpssDxChPortCtrl, that provides
* CPSS implementation for Port configuration and control facility.
*
* @version   137
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChGEMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChXLGMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrlUT.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

/* defines */

/* Default valid value for port id */
#define PORT_CTRL_VALID_PHY_PORT_CNS  0

/* Default valid value for ipg */
#define PORT_IPG_VALID_CNS  8

/* EOM random parameters*/
#define PORT_EOM_TOTAL_PORTS_PER_CORE 12
#define PORT_EOM_TOTAL_PORTS_PER_CORE_TO_CHECK 2

/* jump 19 ports to reduce runtime --
   do not reduce !!! it hide bugs in the code when not all ports tested !!! */
#define PORT_STEP_FOR_REDUCED_TIME_CNS  0

/* string for GM fails */
#define GM_PORTCTRL_GM_FAIL_STR     "CQ 153218: GM BC2 B0 mainUT crashes \n"

/* Save active ports parameters */
GT_STATUS prvCpssDxChSaveActivePortsParam
(
    GT_U8                                          dev,
    HDR_PORT_NUM_SPEED_INTERFACE_STC              *portActivePtr
)
{
    GT_STATUS                                            st;
    static HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    portShadow;

    /* Saving default mapping*/
    st = prvPortShadowMapGet( dev, /*OUT*/&portShadow);
    if(GT_NOT_APPLICABLE_DEVICE == st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortShadowMapGet", dev);
    if(GT_OK != st)
    {
        return GT_FAIL;
    }

    /* Saving active ports*/
    st = prvDefaultPortsMappingAndActivePortsParamSaving(dev, &portShadow, /*OUT*/portActivePtr);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvDefaultPortsMappingAndActivePortsParamSaving", dev);
    if(GT_OK != st)
    {
         return GT_FAIL;
    }

    return GT_OK;
}

/* this function purposed to tell if port has GE MAC */
static GT_STATUS prvDxChPortCtrlUtGeSupportedGet
(
    GT_U8                   dev,
    GT_PHYSICAL_PORT_NUM    port,
    GT_BOOL                 *geSupportedPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    CPSS_NULL_PTR_CHECK_MAC(geSupportedPtr);

    if(CPSS_PP_FAMILY_DXCH_XCAT_E == PRV_CPSS_PP_MAC(dev)->devFamily)
    {/* although on some XCAT devices flex links defined as XG_HX_QX
        etc., still physically GE MAC exists and to prevent changes in XCAT
        init let's return here "true" */
        *geSupportedPtr = GT_TRUE;
        return GT_OK;
    }

    rc = prvUtfPortTypeOptionsGet(dev, port, &portTypeOptions);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(portTypeOptions)
    {
        case PRV_CPSS_XG_PORT_XG_ONLY_E:
        case PRV_CPSS_XG_PORT_HX_QX_ONLY_E:
        case PRV_CPSS_XG_PORT_XG_HX_QX_E:
            *geSupportedPtr = GT_FALSE;
            break;
        default:
            *geSupportedPtr = GT_TRUE;
            break;
    }

    return GT_OK;
}

static GT_U32 lanesNumInDevGet(IN GT_U8 devNum)
{
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
/*
    NOTE: why the Aldrin ,AC3X 'not supported' ?
          I don't know , but if added , cause to fail of UT : cpssDxChPortModeSpeedSet
          when running (all UTs) : utfTestsTypeRun 1,0,1,2,4,5,0
*/
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            utfFailureMsgLog("not supported devFamily\n", NULL, 0);
            return 0;
        default:
            break;
    }

    return prvCpssDxChHwInitNumOfSerdesGet(devNum);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacSaLsbSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_U8    macSaLsb
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacSaLsbSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with macSaLsb [0 / 250].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortMacSaLsbGet with non-NULL macSaLsb.
    Expected: GT_OK and macSaLsb the same as just written.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port        = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     isRemotePort;
    GT_U8       macSaLsb    = 0;
    GT_U8       macSaLsbGet = 0;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);
            /* 1.1.1. Call with macSaLsb [0 / 250].
               Expected: GT_OK.
            */
            /* iterate with macSaLsb = 0 */
            macSaLsb = 0;

            st = cpssDxChPortMacSaLsbSet(dev, port, macSaLsb);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, macSaLsb);

                /*
                    1.1.2. Call cpssDxChPortMacSaLsbGet with non-NULL macSaLsb.
                    Expected: GT_OK and macSaLsb the same as just written.
                */
                st = cpssDxChPortMacSaLsbGet(dev, port, &macSaLsbGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortMacSaLsbGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(macSaLsb, macSaLsbGet,
                                             "got another macSaLsb as was written: %d, %d", dev, port);
            }

            /* iterate with macSaLsb = 250 */
            macSaLsb = 250;

            st = cpssDxChPortMacSaLsbSet(dev, port, macSaLsb);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, macSaLsb);
            }

            /*
                1.1.2. Call cpssDxChPortMacSaLsbGet with non-NULL macSaLsb.
                Expected: GT_OK and macSaLsb the same as just written.
            */
            st = cpssDxChPortMacSaLsbGet(dev, port, &macSaLsbGet);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortMacSaLsbGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(macSaLsb, macSaLsbGet,
                                             "got another macSaLsb as was written: %d, %d", dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        macSaLsb = 0;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacSaLsbSet(dev, port, macSaLsb);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* macSaLsb == 0 */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacSaLsbSet(dev, port, macSaLsb);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    macSaLsb = 0;
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacSaLsbSet(dev, port, macSaLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssDxChPortMacSaLsbSet(dev, port, macSaLsb);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacSaLsbGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    OUT GT_U8    *macSaLsb
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacSaLsbGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-NULL macSaLsb.
    Expected: GT_OK.
    1.1.2. Call with macSaLsb [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port        = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U8       macSaLsb    = 0;
    GT_BOOL     isRemotePort;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);

            /*
                1.1.1. Call with non-NULL macSaLsb.
                Expected: GT_OK.
            */
            st = cpssDxChPortMacSaLsbGet(dev, port, &macSaLsb);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with macSaLsb [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortMacSaLsbGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, macSaLsbPtr = NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacSaLsbGet(dev, port, &macSaLsb);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacSaLsbGet(dev, port, &macSaLsb);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacSaLsbGet(dev, port, &macSaLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacSaLsbGet(dev, port, &macSaLsb);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with state [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    1.1.2. Call cpssDxChPortEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChPortEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. Call cpssDxChPortEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "[cpssDxChPortEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                         "get another enable value than was set: %d, %d", dev, port);

            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChPortEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. Call cpssDxChPortEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "[cpssDxChPortEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                         "get another enable value than was set: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssDxChPortEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortEnableSet(dev, port, enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEnableGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
    */

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            /* 1.1.1. Call with non-null statePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortEnableGet(dev, port, &state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortDuplexModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPortDuplexModeSet) */
GT_VOID cpssDxChPortDuplexModeSetUT(GT_VOID)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with dMode [CPSS_PORT_FULL_DUPLEX_E].
    Expected: GT_OK.
    1.1.2. Call with dMode [CPSS_PORT_HALF_DUPLEX_E].
    Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED for 10Gbps ports.
    1.1.3. Call with wrong enum values dMode
    Expected: GT_BAD_PARAM
    */

    GT_STATUS            st   = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_DUPLEX_ENT mode = CPSS_PORT_FULL_DUPLEX_E;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    GT_BOOL duplexSupported;
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

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
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = prvDxChPortCtrlUtGeSupportedGet(dev,port,&duplexSupported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call with dMode [CPSS_PORT_FULL_DUPLEX_E].
               Expected: GT_OK.
            */

            /* Call with mode [CPSS_PORT_FULL_DUPLEX_E] */
            mode = CPSS_PORT_FULL_DUPLEX_E;

            st = cpssDxChPortDuplexModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC((duplexSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                            st, dev, port, mode);

            /* 1.1.2. Call with dMode [CPSS_PORT_HALF_DUPLEX_E].
               Expected: GT_OK for non 10Gpbs ports and GT_NOT_SUPPORTED otherwise.
            */

            /* Call with mode [CPSS_PORT_HALF_DUPLEX_E] */
            mode = CPSS_PORT_HALF_DUPLEX_E;

            st = cpssDxChPortDuplexModeSet(dev, port, mode);
            if((PRV_CPSS_PORT_XG_E > portType)
                && !(  (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
                     ||(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                     ||(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(dev))
                    )
              )
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, mode);
            }

            /*
               1.1.3. Call with wrong enum values dMode
               Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortDuplexModeSet
                                (dev, port, mode),
                                mode);
        }

        mode = CPSS_PORT_FULL_DUPLEX_E;

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* dMode [CPSS_PORT_FULL_DUPLEX_E]               */
            st = cpssDxChPortDuplexModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* dMode == CPSS_PORT_FULL_DUPLEX_E */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDuplexModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* dMode == CPSS_PORT_FULL_DUPLEX_E */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDuplexModeSet(dev, port, mode);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mode = CPSS_PORT_FULL_DUPLEX_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDuplexModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, dMode == CPSS_PORT_FULL_DUPLEX_E */

    st = cpssDxChPortDuplexModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortDuplexModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortDuplexModeGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with non-null dModePtr.
    Expected: GT_OK.
    1.1.2. Call with dModePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS            st   = GT_OK;

    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_DUPLEX_ENT mode;

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
            /* 1.1.1. Call with non-null dModePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortDuplexModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with dModePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortDuplexModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }


        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDuplexModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDuplexModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDuplexModeGet(dev, port, &mode);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDuplexModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDuplexModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_U8                portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSpeedSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with speed [CPSS_PORT_SPEED_1000_E].
    Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED for non-10Gbps ports.
    1.1.2. Call with speed [CPSS_PORT_SPEED_10_E, CPSS_PORT_SPEED_100_E,CPSS_PORT_SPEED_1000_E,].
    Expected: GT_OK for non -10Gbps ports and GT_NOT_SUPPORTED for 10Gbps ports.
    1.1.3. Call with wrong enum values speed.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_SPEED_ENT    speed    = CPSS_PORT_SPEED_10_E;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_BOOL failureExpected;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode)
            {
                continue;
            }

            /* 1.1.1. Call with speed [CPSS_PORT_SPEED_10000_E].
               Expected: GT_OK for ports where XG or XLG(for Lion2/3) interface modes defined
            */
            speed = CPSS_PORT_SPEED_10000_E;

            st = cpssDxChPortSpeedSet(dev, port, speed);

            failureExpected = GT_TRUE;
            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                if(PRV_CPSS_PORT_XG_E == portType)
                {
                    failureExpected = GT_FALSE;
                }
                if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
                     (PRV_CPSS_SIP_5_CHECK_MAC(dev)))
                {
                    if(PRV_CPSS_PORT_XLG_E == portType)
                        failureExpected = GT_FALSE;
                }
            }

            if(GT_TRUE == failureExpected)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }

            /* 1.1.2. Call with speed [CPSS_PORT_SPEED_10_E /
                                       CPSS_PORT_SPEED_100_E /
                                       CPSS_PORT_SPEED_1000_E].
               Expected: GT_OK for non-10Gbps ports and NON GT_OK for 10Gbps ports.
            */

            /* speed = CPSS_PORT_SPEED_10_E */
            speed = CPSS_PORT_SPEED_10_E;

            st = cpssDxChPortSpeedSet(dev, port, speed);
            /* at the moment 1000baseX is the only ifMode of GE type which doesn't allow 10M speed */
            if((PRV_CPSS_PORT_XG_E <= portType) || (CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }

            /* speed = CPSS_PORT_SPEED_100_E */
            speed = CPSS_PORT_SPEED_100_E;

            st = cpssDxChPortSpeedSet(dev, port, speed);
            /* at the moment 1000baseX is the only ifMode of GE type which doesn't allow 100M speed */
            if((PRV_CPSS_PORT_XG_E <= portType) || (CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode))
            {
                 UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }

            /* speed = CPSS_PORT_SPEED_1000_E */
            speed = CPSS_PORT_SPEED_1000_E;

            st = cpssDxChPortSpeedSet(dev, port, speed);
            if(PRV_CPSS_PORT_GE_E != portType)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
            }

            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
            {
                /* speed = CPSS_PORT_SPEED_16000_E */
                speed = CPSS_PORT_SPEED_16000_E;

                st = cpssDxChPortSpeedSet(dev, port, speed);
                if(PRV_CPSS_PORT_XG_E == portType)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, speed);
                }
            }

            /*
               1.1.3. Call with wrong enum values speed
               Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortSpeedSet
                                (dev, port, speed),
                                speed);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        speed = CPSS_PORT_SPEED_1000_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* speed = CPSS_PORT_SPEED_1000_E                */
            st = cpssDxChPortSpeedSet(dev, port, speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* speed == CPSS_PORT_SPEED_1000_E */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSpeedSet(dev, port, speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* speed == CPSS_PORT_SPEED_1000_E */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSpeedSet(dev, port, speed);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, speed);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    speed = CPSS_PORT_SPEED_1000_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSpeedSet(dev, port, speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, speed == CPSS_PORT_SPEED_1000_E */

    st = cpssDxChPortSpeedSet(dev, port, speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSpeedGet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with non-null speedPtr.
    Expected: GT_OK.
    1.1.2. Call with speedPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS           st    = GT_OK;

    GT_U8               dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null speedPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with speedPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSpeedGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSpeedGet(dev, port, &speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSpeedGet(dev, port, &speed);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSpeedGet(dev, port, &speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSpeedGet(dev, port, &speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortDuplexAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortDuplexAutoNegEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with state [GT_FALSE].
        Expected: GT_OK.
        1.1.2. Call with state [GT_TRUE].
        Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
        1.1.3. Call cpssDxChPortDuplexAutoNegEnableGet with not null state.
        Expected: GT_OK and the same value.
    */

    GT_U8                  dev;
    GT_STATUS              st       = GT_OK;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    GT_BOOL                stateGet = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    GT_BOOL                geSupported;

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
            st = prvDxChPortCtrlUtGeSupportedGet(dev,port,&geSupported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call with state [GT_FALSE].
               Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                            st, dev, port, state);

            /* 1.1.2. Call with state [GT_TRUE].
               Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_TRUE;

            st = prvUtfPortTypeGet(dev, port, &portType);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);

            if((PRV_CPSS_PORT_XG_E > portType) &&
                (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E) &&
                (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                            st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }

            /*
                1.1.3. Call cpssDxChPortDuplexAutoNegEnableGet with not null state.
                Expected: GT_OK and the same value.
            */
            /*Call with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                            st, dev, port, state);

            if(geSupported)
            {
                st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, &stateGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
                UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                         "got another state then was set: %d, %d", dev, port);
            }

            /*Call with state [GT_TRUE] */
            state = GT_TRUE;

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);

            if((PRV_CPSS_PORT_XG_E > portType) &&
                (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E) &&
                (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                            st, dev, port, state);
                if(geSupported)
                {
                    st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, &stateGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
                    UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                             "got another state then was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }
        }

        state = GT_TRUE;

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state = GT_TRUE  */
            st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortDuplexAutoNegEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   pauseAdvertise
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortFlowCntrlAutoNegEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with state [GT_FALSE] and pauseAdvertise [GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call with state [GT_TRUE] and pauseAdvertise [GT_FALSE and GT_TRUE].
    Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
    */

    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    GT_BOOL                pause    = GT_FALSE;
    GT_BOOL                stateGet = GT_FALSE;
    GT_BOOL                pauseGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    GT_BOOL                 geSupported;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvDxChPortCtrlUtGeSupportedGet(dev,port,&geSupported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call with state [GT_FALSE], pauseAdvertise [GT_TRUE].
               Expected: GT_OK and the same values.
            */
            state = GT_FALSE;
            pause = GT_FALSE;

            st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            UTF_VERIFY_EQUAL4_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                    st, dev, port, state, pause);
            if(geSupported)
            {
                /* checking values*/
                st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &stateGet, &pauseGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                         "got another state then was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(pause, pauseGet,
                         "got another pause then was set: %d, %d", dev, port);
            }

            /* Getting port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /*
               1.1.2. Call with state [GT_TRUE]
               pauseAdvertise [GT_FALSE and GT_TRUE].
               Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise..
            */

            /* Call function with pauseAdvertise = GT_FALSE] */
            state = GT_TRUE;
            pause = GT_FALSE;

            st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                    st, dev, port, state, pause);
                if(geSupported)
                {
                    /* checking values*/
                    st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &stateGet, &pauseGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                             "got another state then was set: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(pause, pauseGet,
                             "got another pause then was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state, pause);
            }

            /* Call function with pauseAdvertise = GT_TRUE] */
            pause = GT_TRUE;

            st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                    st, dev, port, state, pause);
                if(geSupported)
                {/* checking values*/
                    st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &stateGet, &pauseGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                             "got another state then was set: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(pause, pauseGet,
                             "got another pause then was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state, pause);
            }
        }

        state = GT_TRUE;
        pause = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    pause = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE, pause == GT_TRUE */

    st = cpssDxChPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSpeedAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSpeedAutoNegEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with state [GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call with state [GT_TRUE ].
    Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    GT_BOOL                 geSupported;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvDxChPortCtrlUtGeSupportedGet(dev,port,&geSupported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /*
                1.1.1. Call with state [GT_FALSE].
                Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssDxChPortSpeedAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                            st, dev, port, state);

            /* Getting port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /*
                1.1.2. Call with state [GT_TRUE ].
                Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_TRUE;

            st = cpssDxChPortSpeedAutoNegEnableSet(dev, port, state);
            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC((geSupported ? GT_OK : GT_NOT_SUPPORTED),
                                                                st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE     */
            st = cpssDxChPortSpeedAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSpeedAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* state == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSpeedAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSpeedAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortSpeedAutoNegEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFlowControlEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortFlowControlEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with state [CPSS_PORT_FLOW_CONTROL_DISABLE_E/
                            CPSS_PORT_FLOW_CONTROL_RX_TX_E  /
                            CPSS_PORT_FLOW_CONTROL_RX_ONLY_E/
                            CPSS_PORT_FLOW_CONTROL_TX_ONLY_E]
    Expected: GT_OK for XG ports and non GT_OK when not XG ports with
              last two values.
    1.1.2. Call cpssDxChPortFlowControlEnableGet with the same params.
    Expected: GT_OK and the same value.
    1.1.3. Call with wrong enum value state.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    GT_U8     dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PORT_FLOW_CONTROL_ENT state  = CPSS_PORT_FLOW_CONTROL_DISABLE_E;

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
                1.1.1. Call with state [CPSS_PORT_FLOW_CONTROL_DISABLE_E/
                                        CPSS_PORT_FLOW_CONTROL_RX_TX_E  /
                                        CPSS_PORT_FLOW_CONTROL_RX_ONLY_E/
                                        CPSS_PORT_FLOW_CONTROL_TX_ONLY_E]
                Expected: GT_OK for XG ports and non GT_OK when not XG ports with
                          last two values.
            */

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_DISABLE_E */
            state = CPSS_PORT_FLOW_CONTROL_DISABLE_E;

            st = cpssDxChPortFlowControlEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChPortFlowControlEnableGet returns actual
                Flow control status but not previouse settings of
                cpssDxChPortFlowControlEnableSet.
                The status depends on a lot of other configurations like
                flow control autonegatiation, type of PHY,
                PHY - MAC autonegatiation and so on.
                The status always disabled in simulation.
            */

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_DISABLE_E */
            state = CPSS_PORT_FLOW_CONTROL_RX_TX_E;

            st = cpssDxChPortFlowControlEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_RX_ONLY_E */
            state = CPSS_PORT_FLOW_CONTROL_RX_ONLY_E;

            st = cpssDxChPortFlowControlEnableSet(dev, port, state);

            if (IS_PORT_XG_E(portType)
                || PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)
                || PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_TX_ONLY_E */
            state = CPSS_PORT_FLOW_CONTROL_TX_ONLY_E;

            st = cpssDxChPortFlowControlEnableSet(dev, port, state);

            if (IS_PORT_XG_E(portType)
                || PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)
                || PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }

            state = CPSS_PORT_FLOW_CONTROL_DISABLE_E;

            /*
                1.1.3. Call with wrong enum value state.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortFlowControlEnableSet
                                (dev, port, state),
                                state);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available physical
           ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE */
            st = cpssDxChPortFlowControlEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortFlowControlEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* state == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortFlowControlEnableSet(dev, port, state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, state);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFlowControlEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortFlowControlEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFlowControlEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT  *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortFlowControlEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st    = GT_OK;

    GT_U8      dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PORT_FLOW_CONTROL_ENT  state;

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
               1.1.1. Call with non-null statePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortFlowControlEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortFlowControlEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortFlowControlEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortFlowControlEnableGet(dev, port, &state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFlowControlEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortFlowControlEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPeriodicFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPeriodicFcEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    */

    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT     enable    = GT_FALSE;
    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get type of port */
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChPortPeriodicFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortPeriodicFcEnableGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable value than was set: %d, %d", dev, port);

            /* Call function with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChPortPeriodicFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortPeriodicFcEnableGet: %d, %d", dev, port);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) != GT_TRUE)
            {
                /* Gig has only Xon (Xoff always enabled)*/
                if(portMacType < PRV_CPSS_PORT_XG_E)
                    enable = CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E;
            }

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable value than was set: %d, %d", dev, port);

        }

        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssDxChPortPeriodicFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPeriodicFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPeriodicFcEnableSet(dev, port, enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPeriodicFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortPeriodicFcEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortBackPressureEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortBackPressureEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
        1.1.1. Call with state [GT_FALSE and GT_TRUE].
        Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED otherwise.
        1.1.2. Call cpssDxChPortBackPressureEnableGet with not null state.
        Expected: GT_OK for non-10Gbps ports and the same value.
                  GT_NOT_SUPPORTED otherwise.
    */

    GT_STATUS              st       = GT_OK;
    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    GT_BOOL                stateGet = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;

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
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_FALSE;

            st = cpssDxChPortBackPressureEnableSet(dev, port, state);
            if((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PORT_XG_E == portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }

            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_TRUE;

            st = cpssDxChPortBackPressureEnableSet(dev, port, state);
            if((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PORT_XG_E == portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            /*
                1.1.2. Call cpssDxChPortBackPressureEnableGet with not null state.
                Expected: GT_OK for non-10Gbps ports and the same value.
                          GT_NOT_SUPPORTED otherwise.
            */
            state = GT_FALSE;
            st = cpssDxChPortBackPressureEnableSet(dev, port, state);
            if((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PORT_XG_E == portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }
            else
            {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*checking value*/
            st = cpssDxChPortBackPressureEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
             "cpssDxChPortBackPressureEnableGet: got another state then was set: %d, %d",
                                         dev, port);
        }
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available
            physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state = GT_TRUE  */
            st = cpssDxChPortBackPressureEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        /* state == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortBackPressureEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */

        /* state == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortBackPressureEnableSet(dev, port, state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, state);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortBackPressureEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortBackPressureEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortLinkStatusGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    OUT GT_BOOL   *isLinkUpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLinkStatusGet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with non-null isLinkUpPtr.
    Expected: GT_OK.
    1.1.2. Call with isLinkUpPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

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
            /* 1.1.1. Call with non-null isLinkUpPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortLinkStatusGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with isLinkUpPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortLinkStatusGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortLinkStatusGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortLinkStatusGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        CPSS_TBD_BOOKMARK_XCAT3
        if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            st = cpssDxChPortLinkStatusGet(dev, port, &state);
            if(!prvUtfIsCpuPortMacSupported(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLinkStatusGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLinkStatusGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInterfaceModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_U8                          portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPortInterfaceModeSet) */
GT_VOID cpssDxChPortInterfaceModeSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with ifMode[CPSS_PORT_INTERFACE_MODE_SGMII_E /
                            CPSS_PORT_INTERFACE_MODE_1000BASE_X_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortInterfaceModeGet with not-NULL ifModePtr.
    Expected: GT_OK and the same ifMode as was set.
    1.1.3. Call with ifMode [CPSS_PORT_INTERFACE_MODE_MII_E] (only for CPU port).
    Expected: NOT GT_OK.
    1.1.4. Call with wrong enum values ifMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;
    GT_BOOL isFlexLink;

    CPSS_PORT_INTERFACE_MODE_ENT ifMode    = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_INTERFACE_MODE_ENT ifModeGet = CPSS_PORT_INTERFACE_MODE_NA_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlexLink);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if ((GT_FALSE == isFlexLink)
                    && (!IS_PORT_XG_E(portType) && (port < 24)))
            {
                /*
                    1.1.1. Call with ifMode[CPSS_PORT_INTERFACE_MODE_SGMII_E /
                                            CPSS_PORT_INTERFACE_MODE_1000BASE_X_E].
                    Expected: GT_OK.
                */
                /* iterate with ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E */
                ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

                st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
                if (IS_PORT_FE_E(portType))
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);

                    /*
                        1.1.2. Call cpssDxChPortInterfaceModeGet with not-NULL ifModePtr.
                        Expected: GT_OK and the same ifMode as was set.
                    */
                    st = cpssDxChPortInterfaceModeGet(dev, port, &ifModeGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);

                    /* verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(ifMode, ifModeGet,
                         "got another ifMode then was set: %d, %d", dev, port);
                }

                /* iterate with ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E */
                ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;

                st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
                if (IS_PORT_FE_E(portType))
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);

                    /*
                        1.1.2. Call cpssDxChPortInterfaceModeGet with not-NULL ifModePtr.
                        Expected: GT_OK and the same ifMode as was set.
                    */
                    st = cpssDxChPortInterfaceModeGet(dev, port, &ifModeGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);

                    /* verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(ifMode, ifModeGet,
                         "got another ifMode then was set: %d, %d", dev, port);
                }
                /*
                    1.1.3. Call with ifMode [CPSS_PORT_INTERFACE_MODE_MII_E]
                           (only for CPU port).
                    Expected: NOT GT_OK.
                */
                ifMode = CPSS_PORT_INTERFACE_MODE_MII_E;

                st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
                if (!IS_PORT_FE_E(portType))
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);

                    /*
                        1.1.2. Call cpssDxChPortInterfaceModeGet with not-NULL ifModePtr.
                        Expected: GT_OK and the same ifMode as was set.
                    */
                    st = cpssDxChPortInterfaceModeGet(dev, port, &ifModeGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);

                    /* verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(ifMode, ifModeGet,
                         "got another ifMode then was set: %d, %d", dev, port);
                }

                /*
                    1.1.4. Call with wrong enum values ifMode.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChPortInterfaceModeSet
                                    (dev, port, ifMode),
                                    ifMode);
            }
            else
            {

                st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                st = prvUtfPortIsFlexLinkGet(dev, port, &isFlexLink);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /* the Interface for NOT Flex XG port cannot be changed */
                /* ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E   */
                /* Expected: GT_BAD_PARAM                      */
                ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

                st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
                if (isFlexLink)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                }
                else
                {
                    if(IS_SGMII_CAPABLE_PORT(portTypeOptions))
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                    else
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, ifMode);
                }

                /* ifMode = CPSS_PORT_INTERFACE_MODE_XGMII_E   */
                /* Expected: GT_OK for XAUI capable ports     */
                ifMode = CPSS_PORT_INTERFACE_MODE_XGMII_E;

                st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);

                if (IS_XAUI_CAPABLE_XG_PORT(portTypeOptions))
                {
                    if (((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)&&
                           (port%2 != 0)) ||
                         (((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)||
                          (PRV_CPSS_SIP_5_CHECK_MAC(dev))) &&
                           (port%4 != 0) && (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port) != 9)))
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                    }
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                }

                if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)||
                   (PRV_CPSS_SIP_5_CHECK_MAC(dev)))
                {/* this test case impartant because it reconfigures ports to mode using 1 serdes
                    and prevents failures in SGMII test case for ports where port%4!=0 */
                    /* ifMode = CPSS_PORT_INTERFACE_MODE_KR_E   */
                    /* Expected: GT_OK for all ports     */
                    ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;

                    st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);
                }
            }
        }

        ifMode = CPSS_PORT_INTERFACE_MODE_GMII_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[overrun-local] */
        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS | UTF_XCAT3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInterfaceModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_U8                          portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInterfaceModeGet)
{
/*
    ITERATE_DEVICES_PHY_OR_CPU_PORTS (DxCh)
    1.1.1. Call with non-null ifModePtr.
    Expected: GT_OK.
    1.1.2. Call with ifModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st   = GT_OK;

    GT_U8                        dev;
    GT_PHYSICAL_PORT_NUM         port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_INTERFACE_MODE_ENT mode = CPSS_PORT_INTERFACE_MODE_NA_E;

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
            /* 1.1.1. Call with non-null ifModePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortInterfaceModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with ifModePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortInterfaceModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInterfaceModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInterfaceModeGet(dev, port, &mode);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInterfaceModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInterfaceModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForceLinkPassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForceLinkPassEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortForceLinkPassEnableGet with non NULL statePtr.
    Expected: GT_OK and the same state.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE */
            state = GT_FALSE;

            st = cpssDxChPortForceLinkPassEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChPortForceLinkPassEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChPortForceLinkPassEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortForceLinkPassEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);

            /* Call function with enable = GT_TRUE */
            state = GT_TRUE;

            st = cpssDxChPortForceLinkPassEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChPortForceLinkPassEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChPortForceLinkPassEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortForceLinkPassEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE     */
            st = cpssDxChPortForceLinkPassEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForceLinkPassEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* state == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForceLinkPassEnableSet(dev, port, state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, state);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForceLinkPassEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortForceLinkPassEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForceLinkPassEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForceLinkPassEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with not NULL statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     state = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL statePtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortForceLinkPassEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortForceLinkPassEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statePtr = NULL", dev);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortForceLinkPassEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForceLinkPassEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForceLinkPassEnableGet(dev, port, &state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, state);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForceLinkPassEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortForceLinkPassEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForceLinkDownEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForceLinkDownEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortForceLinkDownEnableGet with non NULL statePtr.
    Expected: GT_OK and the same state.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChPortForceLinkDownEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChPortForceLinkDownEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChPortForceLinkDownEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortForceLinkDownEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);

            /* Call function with enable = GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChPortForceLinkDownEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChPortForceLinkDownEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChPortForceLinkDownEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortForceLinkDownEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE */
            st = cpssDxChPortForceLinkDownEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForceLinkDownEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* state == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForceLinkDownEnableSet(dev, port, state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, state);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForceLinkDownEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortForceLinkDownEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForceLinkDownEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForceLinkDownEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with not NULL statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     state = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL statePtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortForceLinkDownEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortForceLinkDownEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statePtr = NULL", dev);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortForceLinkDownEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForceLinkDownEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForceLinkDownEnableGet(dev, port, &state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, state);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForceLinkDownEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortForceLinkDownEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_U8  portNum,
    IN  GT_U32 mruSize
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMruSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with mruSize [0, 10304].
    Expected: GT_OK and the same value.
    1.1.2. Check that odd values are unsupported. Call with mruSize [1, 16383].
    Expected: NOT GT_OK.
    1.1.3. Call with mruSize [16384] (out of range).
    Expected: NOT GT_OK.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: GT_OK.
    */

    GT_STATUS st      = GT_OK;

    GT_U8     dev;
    GT_PHYSICAL_PORT_NUM port    = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32    size    = 0;
    GT_U32    sizeGet = 1;
    GT_BOOL   isRemote;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            /* 1.1.1. Call with mruSize [0, 10304/10240, 2048, 1522].
               Expected: GT_OK and the same value.
            */

            isRemote = prvCpssDxChPortRemotePortCheck(dev,port);

            /* mruSize = 0 */
            size = 0;

            st = cpssDxChPortMruSet(dev, port, size);
            if(isRemote == GT_FALSE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

                /*checking value*/
                st = cpssDxChPortMruGet(dev, port, &sizeGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
                UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                         "got another size then was set: %d, %d", dev, port);
            }

            if(isRemote == GT_FALSE)
            {
                /* mruSize = 10304 */
                size = 10304;
            }
            else
            {
                /* mruSize = 10240 */
                size = 10240;
            }

            st = cpssDxChPortMruSet(dev, port, size);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /*checking value*/
            st = cpssDxChPortMruGet(dev, port, &sizeGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
            UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                     "got another size then was set: %d, %d", dev, port);

            if(isRemote == GT_FALSE)
            {
                /* mruSize = 2048 */
                size = 2048;

                st = cpssDxChPortMruSet(dev, port, size);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

                /*checking value*/
                st = cpssDxChPortMruGet(dev, port, &sizeGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
                UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                         "got another size then was set: %d, %d", dev, port);

                /* mruSize = 1522 */
                size = 1522;

                st = cpssDxChPortMruSet(dev, port, size);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

                /*checking value*/
                st = cpssDxChPortMruGet(dev, port, &sizeGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
                UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                         "got another size then was set: %d, %d", dev, port);
            }

            /* 1.1.2. Check that odd values are unsupported.
                      Call with mruSize [1, 16383].
               Expected: NOT GT_OK.
            */

            /* mruSize = 1 */
            size = 1;

            st = cpssDxChPortMruSet(dev, port, size);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /* mruSize = 16383 */
            size = 16383;

            st = cpssDxChPortMruSet(dev, port, size);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /* 1.1.3. Call with mruSize [16384] (out of range).
               Expected: NOT GT_OK.
            */
            size = 16384;

            st = cpssDxChPortMruSet(dev, port, size);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
        }

        size = 500;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMruSet(dev, port, size);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMruSet(dev, port, size);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMruSet(dev, port, size);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, size);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    size = 500;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMruSet(dev, port, size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, size == 500 */

    st = cpssDxChPortMruSet(dev, port, size);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCrcCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCrcCheckEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
        1.1.1. Call with state [GT_FALSE and GT_TRUE].
        Expected: GT_OK.
        1.1.2. Call cpssDxChPortCrcCheckEnableGet with not null state.
        Expected: GT_OK and the same value.
    */
    GT_U8       dev;
    GT_STATUS   st       = GT_OK;
    GT_PHYSICAL_PORT_NUM port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_TRUE;

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
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK and the same value.
            */

            /* Call function with enable = GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* Call function with enable = GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChPortCrcCheckEnableGet with not null state.
                Expected: GT_OK and the same value.
            */
            state = GT_FALSE;

            st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*checking value*/
            st = cpssDxChPortCrcCheckEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                     "got another state then was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE */
            st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* state == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, state);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortCrcCheckEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXGmiiModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXGmiiModeSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with mode
    [CPSS_PORT_XGMII_LAN_E, CPSS_PORT_XGMII_WAN_E].
    Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED for others.
    1.1.2. For 10Gbps port call with mode [3].
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS                st       = GT_OK;
    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM     port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_XGMII_MODE_ENT mode     = CPSS_PORT_XGMII_LAN_E;
    PRV_CPSS_PORT_TYPE_ENT   portType = PRV_CPSS_PORT_NOT_EXISTS_E;

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
            /* 1.1.1. Call with mode [CPSS_PORT_XGMII_LAN_E /
                                      CPSS_PORT_XGMII_WAN_E].
               Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED for others.
            */

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* mode = CPSS_PORT_XGMII_LAN_E */
            mode = CPSS_PORT_XGMII_LAN_E;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortXGmiiModeSet(dev, port, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
                continue;
            }

            st = cpssDxChPortXGmiiModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /* mode = CPSS_PORT_XGMII_WAN_E */
            mode = CPSS_PORT_XGMII_WAN_E;

            st = cpssDxChPortXGmiiModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
               1.1.2. For 10Gbps port call with wrong enum values mode.
               Expected: GT_BAD_PARAM.
            */
            if(PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChPortXGmiiModeSet
                                    (dev, port, mode),
                                    mode);
            }
        }

        mode = CPSS_PORT_XGMII_LAN_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* mode = CPSS_PORT_XGMII_LAN_E                 */
            st = cpssDxChPortXGmiiModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* mode = CPSS_PORT_XGMII_LAN_E */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXGmiiModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* mode = CPSS_PORT_XGMII_LAN_E */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortXGmiiModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mode = CPSS_PORT_XGMII_LAN_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXGmiiModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, mode = CPSS_PORT_XGMII_LAN_E               */

    st = cpssDxChPortXGmiiModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortIpgSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_U32  ipg
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortIpgSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    Expected: GT_NOT_SUPPORTED XG ports GT_OK for others.
    */

    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      ipg      = PORT_IPG_VALID_CNS;
    GT_U32      ipgGet;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;
    GT_U32      validPorts[1];
    GT_PHYSICAL_PORT_NUM    firstPortNum;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
       /* get ports with MAC */
       prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
           0  /*start port */, &validPorts[0] , 1);
       firstPortNum = validPorts[0];

       ipg      = PORT_IPG_VALID_CNS;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortIpgSet(dev, port, ipg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /* 1.1.1.
               GT_NOT_SUPPORTED XG ports GT_OK for others.
            */
            st = cpssDxChPortIpgSet(dev, port, ipg);
            if(GE_NOT_SUPPORTED(portTypeOptions))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortIpgSet(dev, port, ipg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3 For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortIpgSet(dev, port, ipg);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /* 1.4 check for valid and invalid IPG values */
        port = firstPortNum;
        /* skip XG ports */

        st = prvUtfPortTypeGet(dev, port, &portMacType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        if(portMacType >= PRV_CPSS_PORT_XG_E)
            continue;

        ipg  = PORT_IPG_VALID_CNS;

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* ipg  = PORT_IPG_VALID_CNS(8), valid for all DxCh devices */
        st = cpssDxChPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChPortIpgGet(dev, port, &ipgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortIpgGet: %d, %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                     "get another enable value than was set: %d, %d", dev, port);

        /* ipg  = 15, valid for all DxCh devices */
        ipg = 15;

        st = cpssDxChPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChPortIpgGet(dev, port, &ipgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortIpgGet: %d, %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                     "get another enable value than was set: %d, %d", dev, port);

        /* ipg  = 16, valid for Ch3 and above */
        ipg = 16;

        st = cpssDxChPortIpgSet(dev, port, ipg);
        if(devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChPortIpgGet(dev, port, &ipgGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortIpgGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                     "get another enable value than was set: %d, %d", dev, port);
        }
        else /* CPSS_PP_FAMILY_CHEETAH_E  or CPSS_PP_FAMILY_CHEETAH2_E */
        {
           UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, ipg);
        }

        /* ipg  = 511, valid for Ch3 and above */
        ipg = 511;

        st = cpssDxChPortIpgSet(dev, port, ipg);
        if(devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChPortIpgGet(dev, port, &ipgGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortIpgGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                     "get another enable value than was set: %d, %d", dev, port);
        }
        else /* CPSS_PP_FAMILY_CHEETAH_E  or CPSS_PP_FAMILY_CHEETAH2_E */
        {
           UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, ipg);
        }

        /* ipg  = 512, invalid for all DxCh devices */
        ipg = 512;

        st = cpssDxChPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, ipg);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;
    ipg  = PORT_IPG_VALID_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0 */

    st = cpssDxChPortIpgSet(dev, port, ipg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortIpgGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT  GT_U32  *ipgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortIpgGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    Expected: GT_NOT_SUPPORTED XG ports GT_OK for others.
    */

    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      ipg;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;

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
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);

            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortIpgGet(dev, port, &ipg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /* 1.1.1.
               GT_NOT_SUPPORTED XG ports GT_OK for others.
            */
            st = cpssDxChPortIpgGet(dev, port, &ipg);
            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                if(GE_NOT_SUPPORTED(portTypeOptions))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    /* 1.1.2. Call with speedPtr [NULL].
                    Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChPortIpgGet(dev, port, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortIpgGet(dev, port, &ipg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3 For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortIpgGet(dev, port, &ipg);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortIpgGet(dev, port, &ipg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0 */

    st = cpssDxChPortIpgGet(dev, port, &ipg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortExtraIpgSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_U8       number
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortExtraIpgSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with number [2].
    Expected: GT_OK for 10-Gbps ports and GT_NOT_SUPPORTED for others.
    */

    GT_STATUS   st        = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U8       number    = 2;
    GT_U8       numberGet = 1;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortExtraIpgSet(dev, port, number);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, number);
                continue;
            }

            /* 1.1.1. Call with number [2].
               Expected: GT_OK for 10-Gbps ports and GT_NOT_SUPPORTED for others.
            */
            st = cpssDxChPortExtraIpgSet(dev, port, number);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);

            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(portMacType >= PRV_CPSS_PORT_XG_E && portMacType!=PRV_CPSS_PORT_CG_E)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);

                st = cpssDxChPortExtraIpgGet(dev, port, &numberGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);
                UTF_VERIFY_EQUAL2_STRING_MAC(number, numberGet,
                         "got another number then was set: %d, %d", dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* number == 2 */

            st = cpssDxChPortExtraIpgSet(dev, port, number);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* number == 2 */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortExtraIpgSet(dev, port, number);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* number == 2 */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortExtraIpgSet(dev, port, number);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* number == 2 */
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortExtraIpgSet(dev, port, number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, number == 2 */

    st = cpssDxChPortExtraIpgSet(dev, port, number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *isLocalFaultPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXgmiiLocalFaultGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with non-null isLocalFaultPtr.
    Expected: GT_OK  for 10-Gbps ports and GT_NOT_SUPPORTED for others.
    1.1.2. Call with isLocalFaultPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;

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
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortXgmiiLocalFaultGet(dev, port, &state);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /* 1.1.1. Call with non-null isLocalFaultPtr.
               Expected: GT_OK  for 10-Gbps ports and GT_NOT_SUPPORTED for others.
            */
            st = cpssDxChPortXgmiiLocalFaultGet(dev, port, &state);
            if(portMacType < PRV_CPSS_PORT_XG_E)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with isLocalFaultPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortXgmiiLocalFaultGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortXgmiiLocalFaultGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXgmiiLocalFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortXgmiiLocalFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXgmiiLocalFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXgmiiLocalFaultGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXgmiiRemoteFaultGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with non-null isRemoteFaultPtr.
    Expected: GT_OK  for 10-Gbps ports and GT_NOT_SUPPORTED for others.
    1.1.2. Call with isRemoteFaultPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;

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
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, &state);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /* 1.1.1. Call with non-null isRemoteFaultPtr.
               Expected: GT_OK for 10-Gbps ports and GT_NOT_SUPPORTED for others.
            */
            st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, &state);
            if(portMacType < PRV_CPSS_PORT_XG_E)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with isRemoteFaultPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXgmiiRemoteFaultGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacStatusGet
(
    IN  GT_U8                      devNum,
    IN  GT_U8                      portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacStatusGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with non-null portMacStatusPtr.
    Expected: GT_OK.
    1.1.2. Call with portMacStatusPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS                st     = GT_OK;

    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM     port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_MAC_STATUS_STC status;

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
            /* 1.1.1. Call with non-null portMacStatusPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortMacStatusGet(dev, port, &status);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with portMacStatusPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortMacStatusGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortMacStatusGet(dev, port, &status);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacStatusGet(dev, port, &status);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacStatusGet(dev, port, &status);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacStatusGet(dev, port, &status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacStatusGet(dev, port, &status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInternalLoopbackEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInternalLoopbackEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortInternalLoopbackEnableGet.
    Expected: GT_OK and enablePtr the same as was written.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT             speed = CPSS_PORT_SPEED_NA_E;
#ifndef ASIC_SIMULATION
    GT_BOOL                         isLinkUp;
#endif

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            if (st != GT_OK)
            {
                continue;
            }

            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssDxChPortSpeedGet(dev, port, &speed);
                if (st != GT_OK)
                {
                    continue;
                }
            }

            /* Call function with enable = GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
            if(((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
               && (CPSS_PP_FAMILY_DXCH_LION2_E <= PRV_CPSS_PP_MAC(dev)->devFamily))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /* 1.1.2. Call cpssDxChPortInternalLoopbackEnableGet.
               Expected: GT_OK and enablePtr the same as was written.
            */
            st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &state);
            if(((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
               && (CPSS_PP_FAMILY_DXCH_LION2_E <= PRV_CPSS_PP_MAC(dev)->devFamily))
            {
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortInternalLoopbackEnableGet: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortInternalLoopbackEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                             "get another enable value than was set: %d, %d", dev, port);
            }
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
            if(((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
               && (CPSS_PP_FAMILY_DXCH_LION2_E <= PRV_CPSS_PP_MAC(dev)->devFamily))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /* 1.1.2. Call cpssDxChPortInternalLoopbackEnableGet.
               Expected: GT_OK and enablePtr the same as was written.
            */
            st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &state);
            if(((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
               && (CPSS_PP_FAMILY_DXCH_LION2_E <= PRV_CPSS_PP_MAC(dev)->devFamily))
            {
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortInternalLoopbackEnableGet: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortInternalLoopbackEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                             "get another enable value than was set: %d, %d", dev, port);
#ifndef ASIC_SIMULATION
                /* wait 10mSec before link check to let it stabilize */
                cpssOsTimerWkAfter(10);

                st = cpssDxChPortLinkStatusGet(dev, port, &isLinkUp);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortLinkStatusGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isLinkUp,
                                             "link is down although defined loopback: %d, %d", dev, port);
#endif
            }
        }

#ifndef ASIC_SIMULATION
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1.3 Go over all available physical ports and check link,
           in previous loop on all ports was loopback enabled, so I expect all
           ports be in link up */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            if (st != GT_OK)
            {
                continue;
            }

            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssDxChPortSpeedGet(dev, port, &speed);
                if (st != GT_OK)
                {
                    continue;
                }
            }

            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
                continue;

            st = cpssDxChPortLinkStatusGet(dev, port, &isLinkUp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChPortLinkStatusGet_2: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isLinkUp,
                                         "link is down although loopback enabled on it: %d, %d", dev, port);

            /* disable loopback to avoid surprises in other tests. */
            enable = GT_FALSE;
            st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }
#endif

        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE */
            st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortInternalLoopbackEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInternalLoopbackEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInternalLoopbackEnableGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT             speed = CPSS_PORT_SPEED_NA_E;

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
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            if (st != GT_OK)
            {
                continue;
            }

            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssDxChPortSpeedGet(dev, port, &speed);
                if (st != GT_OK)
                {
                    continue;
                }
            }

            /* 1.1.1. Call with non-null enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &enable);
            if(((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
               && (CPSS_PP_FAMILY_DXCH_LION2_E <= PRV_CPSS_PP_MAC(dev)->devFamily))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortInternalLoopbackEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInternalLoopbackEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInbandAutoNegEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInbandAutoNegEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED for 10Gbps.
    1. 2. Call for CPU port [CPSS_CPU_PORT_NUM_CNS] with enable [GT_TRUE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS              st        = GT_OK;

    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                enable    = GT_FALSE;
    GT_BOOL                enableGet = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT portType  = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* 1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED for 10Gbps.
            */

            /* enable = GT_FALSE */
            enable = GT_FALSE;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                continue;
            }

            st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
            if(((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PORT_XG_E == portType)) ||
                ((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_CHEETAH3_E)
                                && (port >= 24)))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPortInbandAutoNegEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                        "get another enable value than was set: %d, %d", dev, port);
            }

            /* enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
            if(((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PORT_XG_E == portType)) ||
                ((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_CHEETAH3_E)
                                && (port >= 24)))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPortInbandAutoNegEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                        "get another enable value than was set: %d, %d", dev, port);
            }
        }

        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state = GT_TRUE */
            st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssDxChPortInbandAutoNegEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortAttributesOnPortGet
(
    IN    GT_U8                     devNum,
    IN    GT_U8                     portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortAttributesOnPortGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with non-null portAttributSetArrayPtr.
    Expected: GT_OK.
    1.1.2. Call with portAttributSetArrayPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                st  = GT_OK;

    GT_U8                    dev;
   GT_PHYSICAL_PORT_NUM      port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_ATTRIBUTES_STC attributes;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_BOOL                  isFlexLink;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if((CPSS_PP_FAMILY_DXCH_XCAT_E == devFamily) ||
               (CPSS_PP_FAMILY_DXCH_XCAT2_E == devFamily))
            {/* cpss appDemo doesn't initialize stacking ports of xcat/2 and
              * attribitesGet returns NOT_INITIALIZED, so let initialize the ports here
              * to avoid failures
              */

                st = prvUtfPortIsFlexLinkGet(dev, port, &isFlexLink);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                if(isFlexLink)
                {
                    st = cpssDxChPortInterfaceModeSet(dev,port,CPSS_PORT_INTERFACE_MODE_SGMII_E);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPortInterfaceModeSet %d, %d, CPSS_PORT_INTERFACE_MODE_SGMII_E", dev, port);

                    st = cpssDxChPortSpeedSet(dev, port, CPSS_PORT_SPEED_1000_E);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPortSpeedSet %d, %d, CPSS_PORT_SPEED_1000_E", dev, port);

                    st = cpssDxChPortSerdesPowerStatusSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, 0x1, GT_TRUE);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPortSerdesPowerStatusSet %d, %d, CPSS_PORT_DIRECTION_BOTH_E, 0x1, GT_TRUE",
                        dev, port);
                }
            }

            /*
               1.1.1. Call with non-null portAttributSetArrayPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortInterfaceModeGet(dev,port,&ifMode);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            st = cpssDxChPortAttributesOnPortGet(dev, port, &attributes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with portAttributSetArrayPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortAttributesOnPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortAttributesOnPortGet(dev, port, &attributes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortAttributesOnPortGet(dev, port, &attributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /* 1.3. For active device check that function returns GT_OK */
            /* for CPU port number.                                     */
            port = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChPortAttributesOnPortGet(dev, port, &attributes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortAttributesOnPortGet(dev, port, &attributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortAttributesOnPortGet(dev, port, &attributes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_U8                    portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPreambleLengthSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E] and length [8, 4]
    Expected: GT_OK.
    1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E /
                                CPSS_PORT_DIRECTION_BOTH_E] and length [4]
    Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED  for others.
    1.1.3. Call cpssDxChPortPreambleLengthGet with the same parameters
    as in 1.1.1 and 1.1.2.
    Expected: GT_OK and the same value.
    1.1.4. Call with direction [4] and length [4, 8]
    Expected: GT_BAD_PARAM.
    1.1.5. Call with direction [CPSS_PORT_DIRECTION_TX_E] and length [1, 2]
    Expected: NON GT_OK.
*/

    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_DIRECTION_ENT direction = CPSS_PORT_DIRECTION_RX_E;
    GT_U32                  length    = 0;
    GT_U32                  lengthGet = 1;
    PRV_CPSS_PORT_TYPE_ENT  portType  = PRV_CPSS_PORT_NOT_EXISTS_E;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_BOOL                 is_88E1690_RemotePort;
    GT_STATUS               expSt; /* expected status */
    GT_BOOL                 isXlgMacSupported = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* remote 88E1690 port doesn't support preamble other than 8 bytes. */
            is_88E1690_RemotePort = prvCpssDxChPortRemotePortCheck(dev, port);


            isXlgMacSupported = GT_FALSE;
            if(GT_FALSE == is_88E1690_RemotePort &&
               (PRV_CPSS_SIP_5_CHECK_MAC(dev) || devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                st = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                          PRV_CPSS_PORT_XLG_E,
                                                          /* OUT */&isXlgMacSupported);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            /*
                1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E], length [8, 4]
               Expected: GT_OK.
            */

            direction = CPSS_PORT_DIRECTION_TX_E;

            /* Call with length = 8 */
            length = 8;

            st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);

            if(PRV_CPSS_PORT_GE_E == portType || PRV_CPSS_PORT_FE_E == portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                /*
                    1.1.3. Call cpssDxChPortPreambleLengthGet with the same parameters
                    as in 1.1.1 and 1.1.2.
                    Expected: GT_OK and the same value.
                */
                st = cpssDxChPortPreambleLengthGet(dev, port, direction, &lengthGet);

                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                UTF_VERIFY_EQUAL2_STRING_MAC(length, lengthGet,
                                 "cpssDxChPortPreambleLengthGet: %d, %d", dev, port);
            }


            /* Call with length = 4 */
            length = 4;

            st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);

            if (GT_TRUE == is_88E1690_RemotePort)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, direction, length);
            }
            else if(PRV_CPSS_PORT_GE_E == portType || PRV_CPSS_PORT_FE_E == portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                /*
                    1.1.3. Call cpssDxChPortPreambleLengthGet with the same parameters
                    as in 1.1.1 and 1.1.2.
                    Expected: GT_OK and the same value.
                */
                st = cpssDxChPortPreambleLengthGet(dev, port, direction, &lengthGet);

                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                UTF_VERIFY_EQUAL2_STRING_MAC(length, lengthGet,
                                 "cpssDxChPortPreambleLengthGet: %d, %d", dev, port);
            }

            /*
                1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E /
                                           CPSS_PORT_DIRECTION_BOTH_E], length [4]
               Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED  for others.
            */

            /* Call with direction = CPSS_PORT_DIRECTION_RX_E */
            direction = CPSS_PORT_DIRECTION_RX_E;
            length = 4;

            if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
               (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                GT_BOOL                          valid;

                if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    st = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/&portMapShadow);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                    valid = portMapShadow.valid;
                }
                else
                {
                    valid = GT_TRUE;
                }

                if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
                {
                    if (GT_FALSE == is_88E1690_RemotePort)
                    {
                        st = prvCpssDxChBobcat2PortGigaMacPreambleLengthSet(dev,port,direction,length);
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                        st = prvCpssDxChBobcat2PortXLGMacPreambleLengthSet(dev,port,direction,length);
                        if (isXlgMacSupported == GT_TRUE)
                        {
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        }
                        else
                        {
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        }
                    }
                    st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                    if (isXlgMacSupported == GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                }
            }
            else
            {
                st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                if(PRV_CPSS_PORT_XG_E <= portType )
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                    /*
                        1.1.3. Call cpssDxChPortPreambleLengthGet with the same parameters
                        as in 1.1.1 and 1.1.2.
                        Expected: GT_OK and the same value.
                    */
                    st = cpssDxChPortPreambleLengthGet(dev, port, direction, &lengthGet);

                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                    UTF_VERIFY_EQUAL2_STRING_MAC(length, lengthGet,
                                     "cpssDxChPortPreambleLengthGet: %d, %d", dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, length);
                }
            }

            /* Call with direction = CPSS_PORT_DIRECTION_BOTH_E */
            direction = CPSS_PORT_DIRECTION_BOTH_E;

            st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
            if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
               (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                GT_STATUS                        rc;
                GT_BOOL                          valid;

                if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/&portMapShadow);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);
                    valid = portMapShadow.valid;
                }
                else
                {
                    valid = GT_TRUE;
                }

                if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
                {
                    if (isXlgMacSupported == GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                        /*we don't call get for direction = CPSS_PORT_DIRECTION_BOTH_E */
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                    }
                }
            }
            else
            {
                if(PRV_CPSS_PORT_XG_E <= portType)
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                    /*we don't call get for direction = CPSS_PORT_DIRECTION_BOTH_E */
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, length);
                }
            }
            /*
                1.1.3. Call with wrong enum values direction , length [4]
               Expected: GT_BAD_PARAM.
            */
            /* length == 4; */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPreambleLengthSet
                                (dev, port, direction, length),
                                direction);

            /*
               1.1.4. Call with direction [CPSS_PORT_DIRECTION_TX_E], length [1, 2]
               Expected: NON GT_OK.

            */

            direction = CPSS_PORT_DIRECTION_TX_E;
            expSt = (is_88E1690_RemotePort == GT_TRUE) ? GT_NOT_SUPPORTED : GT_OK;

            if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
               (devFamily ==  CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                GT_BOOL                          valid;

                if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    st = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/&portMapShadow);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                    valid = portMapShadow.valid;
                }
                else
                {
                    valid = GT_TRUE;
                }

                if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
                {
                    if (isXlgMacSupported == GT_TRUE)
                    {
                        /* Call with length = 1 */
                        length = 1;
                        st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);

                        /* Call with length = 2 */
                        length = 2;
                        st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);
                    }
                    else
                    {
                        /* Call with length = 1 */
                        length = 1;
                        st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);

                        /* Call with length = 2 */
                        length = 2;
                        st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);
                    }
                }
            }
            else
            {
                /* Call with length = 1 */
                length = 1;
                st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);

                /* Call with length = 2 */
                length = 2;
                st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        length = 4;
        direction = CPSS_PORT_DIRECTION_TX_E;

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* direction == CPSS_PORT_DIRECTION_TX_E */

            st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, length);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    length = 4;
    direction = CPSS_PORT_DIRECTION_TX_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, direction == CPSS_PORT_DIRECTION_TX_E, length == 4 */

    st = cpssDxChPortPreambleLengthSet(dev, port, direction, length);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacSaBaseSet)
{
/*
    ITERATE_DEVICES (DxCh)
    1.1. Call with mac {arEther[10, 20, 30, 40, 50, 60]}.
    Expected: GT_OK.
    1.2. Call cpssDxChPortMacSaBaseGet with not-NULL macPtr.
    Expected: GT_OK and the same mac as was set.
    1.3. Call with mac[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_ETHERADDR    mac;
    GT_ETHERADDR    macGet;
    GT_BOOL         isEqual = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &mac, sizeof(mac));
    cpssOsBzero((GT_VOID*) &macGet, sizeof(macGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mac {arEther[10, 20, 30, 40, 50, 0]}.
            Expected: GT_OK.
        */
        mac.arEther[0] = 10;
        mac.arEther[1] = 20;
        mac.arEther[2] = 30;
        mac.arEther[3] = 40;
        mac.arEther[4] = 50;
        mac.arEther[5] = 0;

        st = cpssDxChPortMacSaBaseSet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortMacSaBaseGet with not-NULL macPtr.
            Expected: GT_OK and the same mac as was set.
        */
        st = cpssDxChPortMacSaBaseGet(dev, &macGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortMacSaBaseGet: %d", dev);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mac, (GT_VOID*) &macGet, sizeof(mac))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another mac then was set: %d", dev);

        /*
            1.3. Call with mac[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortMacSaBaseSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mac = NULL" ,dev);
    }

    mac.arEther[0] = 10;
    mac.arEther[1] = 20;
    mac.arEther[2] = 30;
    mac.arEther[3] = 40;
    mac.arEther[4] = 50;
    mac.arEther[5] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacSaBaseSet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacSaBaseSet(dev, &mac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacSaBaseGet)
{
/*
    ITERATE_DEVICES (DxCh)
    1.1. Call with not-NULL mac
    Expected: GT_OK.
    1.2. Call with mac[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_ETHERADDR    mac;


    cpssOsBzero((GT_VOID*) &mac, sizeof(mac));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL mac
            Expected: GT_OK.
        */
        st = cpssDxChPortMacSaBaseGet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with mac[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortMacSaBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mac = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacSaBaseGet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacSaBaseGet(dev, &mac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesComboModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portNum,
    IN  CPSS_DXCH_PORT_SERDES_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesComboModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh3, 20..23)
    1.1.1. Call with mode[CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E /
                          CPSS_DXCH_PORT_SERDES_MODE_COMBO_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesComboModeGet with not-NULL modePtr.
    Expected: GT_OK and the same mode as was set.
    1.1.3. Call with mode[wrong enum values].
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS            st   = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_DXCH_PORT_SERDES_MODE_ENT  mode    = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E;
    CPSS_DXCH_PORT_SERDES_MODE_ENT  modeGet = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if((port>19) && (port<24))
            {
                /*
                    1.1.1. Call with mode[CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E /
                                          CPSS_DXCH_PORT_SERDES_MODE_COMBO_E].
                    Expected: GT_OK.
                */
                /* iterate with mode = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E */
                mode = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E;

                st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                /*
                    1.1.2. Call cpssDxChPortSerdesComboModeGet with not-NULL modePtr.
                    Expected: GT_OK and the same mode as was set.
                */
                st = cpssDxChPortSerdesComboModeGet(dev, port, &modeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesComboModeGet: %d, %d" ,dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet, "got another mode as was set: %d, %d" ,dev, port);

                /* iterate with mode = CPSS_DXCH_PORT_SERDES_MODE_COMBO_E */
                mode = CPSS_DXCH_PORT_SERDES_MODE_COMBO_E;

                st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                /*
                    1.1.2. Call cpssDxChPortSerdesComboModeGet with not-NULL modePtr.
                    Expected: GT_OK and the same mode as was set.
                */
                st = cpssDxChPortSerdesComboModeGet(dev, port, &modeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesComboModeGet: %d, %d" ,dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet, "got another mode as was set: %d, %d" ,dev, port);

                /*
                    1.1.3. Call with mode[wrong enum values].
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChPortSerdesComboModeSet
                                    (dev, port, mode),
                                    mode);
            }
            else
            {
                /* wrong port. Expected: GT_BAD_PARAM */
                mode = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E;

                st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
            }
        }

        mode = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E;

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mode = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, dMode == CPSS_PORT_FULL_DUPLEX_E */

    st = cpssDxChPortSerdesComboModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesComboModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_U8                             portNum,
    OUT CPSS_DXCH_PORT_SERDES_MODE_ENT    *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesComboModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh3, 20..23)
    1.1.1. Call with not-NULL modePTr.
    Expected: GT_OK.
    1.1.2. Call with modePtr[NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS            st   = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_DXCH_PORT_SERDES_MODE_ENT  mode    = CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if((port>19) && (port<24))
            {
                /*
                    1.1.1. Call with not-NULL modePTr.
                    Expected: GT_OK.
                */
                st = cpssDxChPortSerdesComboModeGet(dev, port, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call with modePtr[NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortSerdesComboModeGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, mode = NULL", dev, port);
            }
            else
            {
                /* wrong port. Expected: GT_BAD_PARAM */
                st = cpssDxChPortSerdesComboModeGet(dev, port, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesComboModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesComboModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesComboModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesComboModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, dMode == CPSS_PORT_FULL_DUPLEX_E */

    st = cpssDxChPortSerdesComboModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesConfigGet
(
    IN GT_U8  devNum,
    IN GT_U8  portNum,
    OUT CPSS_DXCH_PORT_SERDES_CONFIG_STC  *serdesCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesConfigGet)
{
/*
    ITERATE_DEVICE_PHY_PORT (DxCh3 and above)
    1.1.1. Call with not NULL serdesCfgPtr.
    Expected: GT_OK.
    1.1.2. Call with serdesCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    CPSS_DXCH_PORT_SERDES_CONFIG_STC  serdesCfg;

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);
            /*
                1.1.1. Call with not NULL serdesCfgPtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortSerdesConfigGet(dev, port, &serdesCfg);
            if(IS_PORT_FE_E(portType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with serdesCfgPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSerdesConfigGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, serdesCfgPtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesConfigGet(dev, port, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesConfigGet(dev, port, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesConfigGet(dev, port, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesConfigGet(dev, port, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesConfigGet(dev, port, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesManualTxConfigSet)
{
/*
    ITERATE_DEVICE_PHY_PORT (Lion2 and above)
    1.1.1. Call with valid values
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesConfigGet with not NULL serdesCfgPtr.
    Expected: GT_OK and the same serdesCfgPtr as was set.
    1.1.3. Call with values out of range
    Expected: NOT GT_OK.
    1.1.5. Call with serdesCfg [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32               laneNum;
    GT_BOOL              isSimulation;

    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = 0;
    GT_U32                          startSerdes;
    GT_U32                          numOfSerdesLanes;

    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesCfg;
    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesCfgGet;
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));
    cpssOsBzero((GT_VOID*) &serdesCfgGet, sizeof(serdesCfgGet));
    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    #ifdef ASIC_SIMULATION
          isSimulation = GT_TRUE;
    #else
          isSimulation = GT_FALSE;
    #endif

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily;
        if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) && (PRV_CPSS_PP_MAC(dev)->revision == 0))
        {
                 SKIP_TEST_MAC
        }

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            GT_BOOL                            valid;
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

                valid = GT_FALSE;
                st = cpssDxChPortPhysicalPortDetailedMapGet(dev, port, /*OUT*/portMapShadowPtr);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if(GT_TRUE == portMapShadowPtr->valid)
                {
                    st = prvUtfPortInterfaceModeGet(dev, port, &ifMode);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", dev, port);

                    if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    {
                        valid = GT_FALSE;
                    }
                    else /* (numOfSerdesLanes > 0) */
                    {
                        valid = GT_TRUE;
                    }
                }
            }
            else
            {
                valid = GT_TRUE;
                st = prvUtfPortInterfaceModeGet(dev,port,&ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", dev, port);
            }

            if (valid == GT_TRUE)
            {
                if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

                st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode,
                                                       &startSerdes,
                                                       &numOfSerdesLanes);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                        1.1.5. Call with serdesCfg [NULL].
                        Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortSerdesManualTxConfigSet(dev, port, startSerdes, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                             "%d, %d, serdesCfgPtr = NULL", dev, port);

                for(laneNum = 0; laneNum < numOfSerdesLanes; laneNum++)
                {
                    /* call with first group of the values */
                    serdesCfg.txAmp = 0;
                    serdesCfg.txAmpAdjEn = GT_FALSE;
                    serdesCfg.emph0 = 0;
                    serdesCfg.emph1 = 0;
                    serdesCfg.txAmpShft = GT_FALSE;

                    st = cpssDxChPortSerdesManualTxConfigSet(dev, port, laneNum, &serdesCfg);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                            1.1.2. Call cpssDxChPortSerdesConfigGet with not NULL serdesCfgPtr.
                            Expected: GT_OK and the same serdesCfgPtr as was set.
                    */
                    st = cpssDxChPortSerdesManualTxConfigGet(dev, port, laneNum, &serdesCfgGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "cpssDxChPortSerdesConfigGet: %d, %d", dev, port);
                    if((! PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) || (GT_FALSE == isSimulation))
                    {
                        if(GT_OK == st)
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmp, serdesCfgGet.txAmp,
                                                         "get another serdesCfg.txAmp than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmp, serdesCfgGet.txAmp);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmpAdjEn, serdesCfgGet.txAmpAdjEn,
                                                         "get another serdesCfg.txAmpAdjEn than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmpAdjEn, serdesCfgGet.txAmpAdjEn);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph0, serdesCfgGet.emph0,
                                                         "get another serdesCfg.emph0 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph0, serdesCfgGet.emph0);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph1, serdesCfgGet.emph1,
                                                         "get another serdesCfg.emph1 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph1, serdesCfgGet.emph1);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmpShft, serdesCfgGet.txAmpShft,
                                                         "get another serdesCfg.txAmpShft than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmpShft, serdesCfgGet.txAmpShft);
                        }
                    }

                    /* call with second group of the values */
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
                        (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
                       (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
                       (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ||
                       (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
                          ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
                          (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)) )
                    {
                        serdesCfg.txAmp = 10;
                        serdesCfg.txAmpAdjEn = GT_FALSE;
                        serdesCfg.emph0 = 12;
                        serdesCfg.emph1 = 9;
                        serdesCfg.txAmpShft = GT_FALSE;
                    }
                    else
                    {
                        serdesCfg.txAmp = 10;
                        serdesCfg.txAmpAdjEn = GT_TRUE;
                        serdesCfg.emph0 = 15;
                        serdesCfg.emph1 = 15;
                        serdesCfg.txAmpShft = GT_TRUE;
                    }

                    st = cpssDxChPortSerdesManualTxConfigSet(dev, port, laneNum, &serdesCfg);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                            1.1.2. Call cpssDxChPortSerdesConfigGet with not NULL serdesCfgPtr.
                            Expected: GT_OK and the same serdesCfgPtr as was set.
                    */
                    st = cpssDxChPortSerdesManualTxConfigGet(dev, port, laneNum, &serdesCfgGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "cpssDxChPortSerdesConfigGet: %d, %d", dev, port);

                    if((! PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) || (GT_FALSE == isSimulation))
                    {
                        if(GT_OK == st)
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmp, serdesCfgGet.txAmp,
                                                         "get another serdesCfg.txAmp than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmp, serdesCfgGet.txAmp);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmpAdjEn, serdesCfgGet.txAmpAdjEn,
                                                         "get another serdesCfg.txAmpAdjEn than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmpAdjEn, serdesCfgGet.txAmpAdjEn);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph0, serdesCfgGet.emph0,
                                                         "get another serdesCfg.emph0 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph0, serdesCfgGet.emph0);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph1, serdesCfgGet.emph1,
                                                         "get another serdesCfg.emph1 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph1, serdesCfgGet.emph1);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmpShft, serdesCfgGet.txAmpShft,
                                                         "get another serdesCfg.txAmpShft than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmpShft, serdesCfgGet.txAmpShft);
                        }
                    }

                }

                /*
                        1.1.3. Call with serdesCfg->txAmp [32] out of range, and other params from 1.1.1.
                        Expected: NOT GT_OK.
                */
                serdesCfg.txAmp = 32;
                st = cpssDxChPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                /* if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) */
                {
                    st = cpssDxChPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesManualTxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesManualTxConfigGet)
{
/*
    ITERATE_DEVICE_PHY_PORT (DxCh3 and above)
    1.1.1. Call with not NULL serdesCfgPtr.
    Expected: GT_OK.
    1.1.2. Call with serdesCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_PHYSICAL_PORT_NUM                port;
    CPSS_PORT_INTERFACE_MODE_ENT        ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC serdesCfg;
    CPSS_PP_FAMILY_TYPE_ENT             devFamily;

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily;
        if((devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) && (PRV_CPSS_PP_MAC(dev)->revision == 0))
        {
            SKIP_TEST_MAC
        }


        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            GT_BOOL                            valid;
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

                valid = GT_FALSE;
                st = cpssDxChPortPhysicalPortDetailedMapGet(dev, port, /*OUT*/portMapShadowPtr);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if(GT_TRUE == portMapShadowPtr->valid)
                {
                    st = prvUtfPortInterfaceModeGet(dev, port, &ifMode);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", dev, port);

                    if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    {
                        valid = GT_FALSE;
                    }
                    else /* (numOfSerdesLanes > 0) */
                    {
                        valid = GT_TRUE;
                    }
                }
            }
            else
            {
                st = prvUtfPortInterfaceModeGet(dev,port,&ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", dev, port);
                valid = GT_TRUE;
            }

            if (valid == GT_TRUE)
            {
                /*
                        1.1.1. Call with not NULL serdesCfgPtr.
                        Expected: GT_OK.
                */
                st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
                if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    continue;
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

                /*
                        1.1.2. Call with serdesCfgPtr [NULL].
                        Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, serdesCfgPtr = NULL", dev, port);
            }
            else
            {
                st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesManualRxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesManualRxConfigSet)
{
/*
    ITERATE_DEVICE_PHY_PORT (Lion2 and above)
    1.1.1. Call with valid values
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesConfigGet with not NULL serdesCfgPtr.
    Expected: GT_OK and the same serdesCfgPtr as was set.
    1.1.3. Call with values out of range
    Expected: NOT GT_OK.
    1.1.5. Call with serdesCfg [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32               laneNum;
    GT_BOOL              isSimulation;

    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32                          startSerdes;
    GT_U32                          numOfSerdesLanes;
    GT_U32                          boardIdx;
    GT_U32                          boardRevId;
    GT_U32                          reloadEeprom;

    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  serdesCfg;
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  serdesCfgGet;
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));
    cpssOsBzero((GT_VOID*) &serdesCfgGet, sizeof(serdesCfgGet));
    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    #ifdef ASIC_SIMULATION
          isSimulation = GT_TRUE;
    #else
          isSimulation = GT_FALSE;
    #endif
    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
      SKIP_TEST_MAC;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(!((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
             (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
             (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
            (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)) )
            && (GT_FALSE == PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(dev)))
        {
            SKIP_TEST_MAC
        }

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() &&PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                continue;
            }

            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes,
                                                   &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.5. Call with serdesCfg [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSerdesManualRxConfigSet(dev, port, startSerdes, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                "%d, %d, serdesCfgPtr = NULL", dev, port);

            for(laneNum = 0; laneNum < numOfSerdesLanes; laneNum++)
            {
                /* call with first group of the values */
                serdesCfg.sqlch = 68;
                serdesCfg.ffeRes = 0;
                serdesCfg.ffeCap = 0;
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
                {
                    serdesCfg.dcGain = 0;
                    serdesCfg.bandWidth = 0;
                    serdesCfg.loopBandwidth = 0;
                }
                else
                {
                    serdesCfg.sqlch = 31;
                    serdesCfg.align90 = 0;
                }

                st = cpssDxChPortSerdesManualRxConfigSet(dev, port, laneNum, &serdesCfg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call cpssDxChPortSerdesConfigGet with not NULL serdesCfgPtr.
                    Expected: GT_OK and the same serdesCfgPtr as was set.
                */
                st = cpssDxChPortSerdesManualRxConfigGet(dev, port, laneNum, &serdesCfgGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortSerdesManualRxConfigSet: %d, %d", dev, port);

                if((! PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) || (GT_FALSE == isSimulation))
                {
                     if (GT_OK == st)
                     {
                         UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.sqlch, serdesCfgGet.sqlch,
                             "get another serdesCfg.sqlch than was set:dev-%d,port-%d,set-%d,get-%d",
                             dev, port, serdesCfg.sqlch, serdesCfgGet.sqlch);
                         UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeRes, serdesCfgGet.ffeRes,
                             "get another serdesCfg.ffeRes than was set:dev-%d,port-%d,set-%d,get-%d",
                             dev, port, serdesCfg.ffeRes, serdesCfgGet.ffeRes);
                        UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeCap, serdesCfgGet.ffeCap,
                            "get another serdesCfg.ffeCap than was set:dev-%d,port-%d,set-%d,get-%d",
                            dev, port, serdesCfg.ffeCap, serdesCfgGet.ffeCap);
                        if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.dcGain, serdesCfgGet.dcGain,
                                "get another serdesCfg.dcGain than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.dcGain, serdesCfgGet.dcGain);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.bandWidth, serdesCfgGet.bandWidth,
                                "get another serdesCfg.bandWidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.bandWidth, serdesCfgGet.bandWidth);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth,
                                "get another serdesCfg.loopBandwidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.align90, serdesCfgGet.align90,
                                "get another serdesCfg.align90 than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.align90, serdesCfgGet.align90);
                        }
                    }
                }

                /* call with second group of the values */
                serdesCfg.sqlch = 31;
                serdesCfg.ffeRes = 7;
                serdesCfg.ffeCap = 15;
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
                {
                    serdesCfg.dcGain = 99;
                    serdesCfg.bandWidth = 13;
                    serdesCfg.loopBandwidth = 1;
                }
                else
                {
                    serdesCfg.align90 = 127;
                }

                st = cpssDxChPortSerdesManualRxConfigSet(dev, port, laneNum, &serdesCfg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call cpssDxChPortSerdesConfigGet with not NULL serdesCfgPtr.
                    Expected: GT_OK and the same serdesCfgPtr as was set.
                */
                st = cpssDxChPortSerdesManualRxConfigGet(dev, port, laneNum, &serdesCfgGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortSerdesConfigGet: %d, %d", dev, port);

                if((! PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) || (GT_FALSE == isSimulation))
                {
                    if (GT_OK == st)
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeRes, serdesCfgGet.ffeRes,
                            "get another serdesCfg.ffeRes than was set:dev-%d,port-%d,set-%d,get-%d",
                            dev, port, serdesCfg.ffeRes, serdesCfgGet.ffeRes);
                        UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeCap, serdesCfgGet.ffeCap,
                            "get another serdesCfg.ffeCap than was set:dev-%d,port-%d,set-%d,get-%d",
                            dev, port, serdesCfg.ffeCap, serdesCfgGet.ffeCap);
                        if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.dcGain, serdesCfgGet.dcGain,
                                "get another serdesCfg.dcGain than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.dcGain, serdesCfgGet.dcGain);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.bandWidth, serdesCfgGet.bandWidth,
                                "get another serdesCfg.bandWidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.bandWidth, serdesCfgGet.bandWidth);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth,
                                "get another serdesCfg.loopBandwidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.sqlch, serdesCfgGet.sqlch,
                                "get another serdesCfg.sqlch than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.sqlch, serdesCfgGet.sqlch);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.align90, serdesCfgGet.align90,
                                "get another serdesCfg.align90 than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.align90, serdesCfgGet.align90);
                        }
                    }
                }
            }

            /*
                1.1.3. Call with serdesCfg->txAmp [32] out of range, and other params from 1.1.1.
                Expected: NOT GT_OK.
            */
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
            {
                serdesCfg.sqlch = 311;
            }
            else
            {
                serdesCfg.sqlch = 32;
            }
            st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesManualRxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesManualRxConfigGet)
{
/*
    ITERATE_DEVICE_PHY_PORT (DxCh3 and above)
    1.1.1. Call with not NULL serdesCfgPtr.
    Expected: GT_OK.
    1.1.2. Call with serdesCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC serdesCfg;

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(!((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
             (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
             (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
            (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)) )
            && (GT_FALSE == PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(dev)))
        {
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }
            /*
                1.1.1. Call with not NULL serdesCfgPtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
            if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                continue;
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with serdesCfgPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, serdesCfgPtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPaddingEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPaddingEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortPaddingEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT      portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL     is_88E1690_RemotePort = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            if (portType == PRV_CPSS_PORT_CG_E)
            {
                continue;
            }
            is_88E1690_RemotePort = prvCpssDxChPortRemotePortCheck(dev, port);
            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable - GT_FALSE */


            if (is_88E1690_RemotePort != GT_TRUE) /* 88E1690 doesn't support disabling of padding */
            {
                enable = GT_FALSE;

                st = cpssDxChPortPaddingEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                /*
                  1.1.2. Call cpssDxChPortPaddingEnableSet with not-NULL enablePtr.
                  Expected: GT_OK and the same enable as was set.
                */
                st = cpssDxChPortPaddingEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPaddingEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
            }
            /* iterate with enable - GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChPortPaddingEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChPortPaddingEnableSet with not-NULL enablePtr.
                Expected: GT_OK and the same enable as was set.
            */
            st = cpssDxChPortPaddingEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPaddingEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPaddingEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPaddingEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPaddingEnableSet(dev, port, enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPaddingEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPaddingEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPaddingEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPaddingEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT      portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    GT_BOOL     enable    = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            if (portType == PRV_CPSS_PORT_CG_E)
            {
                continue;
            }
            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            st = cpssDxChPortPaddingEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call enablePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortPaddingEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPaddingEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPaddingEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPaddingEnableGet(dev, port, &enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPaddingEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPaddingEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortExcessiveCollisionDropEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortExcessiveCollisionDropEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable - GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChPortExcessiveCollisionDropEnableSet(dev, port, enable);
            if(GE_NOT_SUPPORTED(portTypeOptions))
            {/* this is GE specific feature */
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                continue;
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChPortExcessiveCollisionDropEnableGet with not-NULL enablePtr.
                Expected: GT_OK and the same enable as was set.
            */
            st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortExcessiveCollisionDropEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);

            /* iterate with enable - GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChPortExcessiveCollisionDropEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChPortExcessiveCollisionDropEnableGet with not-NULL enablePtr.
                Expected: GT_OK and the same enable as was set.
            */
            st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortExcessiveCollisionDropEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortExcessiveCollisionDropEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortExcessiveCollisionDropEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortExcessiveCollisionDropEnableSet(dev, port, enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortExcessiveCollisionDropEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortExcessiveCollisionDropEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortExcessiveCollisionDropEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enable);
            if(GE_NOT_SUPPORTED(portTypeOptions))
            {/* this is GE specific feature */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call enablePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortExcessiveCollisionDropEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXgLanesSwapEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPortXgLanesSwapEnableSet) */
GT_VOID cpssDxChPortXgLanesSwapEnableSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortXgLanesSwapEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            CPSS_PORT_SPEED_ENT speed;

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChPortSpeedGet(dev,port,/*OUT*/&speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (speed != CPSS_PORT_SPEED_NA_E)
            {
                /* Supported for XAUI or HyperG.Stack ports only */
                if( IS_PORT_XG_E(portType) )
                {
                    /*
                        1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                        Expected: GT_OK.
                    */
                    /* iterate with enable - GT_FALSE */
                    enable = GT_FALSE;

                    st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                        else
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) &&
                            (prvUtfPortMacModuloCalc(dev, port, 2) != 0))
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                    /*
                        1.1.2. Call cpssDxChPortPaddingEnableSet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enableGet);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortXgLanesSwapEnableGet: %d, %d", dev, port);
                            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
                        }
                    }
                    else if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) &&
                            (prvUtfPortMacModuloCalc(dev, port, 2) != 0))
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortXgLanesSwapEnableGet: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
                    }

                    /* iterate with enable - GT_TRUE */
                    enable = GT_TRUE;

                    st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                        else
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) &&
                            (prvUtfPortMacModuloCalc(dev, port, 2) != 0))
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }

                    else
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);


                    /*
                        1.1.2. Call cpssDxChPortXgLanesSwapEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enableGet);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortXgLanesSwapEnableGet: %d, %d", dev, port);
                            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
                        }
                    }
                    else if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) &&
                            (prvUtfPortMacModuloCalc(dev, port, 2) != 0))
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortXgLanesSwapEnableGet: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
                    }
                }
                else
                {
                    /* not supported NOT XG_E ports*/
                    enable = GT_FALSE;

                    st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                }
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXgLanesSwapEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXgLanesSwapEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Supported for XAUI or HyperG.Stack ports only */
            if( IS_PORT_XG_E(portType) )
            {
                /*
                    1.1.1. Call with not-NULL enable.
                    Expected: GT_OK.
                */
                st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enable);
                if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                    if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                }
                else if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) &&
                        (prvUtfPortMacModuloCalc(dev, port, 2) != 0))
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

                /*
                    1.1.2. Call enablePtr[NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortXgLanesSwapEnableGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
            }
            else
            {
                /* not supported NOT XG_E ports*/
                enable = GT_FALSE;

                st = cpssDxChPortXgLanesSwapEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXgLanesSwapEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInBandAutoNegBypassEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortInBandAutoNegBypassEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (!IS_PORT_XG_E(portType))
            {
                /* Set Inband Auto-Negotiation */
                if(prvCpssDxChPortRemotePortCheck(dev, port))
                {
                    st = cpssDxChPortInbandAutoNegEnableSet(dev, port, GT_TRUE);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                               "cpssDxChPortInbandAutoNegEnableSet: %d, %d, GT_TRUE", dev, port);
                    continue;
                }

                st = cpssDxChPortInbandAutoNegEnableSet(dev, port, GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChPortInbandAutoNegEnableSet: %d, %d, GT_TRUE", dev, port);

                /*
                    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                    Expected: GT_OK.
                */
                /* iterate with enable - GT_FALSE */
                enable = GT_FALSE;

                st = cpssDxChPortInBandAutoNegBypassEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                /*
                    1.1.2. Call cpssDxChPortInBandAutoNegBypassEnableGet with not-NULL enablePtr.
                    Expected: GT_OK and the same enable as was set.
                */
                st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInBandAutoNegBypassEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);

                /* iterate with enable - GT_TRUE */
                enable = GT_TRUE;

                st = cpssDxChPortInBandAutoNegBypassEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                /*
                    1.1.2. Call cpssDxChPortInBandAutoNegBypassEnableGet with not-NULL enablePtr.
                    Expected: GT_OK and the same enable as was set.
                */
                st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInBandAutoNegBypassEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortInBandAutoNegBypassEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInBandAutoNegBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInBandAutoNegBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInBandAutoNegBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInBandAutoNegBypassEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInBandAutoNegBypassEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (!IS_PORT_XG_E(portType))
            {
                /* Set Inband Auto-Negotiation */
                if(prvCpssDxChPortRemotePortCheck(dev, port))
                {
                    st = cpssDxChPortInbandAutoNegEnableSet(dev, port, GT_TRUE);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                               "cpssDxChPortInbandAutoNegEnableSet: %d, %d, GT_TRUE", dev, port);
                    continue;
                }

                st = cpssDxChPortInbandAutoNegEnableSet(dev, port, GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChPortInbandAutoNegEnableSet: %d, %d, GT_TRUE", dev, port);
                /*
                    1.1.1. Call with not-NULL enable.
                    Expected: GT_OK.
                */
                st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call enablePtr[NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInBandAutoNegBypassEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacErrorIndicationPortSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacErrorIndicationPortSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh)
    1.1.1. Call with port.
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortMacErrorIndicationPortGet with not-NULL portPtr.
    Expected: GT_OK and the same port as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_PHYSICAL_PORT_NUM       portGet = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PP_FAMILY_TYPE_ENT                 devFamily;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                            UTF_XCAT2_E | UTF_LION2_E |
                                             UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfDeviceFamilyGet(dev, &devFamily);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

            /*
                1.1.1. Call with not null port.
                Expected: GT_OK.
            */
            st = cpssDxChPortMacErrorIndicationPortSet(dev, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortInBandAutoNegBypassEnableGet
                       with not-NULL enablePtr.
                Expected: GT_OK and the same enable as was set.
            */
            st = cpssDxChPortMacErrorIndicationPortGet(dev, &portGet);

            /* get function works only for ch1/ch2*/
            if(devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
            else /* CPSS_PP_FAMILY_CHEETAH_E  or CPSS_PP_FAMILY_CHEETAH2_E */
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChPortMacErrorIndicationPortGet: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(port, portGet,
                        "got another port then was set: %d, %d", dev);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacErrorIndicationPortSet(dev, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacErrorIndicationPortSet(dev, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacErrorIndicationPortSet(dev, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                            UTF_XCAT2_E | UTF_LION2_E |
                                             UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacErrorIndicationPortSet(dev, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacErrorIndicationPortSet(dev, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacErrorIndicationPortGet
(
    IN  GT_U8      devNum,
    OUT GT_U8      *portNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacErrorIndicationPortGet)
{
/*
    ITERATE_DEVICES (DxCh)
    1.1.1. Call with not-NULL portPtr.
    Expected: GT_OK.
    1.1.2. Call portPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not-NULL portPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortMacErrorIndicationPortGet(dev, &port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with null portPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortMacErrorIndicationPortGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, port = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacErrorIndicationPortGet(dev, &port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacErrorIndicationPortGet(dev, &port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacErrorIndicationGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT   *typePtr,
    OUT GT_ETHERADDR                        *macPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacErrorIndicationGet)
{
/*
    ITERATE_DEVICES (DxCh)
    1.1. Call with not-NULL typePtr and not-NULL macPtr.
    Expected: GT_OK.
    1.2. Call typePtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call macPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT   type = CPSS_DXCH_PORT_MAC_ERROR_JABBER_E;
    GT_ETHERADDR                        mac;

    cpssOsBzero((GT_VOID*) &mac, sizeof(mac));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL typePtr and not-NULL macPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortMacErrorIndicationGet(dev, &type, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call typePtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortMacErrorIndicationGet(dev, NULL, &mac);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, typePtr = NULL", dev);

        /*
            1.3. Call macPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortMacErrorIndicationGet(dev, &type, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacErrorIndicationGet(dev, &type, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacErrorIndicationGet(dev, &type, &mac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXgPscLanesSwapSet
(
    IN GT_U8 devNum,
    IN GT_U8 portNum,
    IN GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS],
    IN GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS]
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPortXgPscLanesSwapSet) */
GT_VOID cpssDxChPortXgPscLanesSwapSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with rxSerdesLaneArr [0,1,2,3 / 0,1,2,3],
                     txSerdesLaneArr [0,1,2,3 / 3,2,1,0].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortXgPscLanesSwapGet with not NULL pointers.
    Expected: GT_OK and the same params as was set.
    1.1.3. Call with out of range rxSerdesLaneArr [0] [4]
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.4. Call with out of range txSerdesLaneArr [0] [4]
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.5. Call with rxSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.6. Call with txSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.7. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.1.8. Call with txSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];

    GT_U32 rxSerdesLaneArrGet[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArrGet[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_BOOL checkSuccess;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            CPSS_PORT_SPEED_ENT speed;


            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            st = cpssDxChPortSpeedGet(dev,port,/*OUT*/&speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (speed != CPSS_PORT_SPEED_NA_E)
            {
                /* Supported for XAUI or HyperG.Stack ports only! */
                if(portType == PRV_CPSS_PORT_XG_E)
                {
                    /*
                        1.1.1. Call with rxSerdesLaneArr [0,1,2,3 / 0,1,2,3],
                                         txSerdesLaneArr [0,1,2,3 / 3,2,1,0].
                        Expected: GT_OK.
                    */
                    /* iterate with 0 */
                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 3;

                    txSerdesLaneArr[0] = 0;
                    txSerdesLaneArr[1] = 1;
                    txSerdesLaneArr[2] = 2;
                    txSerdesLaneArr[3] = 3;

                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        }
                    }
                    else if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        }
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }


                    /*
                        1.1.2. Call cpssDxChPortXgPscLanesSwapGet with not NULL pointers.
                        Expected: GT_OK and the same params as was set.
                    */
                    checkSuccess = GT_FALSE;
                    st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArrGet, txSerdesLaneArrGet);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            checkSuccess = GT_TRUE;
                        }
                    }
                    else if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            checkSuccess = GT_TRUE;
                        }
                    }
                    else
                    {
                        checkSuccess = GT_TRUE;
                    }

                    if(checkSuccess)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        /* verifuing values */
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[0], rxSerdesLaneArrGet[0],
                                                     "got another rxSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[1], rxSerdesLaneArrGet[1],
                                                     "got another rxSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[2], rxSerdesLaneArrGet[2],
                                                     "got another rxSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[3], rxSerdesLaneArrGet[3],
                                                     "got another rxSerdesLaneArr[3] than was set: %d, %d", dev, port);

                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[0], txSerdesLaneArrGet[0],
                                                     "got another txSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[1], txSerdesLaneArrGet[1],
                                                     "got another txSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[2], txSerdesLaneArrGet[2],
                                                     "got another txSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[3], txSerdesLaneArrGet[3],
                                                     "got another txSerdesLaneArr[3] than was set: %d, %d", dev, port);
                    }

                    /* iterate with 3 */
                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 3;

                    txSerdesLaneArr[0] = 3;
                    txSerdesLaneArr[1] = 2;
                    txSerdesLaneArr[2] = 1;
                    txSerdesLaneArr[3] = 0;

                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        }
                    }
                    else if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        }
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }

                    /*
                        1.1.2. Call cpssDxChPortXgPscLanesSwapGet with not NULL pointers.
                        Expected: GT_OK and the same params as was set.
                    */
                    st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArrGet, txSerdesLaneArrGet);
                    if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            checkSuccess = GT_TRUE;
                        }
                    }
                    else if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            checkSuccess = GT_TRUE;
                        }
                    }
                    else
                    {
                        checkSuccess = GT_TRUE;
                    }

                    if(checkSuccess)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        /* verifuing values */
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[0], rxSerdesLaneArrGet[0],
                                                     "got another rxSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[1], rxSerdesLaneArrGet[1],
                                                     "got another rxSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[2], rxSerdesLaneArrGet[2],
                                                     "got another rxSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[3], rxSerdesLaneArrGet[3],
                                                     "got another rxSerdesLaneArr[3] than was set: %d, %d", dev, port);

                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[0], txSerdesLaneArrGet[0],
                                                     "got another txSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[1], txSerdesLaneArrGet[1],
                                                     "got another txSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[2], txSerdesLaneArrGet[2],
                                                     "got another txSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[3], txSerdesLaneArrGet[3],
                                                     "got another txSerdesLaneArr[3] than was set: %d, %d", dev, port);
                    }

                    /*
                        1.1.3. Call with out of range rxSerdesLaneArr [0] [4]
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    rxSerdesLaneArr[0] = 4;

                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, rxSerdesLaneArr[0] = %d",
                                                     dev, port, rxSerdesLaneArr[0]);

                    rxSerdesLaneArr[0] = 0;

                    /*
                        1.1.4. Call with out of range txSerdesLaneArr [0] [4]
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    txSerdesLaneArr[0] = 4;

                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, txSerdesLaneArr[0] = %d",
                                                     dev, port, txSerdesLaneArr[0]);

                    txSerdesLaneArr[0] = 3;

                    /*
                        1.1.5. Call with rxSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 2;

                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, rxSerdesLaneArr[2] = %d, rxSerdesLaneArr[3] = %d",
                                                     dev, port, rxSerdesLaneArr[2], rxSerdesLaneArr[3]);

                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 3;

                    /*
                        1.1.6. Call with txSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    txSerdesLaneArr[0] = 0;
                    txSerdesLaneArr[1] = 1;
                    txSerdesLaneArr[2] = 2;
                    txSerdesLaneArr[3] = 2;

                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, txSerdesLaneArr[2] = %d, txSerdesLaneArr[3] = %d",
                                                     dev, port, txSerdesLaneArr[2], txSerdesLaneArr[3]);

                    txSerdesLaneArr[0] = 0;
                    txSerdesLaneArr[1] = 1;
                    txSerdesLaneArr[2] = 2;
                    txSerdesLaneArr[3] = 3;

                    /*
                        1.1.3. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, NULL, txSerdesLaneArr);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, rxSerdesLaneArr = NULL", dev, port);

                    /*
                        1.1.4. Call with txSerdesLaneArr [NULL] and other params from 1.1.
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, txSerdesLaneArr = NULL", dev, port);
                }
            }
        }

        /*
            1.2. For all active devices go over all non available physical
            ports.
        */
        rxSerdesLaneArr[0] = 0;
        rxSerdesLaneArr[1] = 1;
        rxSerdesLaneArr[2] = 2;
        rxSerdesLaneArr[3] = 3;

        txSerdesLaneArr[0] = 0;
        txSerdesLaneArr[1] = 1;
        txSerdesLaneArr[2] = 2;
        txSerdesLaneArr[3] = 3;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* macSaLsb == 0 */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    rxSerdesLaneArr[0] = 0;
    rxSerdesLaneArr[1] = 1;
    rxSerdesLaneArr[2] = 2;
    rxSerdesLaneArr[3] = 3;

    txSerdesLaneArr[0] = 0;
    txSerdesLaneArr[1] = 1;
    txSerdesLaneArr[2] = 2;
    txSerdesLaneArr[3] = 3;

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssDxChPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXgPscLanesSwapGet
(
    IN GT_U8 devNum,
    IN GT_U8 portNum,
    OUT GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS],
    OUT GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXgPscLanesSwapGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with not NULL rxSerdesLaneArr and txSerdesLaneArr;
    Expected: GT_OK.
    1.1.2. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.1.3. Call with txSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* Supported for XAUI or HyperG.Stack ports only! */
            if(portType == PRV_CPSS_PORT_XG_E)
            {
                /*
                    1.1.1. Call with not NULL rxSerdesLaneArr and txSerdesLaneArr;
                    Expected: GT_OK.
                */
                st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                    if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                }
                else if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }


                /*
                    1.1.2. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortXgPscLanesSwapGet(dev, port, NULL, txSerdesLaneArr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, rxSerdesLaneArr = NULL", dev, port);

                /*
                    1.1.3. Call with txSerdesLaneArr [NULL] and other params from 1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, txSerdesLaneArr = NULL", dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* macSaLsb == 0 */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssDxChPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortBackPressureEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortBackPressureEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                  dev;
    GT_STATUS              st       = GT_OK;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                enable   = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;

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
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */

            st = cpssDxChPortBackPressureEnableGet(dev, port, &enable);
            if((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PORT_XG_E == portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortBackPressureEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortBackPressureEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortBackPressureEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortBackPressureEnableGet(dev, port, &enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortBackPressureEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortBackPressureEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCrcCheckEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCrcCheckEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

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
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortCrcCheckEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortCrcCheckEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortCrcCheckEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCrcCheckEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCrcCheckEnableGet(dev, port, &enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCrcCheckEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCrcCheckEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortDuplexAutoNegEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortDuplexAutoNegEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

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
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDuplexAutoNegEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortExtraIpgGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_U8    *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortExtraIpgGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null numberPtr.
        Expected: GT_OK.
        1.1.2. Call with numberPtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U8       number = 1;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
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
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortExtraIpgGet(dev, port, &number);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, number);
                continue;
            }

            /* 1.1.1. Call with non-null numberPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortExtraIpgGet(dev, port, &number);

            if(portMacType < PRV_CPSS_PORT_XG_E || portMacType ==PRV_CPSS_PORT_CG_E)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, number);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);
            }

            /* 1.1.2. Call with numberPtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortExtraIpgGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortExtraIpgGet(dev, port, &number);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortExtraIpgGet(dev, port, &number);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortExtraIpgGet(dev, port, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortExtraIpgGet(dev, port, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableGet
(
    IN   GT_U8       devNum,
    IN   GT_U8       portNum,
    OUT  GT_BOOL     *statePtr,
    OUT  GT_BOOL     *pauseAdvertisePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortFlowCntrlAutoNegEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state;
    GT_BOOL     pause;

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
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, NULL, &pause);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);

            st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &state, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInbandAutoNegEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInbandAutoNegEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;

    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portType  = PRV_CPSS_PORT_NOT_EXISTS_E;

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
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enable);
            if(((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PORT_XG_E == portType)) ||
                ((PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_CHEETAH3_E)
                                && (port >= 24)))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortInbandAutoNegEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInbandAutoNegEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortIpgBaseGet
(
    IN   GT_U8                        devNum,
    IN   GT_U8                        portNum,
    OUT  CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBase
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortIpgBaseGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                    CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
        Expected: GT_OK.
        1.1.2. Call with  [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                        dev;
    GT_STATUS                    st        = GT_OK;
    GT_PHYSICAL_PORT_NUM         port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_XG_FIXED_IPG_ENT   ipgBase;
    PRV_CPSS_PORT_TYPE_ENT       portType  = PRV_CPSS_PORT_NOT_EXISTS_E;

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
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortIpgBaseGet(dev, port, &ipgBase);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /*
               1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                           CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
               Expected: GT_OK.
            */
            st = cpssDxChPortIpgBaseGet(dev, port, &ipgBase);

            if(PRV_CPSS_PORT_XG_E <= portType && portType!=PRV_CPSS_PORT_CG_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            /* 1.1.2. Call with  [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortIpgBaseGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortIpgBaseGet(dev, port, &ipgBase);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortIpgBaseGet(dev, port, &ipgBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortIpgBaseGet(dev, port, &ipgBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortIpgBaseGet(dev, port, &ipgBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortIpgBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortIpgBaseSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh)
    1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortIpgBaseSet with not-NULL ipgBase.
    Expected: GT_OK and the same ipgBase as was set.
*/
    GT_STATUS                   st        = GT_OK;
    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT      portType  = PRV_CPSS_PORT_NOT_EXISTS_E;
    CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase   = CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;
    CPSS_PORT_XG_FIXED_IPG_ENT  ipgBaseGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);
            /*
                1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                            CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
                Expected: GT_OK.
            */
            ipgBase = CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortIpgBaseSet: %d, %d, GT_TRUE", dev, port);

            ipgBase = CPSS_PORT_XG_FIXED_IPG_12_BYTES_E;

            st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortIpgBaseSet: %d, %d, GT_TRUE", dev, port);

            /*
                1.1.2. Call cpssDxChPortIpgBaseGet with not-NULL ipgBase.
                Expected: GT_OK and the same ipgBase as was set.
            */
            st = cpssDxChPortIpgBaseGet(dev, port, &ipgBaseGet);

            if(PRV_CPSS_PORT_XG_E <= portType && portType !=PRV_CPSS_PORT_CG_E)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortIpgBaseGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(ipgBase, ipgBaseGet,
                    "got another ipgBase then was set: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            /*  1.1.3. Call with wrong enum values ipgBase.   */
            /*  Expected: GT_BAD_PARAM.                     */
            if(PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChPortIpgBaseSet
                                    (dev, port, ipgBase),
                                    ipgBase);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port    = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        ipgBase = CPSS_PORT_XG_FIXED_IPG_12_BYTES_E;

        st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port    = PORT_CTRL_VALID_PHY_PORT_CNS;
    ipgBase = CPSS_PORT_XG_FIXED_IPG_12_BYTES_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    ipgBase = CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;

    st = cpssDxChPortIpgBaseSet(dev, port, ipgBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMruGet
(
    IN   GT_U8                        devNum,
    IN   GT_U8                        portNum,
    OUT  GT_U32                      *mruSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMruGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with not null mruSizePtr.
        Expected: GT_OK.
        1.1.2. Call with null mruSizePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                        dev;
    GT_STATUS                    st     = GT_OK;
    GT_PHYSICAL_PORT_NUM         port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                       mruSize;

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
                1.1.1. Call with not null mruSizePtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortMruGet(dev, port, &mruSize);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with null mruSizePtr [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssDxChPortMruGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortMruGet(dev, port, &mruSize);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMruGet(dev, port, &mruSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMruGet(dev, port, &mruSize);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMruGet(dev, port, &mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMruGet(dev, port, &mruSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPeriodicFcEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPeriodicFcEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT     enable = GT_FALSE;

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
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortPeriodicFcEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPeriodicFcEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPreambleLengthGet
(
    IN  GT_U8                    devNum,
    IN  GT_U8                    portNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_U32                  *length
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPreambleLengthGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E].
    Expected: GT_OK for GE ports and not GT_OK for others.
    1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E].
    Expected: GT_OK for XG 10Gbps ports and not GT_OK for others.
    1.1.3. Call with direction [CPSS_PORT_DIRECTION_BOTH_E].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong enum values direction.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with length [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_U8                    dev;
    GT_STATUS                st        = GT_OK;
    GT_PHYSICAL_PORT_NUM     port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT   portType  = PRV_CPSS_PORT_NOT_EXISTS_E;
    CPSS_PORT_DIRECTION_ENT  direction = CPSS_PORT_DIRECTION_RX_E;
    GT_U32                   length;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily;
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /*
                1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E].
                Expected: GT_OK
            */

            direction = CPSS_PORT_DIRECTION_TX_E;

            st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);
            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
            }

            /*
                1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E].
                Expected: GT_OK for XG ports and GT_NOT_SUPPORTED for others.
            */

            direction = CPSS_PORT_DIRECTION_RX_E;

            st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);

            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                   (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
                {
                    static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
                    GT_BOOL                            isSupported;
                    GT_STATUS                          rc;
                    GT_BOOL                            valid;

                    if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);
                        valid = portMapShadowPtr->valid;
                    }
                    else
                    {
                        valid = GT_TRUE;
                    }

                    if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
                    {
                        rc = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                                    PRV_CPSS_PORT_XLG_E,
                                                                    /*OUT*/&isSupported);
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);
                        if (isSupported == GT_TRUE &&
                            /* not remote 88E1690 port */
                            !prvCpssDxChPortRemotePortCheck(dev, port))
                        {
                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                            /*we don't call get for direction = CPSS_PORT_DIRECTION_BOTH_E */
                        }
                        else
                        {
                            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                        }
                    }
                    else
                    {
                        /* port is not mapped */
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                    }
                }
                else
                {
                    if(PRV_CPSS_PORT_XG_E == portType)
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                    }
                }
            }

            /*
                1.1.3. Call with direction [CPSS_PORT_DIRECTION_BOTH_E].
                Expected: GT_BAD_PARAM.
            */

            direction = CPSS_PORT_DIRECTION_BOTH_E;

            st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, length);


            /*
                1.1.4. Call with wrong enum values direction.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPreambleLengthGet
                                (dev, port, direction, &length),
                                direction);

            /*
               1.1.5. Call with  NULL lenghtGet [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortPreambleLengthGet(dev, port, direction, NULL);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, port, direction, length);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        direction = CPSS_PORT_DIRECTION_TX_E;
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPreambleLengthGet(dev, port, direction, &length);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSpeedAutoNegEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSpeedAutoNegEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

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
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortSpeedAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortSpeedAutoNegEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortSpeedAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSpeedAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSpeedAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSpeedAutoNegEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXGmiiModeGet
(
    IN  GT_U8                     devNum,
    IN  GT_U8                     portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT *modePtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPortXGmiiModeGet) */
GT_VOID cpssDxChPortXGmiiModeGetUT(GT_VOID)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh)
        1.1.1. Call with non-null modePtr.
        Expected: GT_OK.
        1.1.2. Call with modePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                    dev;
    GT_STATUS                st       = GT_OK;
    GT_PHYSICAL_PORT_NUM     port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_XGMII_MODE_ENT mode;
    PRV_CPSS_PORT_TYPE_ENT   portType = PRV_CPSS_PORT_NOT_EXISTS_E;

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
            /* 1.1.1. Call with non-null modePtr.
               Expected: GT_OK.
            */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortXGmiiModeGet(dev, port, &mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
                continue;
            }

            st = cpssDxChPortXGmiiModeGet(dev, port, &mode);

            if(PRV_CPSS_PORT_XG_E <= portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, mode);
            }
            /* 1.1.2. Call with modePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortXGmiiModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortXGmiiModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXGmiiModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXGmiiModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXGmiiModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupSerdesPowerStatusSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     portSerdesGroup,
    IN  GT_BOOL    powerUp
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupSerdesPowerStatusSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh3 and above)
    1.1. Call with portSerdesGroup[0] and powerUp[GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call with wrong enum values portSerdesGroup.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st        = GT_OK;
    GT_U8      dev;
    GT_U32     portSerdesGroup = 0;
    GT_BOOL    powerUp = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_XCAT3_E | UTF_LION2_E \
                                     | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with portSerdesGroup[0] and powerUp[GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */

        portSerdesGroup = 0;

        st = prvUtfPortTypeGet(dev, portSerdesGroup, &portType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portSerdesGroup);

        st = cpssDxChPortGroupSerdesPowerStatusSet(dev, portSerdesGroup, powerUp);
        if (IS_PORT_FE_E(portType))
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enum values portSerdesGroup.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupSerdesPowerStatusSet
                            (dev, portSerdesGroup, powerUp),
                            portSerdesGroup);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupSerdesPowerStatusSet(dev, portSerdesGroup, powerUp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupSerdesPowerStatusSet(dev, portSerdesGroup, powerUp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesGroupGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    OUT GT_U32   *portSerdesGroupPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesGroupGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
        1.1.1. Call with non-null portSerdesGroupPtr.
        Expected: GT_OK.
        1.1.2. Call with portSerdesGroupPtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                    dev;
    GT_STATUS                st       = GT_OK;
    GT_PHYSICAL_PORT_NUM     port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                   portSerdesGroup;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_LION2_E \
                                     | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null portSerdesGroupPtr.
               Expected: GT_OK.
            */

            st = cpssDxChPortSerdesGroupGet(dev, port, &portSerdesGroup);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portSerdesGroup);

            /*
               1.1.2. Call with portSerdesGroupPtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortSerdesGroupGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortSerdesGroupGet(dev, port, &portSerdesGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesGroupGet(dev, port, &portSerdesGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesGroupGet(dev, port, &portSerdesGroup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesGroupGet(dev, port, &portSerdesGroup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesPowerStatusSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with
    Expected: GT_OK.
    */

    GT_STATUS   st     = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT                 devFamily;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PORT_DIRECTION_ENT direction = 0;
    GT_U32                  lanesBmp = 0;
    GT_BOOL                 powerUp = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT     speed;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with direction [CPSS_PORT_DIRECTION_RX_E /
                                           CPSS_PORT_DIRECTION_TX_E /
                                           CPSS_PORT_DIRECTION_BOTH_E],
                              and powerUp [GT_TRUE / GT_FALSE].
               Expected: GT_OK.
            */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);

            st = cpssDxChPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortSpeedGet: %d, %d",
                                         dev, port);

            /*call with powerUp = GT_FALSE */
            powerUp = GT_FALSE;

            /* call with direction = CPSS_PORT_DIRECTION_RX_E */
            direction = CPSS_PORT_DIRECTION_RX_E;

            st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                                  lanesBmp, powerUp);

            /* CH3 Gig ports does not supports this API */
            if(((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) && (portType == PRV_CPSS_PORT_GE_E)) ||
               (portType == PRV_CPSS_PORT_FE_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else if(CPSS_PP_FAMILY_CHEETAH3_E <= devFamily)
                {/* only BOTH supported */
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                /* the function is supported for XG ports only */
                if (portType != PRV_CPSS_PORT_XG_E)
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }

            /* call with direction = CPSS_PORT_DIRECTION_TX_E */
            direction = CPSS_PORT_DIRECTION_TX_E;

            st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                                  lanesBmp, powerUp);

            /* CH3 Gig ports does not supports this API */
            if(((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) && (portType == PRV_CPSS_PORT_GE_E)) ||
               (portType == PRV_CPSS_PORT_FE_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else if(CPSS_PP_FAMILY_CHEETAH3_E <= devFamily)
                {/* only BOTH supported */
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }

            /* call with direction = CPSS_PORT_DIRECTION_BOTH_E */
            direction = CPSS_PORT_DIRECTION_BOTH_E;

            st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                                  lanesBmp, powerUp);

            /* CH3 Gig ports does not supports this API */
            if(((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) && (portType == PRV_CPSS_PORT_GE_E)) ||
               (portType == PRV_CPSS_PORT_FE_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else if(devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }

            if(devFamily > CPSS_PP_FAMILY_DXCH_LION_E)
            {
                /* restore ifMode and speed configurations of port after it was powered down */
                st = cpssDxChPortInterfaceModeSet(dev, port, ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeSet: %d, %d",
                                             dev, port);

                st = cpssDxChPortSpeedSet(dev, port, speed);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortSpeedSet: %d, %d",
                                             dev, port);
            }

            /*call with powerUp = GT_TRUE */
            powerUp = GT_TRUE;

            /* call with direction = CPSS_PORT_DIRECTION_RX_E */
            direction = CPSS_PORT_DIRECTION_RX_E;

            st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                                  lanesBmp, powerUp);
            if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* CH3 Gig ports does not supports this API */
            else if(((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) && (portType == PRV_CPSS_PORT_GE_E)) ||
               (portType == PRV_CPSS_PORT_FE_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if(CPSS_PP_FAMILY_CHEETAH3_E <= devFamily)
                {/* only BOTH supported */
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }

            /* call with direction = CPSS_PORT_DIRECTION_TX_E */
            direction = CPSS_PORT_DIRECTION_TX_E;

            st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                                  lanesBmp, powerUp);
            if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* CH3 Gig ports does not supports this API */
            else if(((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) && (portType == PRV_CPSS_PORT_GE_E)) ||
               (portType == PRV_CPSS_PORT_FE_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if(CPSS_PP_FAMILY_CHEETAH3_E <= devFamily)
                {/* only BOTH supported */
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
            /* call with direction = CPSS_PORT_DIRECTION_BOTH_E */
            direction = CPSS_PORT_DIRECTION_BOTH_E;

            st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                                  lanesBmp, powerUp);
            if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* CH3 Gig ports does not supports this API */
            else if(((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) && (portType == PRV_CPSS_PORT_GE_E)) ||
               (portType == PRV_CPSS_PORT_FE_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if(devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                                  lanesBmp, powerUp);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }


        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                              lanesBmp, powerUp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                              lanesBmp, powerUp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                              lanesBmp, powerUp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesPowerStatusSet(dev, port, direction,
                                          lanesBmp, powerUp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFlowControlModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT  fcMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortFlowControlModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion)
    1.1.1. Call with fcMode [CPSS_DXCH_PORT_FC_MODE_802_3X_E /
                             CPSS_DXCH_PORT_FC_MODE_PFC_E /
                             CPSS_DXCH_PORT_FC_MODE_LL_FC_E]
    Expected: GT_OK.
    1.1.2. Call with cpssDxChPortFlowControlModeGet with the same fcMode.
    Expected: GT_OK and the same fcMode.
    1.1.3. Call with wrong enum values fcMode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS            st   = GT_OK;

    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_DXCH_PORT_FC_MODE_ENT fcMode = CPSS_DXCH_PORT_FC_MODE_802_3X_E;
    CPSS_DXCH_PORT_FC_MODE_ENT fcModeGet;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with fcMode [CPSS_DXCH_PORT_FC_MODE_802_3X_E /
                                         CPSS_DXCH_PORT_FC_MODE_PFC_E /
                                         CPSS_DXCH_PORT_FC_MODE_LL_FC_E]
               Expected: GT_OK.
            */

            st = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            fcMode = CPSS_DXCH_PORT_FC_MODE_802_3X_E;

            st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
            if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_FC_802_3X_NOT_SUPPORTED_TM_MAPPED_PORTS_WA_E)) &&
               (portMapShadowPtr->portMap.trafficManagerEn))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, fcMode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);
            }

            if (GT_OK == st)
            {
                /*
                    1.1.2. Call with cpssDxChPortFlowControlModeGet with the same fcMode.
                    Expected: GT_OK and the same fcMode.
                */

                st = cpssDxChPortFlowControlModeGet(dev, port, &fcModeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

                UTF_VERIFY_EQUAL2_STRING_MAC(fcMode, fcModeGet,
                            "get another fcMode value than was set: %d, %d", dev, port);
            }

            /*
                1.1.1. Call with fcMode [CPSS_DXCH_PORT_FC_MODE_802_3X_E /
                                         CPSS_DXCH_PORT_FC_MODE_PFC_E /
                                         CPSS_DXCH_PORT_FC_MODE_LL_FC_E]
               Expected: GT_OK.
            */

            fcMode = CPSS_DXCH_PORT_FC_MODE_PFC_E;

            st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
            if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_PFC_NOT_SUPPORTED_PORTS_48_71_WA_E)) &&
               (portMapShadowPtr->portMap.macNum >= 48) && (portMapShadowPtr->portMap.macNum <= 71))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, fcMode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);
            }

            if (GT_OK == st)
            {
                /*
                    1.1.2. Call with cpssDxChPortFlowControlModeGet with the same fcMode.
                    Expected: GT_OK and the same fcMode.
                */

                st = cpssDxChPortFlowControlModeGet(dev, port, &fcModeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

                UTF_VERIFY_EQUAL2_STRING_MAC(fcMode, fcModeGet,
                            "get another fcMode value than was set: %d, %d", dev, port);
            }

            /*
                1.1.1. Call with fcMode [CPSS_DXCH_PORT_FC_MODE_802_3X_E /
                                         CPSS_DXCH_PORT_FC_MODE_PFC_E /
                                         CPSS_DXCH_PORT_FC_MODE_LL_FC_E]
               Expected: GT_OK.
            */

            fcMode = CPSS_DXCH_PORT_FC_MODE_LL_FC_E;

            st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, fcMode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);
            }

            if (GT_OK == st)
            {
                /*
                    1.1.2. Call with cpssDxChPortFlowControlModeGet with the same fcMode.
                    Expected: GT_OK and the same fcMode.
                */

                st = cpssDxChPortFlowControlModeGet(dev, port, &fcModeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

                UTF_VERIFY_EQUAL2_STRING_MAC(fcMode, fcModeGet,
                            "get another fcMode value than was set: %d, %d", dev, port);
            }

            /*
               1.1.3. Call with wrong enum values fcMode
               Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortFlowControlModeSet
                                (dev, port, fcMode),
                                fcMode);
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
            /* fcMode [CPSS_PORT_FULL_DUPLEX_E]               */
            st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, fcMode);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, fcMode == CPSS_PORT_FULL_DUPLEX_E */

    st = cpssDxChPortFlowControlModeSet(dev, port, fcMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFlowControlModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT  *fcModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortFlowControlModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion)
    1.1.1. Call with non-null fcModePtr.
    Expected: GT_OK.
    1.1.2. Call with fcModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS            st   = GT_OK;

    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_DXCH_PORT_FC_MODE_ENT fcMode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null fcModePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortFlowControlModeGet(dev, port, &fcMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with fcModePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortFlowControlModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortFlowControlModeGet(dev, port, &fcMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortFlowControlModeGet(dev, port, &fcMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortFlowControlModeGet(dev, port, &fcMode);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFlowControlModeGet(dev, port, &fcMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortFlowControlModeGet(dev, port, &fcMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesCalibrationStartSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesCalibrationStartSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (xCat and above)
    1.1.1. Call with port.
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E | UTF_BOBCAT2_E |  UTF_CAELUM_E | UTF_ALDRIN_E | UTF_XCAT3_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);

            /*
                1.1.1. Call with not null port.
                Expected: GT_NOT_SUPPORTED for FE ports, GT_OK for other ports.
            */
            st = cpssDxChPortSerdesCalibrationStartSet(dev, port);
            if (IS_PORT_FE_E(portType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesCalibrationStartSet(dev, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesCalibrationStartSet(dev, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesCalibrationStartSet(dev, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesCalibrationStartSet(dev, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesCalibrationStartSet(dev, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacResetStateSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

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
               1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE */
            enable = GT_TRUE;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortMacResetStateSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                continue;
            }

            st = cpssDxChPortMacResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* Call function with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChPortMacResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssDxChPortMacResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacResetStateSet(dev, port, enable);
        if(!prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortMacResetStateSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesResetStateSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                         dev, port);
            /*
               1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE */
            enable = GT_TRUE;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                continue;
            }

            st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
            if (IS_PORT_FE_E(portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
            }
            else
            {
                if((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    || ((0 == PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev)) && (ifMode == CPSS_PORT_INTERFACE_MODE_XGMII_E) && (port%2 != 0)))
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /* Call function with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
            if (IS_PORT_FE_E(portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
            }
            else
            {
                if((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    || ((0 == PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev)) && (ifMode == CPSS_PORT_INTERFACE_MODE_XGMII_E) && (port%2 != 0)))
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortSerdesResetStateSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForward802_3xEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForward802_3xEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortForward802_3xEnableGet(dev, port, &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortForward802_3xEnableGet(dev, port, &enable);
            if(!IS_PORT_XG_E(portType))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) ?
                                                GT_NOT_SUPPORTED : GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortForward802_3xEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortForward802_3xEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForward802_3xEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForward802_3xEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForward802_3xEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortForward802_3xEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForward802_3xEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForward802_3xEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortForward802_3xEnableGet.
    Expected: GT_OK and the same enable value as was set.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_TRUE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with enable [GT_FALSE] */

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            enable = GT_FALSE;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);
            if(!IS_PORT_XG_E(portType))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) ?
                                                GT_NOT_SUPPORTED : GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssDxChPortForward802_3xEnableGet(dev, port, &enableGet);
            if(IS_PORT_XG_E(portType) && (devFamily == CPSS_PP_FAMILY_CHEETAH3_E))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_SUPPORTED, st,
                        "cpssDxChPortForward802_3xEnableGet: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChPortForward802_3xEnableGet: %d", dev);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                        "get another enable than was set: %d, %d", dev, port);
            }

            /* Call function with enable [GT_TRUE] */
            enable = GT_TRUE;
            st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);

            if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_FC_FORWARD_NOT_FUNCTIONAL_WA_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if (!IS_PORT_XG_E(portType))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC((devFamily == CPSS_PP_FAMILY_CHEETAH3_E) ?
                                                GT_NOT_SUPPORTED : GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssDxChPortForward802_3xEnableGet(dev, port, &enableGet);

            if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_FC_FORWARD_NOT_FUNCTIONAL_WA_E))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChPortForward802_3xEnableGet: %d", dev);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, enableGet,
                        "get another enable than was set: %d, %d", dev, port);
            }
            else if(IS_PORT_XG_E(portType) && (devFamily == CPSS_PP_FAMILY_CHEETAH3_E))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_SUPPORTED, st,
                        "cpssDxChPortForward802_3xEnableGet: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChPortForward802_3xEnableGet: %d", dev);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                        "get another enable than was set: %d, %d", dev, port);
            }
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE */
            st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortForward802_3xEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForwardUnknownMacControlFramesEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat2)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortForwardUnknownMacControlFramesEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat2)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortForwardUnknownMacControlFramesEnableGet.
    Expected: GT_OK and the same enable value as was set.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_TRUE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with enable [GT_FALSE] */
            enable = GT_FALSE;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChPortForwardUnknownMacControlFramesEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);

            /* Call function with enable [GT_TRUE] */
            enable = GT_TRUE;
            st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssDxChPortForwardUnknownMacControlFramesEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChPortForwardUnknownMacControlFramesEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE */
            st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortModeSpeedAutoDetectAndConfig
(
    IN   GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN   CPSS_PORT_MODE_SPEED_STC *portModeSpeedOptionsArrayPtr,
    IN   GT_U8                    optionsArrayLen,
    OUT  CPSS_PORT_MODE_SPEED_STC *currentModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortModeSpeedAutoDetectAndConfig)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion, XCAT)
    1.1.1. Call with portModeSpeedOptionsArray[] =
            {
             {CPSS_PORT_INTERFACE_MODE_RXAUI_E      , CPSS_PORT_SPEED_10000_E  },
             {CPSS_PORT_INTERFACE_MODE_XGMII_E      , CPSS_PORT_SPEED_10000_E  },
             {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E , CPSS_PORT_SPEED_1000_E}
            };
            optionsArrayLen [3];
            and not NULL currentModePtr.
    Expected: GT_OK.
    1.1.2. Call with portModeSpeedOptionsArray[] =
            {
             {CPSS_PORT_INTERFACE_MODE_RXAUI_E      , CPSS_PORT_SPEED_10000_E  },
             {CPSS_PORT_INTERFACE_MODE_XGMII_E      , CPSS_PORT_SPEED_10000_E  },
             {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E , CPSS_PORT_SPEED_20000_E  }
            }; (not supported)
            and other valid parameters
    Expected: NOT GT_OK.
    1.1.3. Call with wrong enum values portModeSpeedOptionsArray[0].speed
                     and other valid parameters
    Expected: GT_BAD_PARAM
    1.1.4. Call with wrong enum values portModeSpeedOptionsArray[1].ifMode
                     and other valid parameters
    Expected: GT_BAD_PARAM
    1.1.5. Call with portModeSpeedOptionsArrayPtr [NULL] (any of supported options acceptable)
                     optionsArrayLen [0]
                     and other valid parameters
    Expected: GT_OK
    1.1.6. Call with portModeSpeedOptionsArrayPtr [NULL] (any of supported options acceptable)
                     optionsArrayLen [1] (must be 0 if portModeSpeedOptionsArrayPtr == NULL)
                     and other valid parameters
    Expected: NOT GT_OK.
    1.1.7. Call with currentModePtr [NULL]
                     and other valid parameters
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PORT_MODE_SPEED_STC portModeSpeedOptionsArray[] =
        {
            {CPSS_PORT_INTERFACE_MODE_RXAUI_E      , CPSS_PORT_SPEED_10000_E},
            {CPSS_PORT_INTERFACE_MODE_XGMII_E      , CPSS_PORT_SPEED_10000_E},
            {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E , CPSS_PORT_SPEED_1000_E }
        };
    GT_U8                    optionsArrayLen;
    CPSS_PORT_MODE_SPEED_STC currentMode;
    PRV_CPSS_PORT_TYPE_ENT   portType;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    optionsArrayLen = sizeof(portModeSpeedOptionsArray) / sizeof(CPSS_PORT_MODE_SPEED_STC);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = prvUtfPortTypeOptionsGet(dev, port, &portTypeOptions);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if(PRV_CPSS_PORT_FE_E == portType)
            {   /* auto-detect not supported for FE ports */
                st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, NULL, 0,
                                                              &currentMode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
                continue;
            }

            /*
                1.1.1. Call with currentModePtr is not NULL;
                        portModeSpeedOptionsArray[] = {
                         {CPSS_PORT_INTERFACE_MODE_RXAUI_E      , CPSS_PORT_SPEED_10000_E  },
                         {CPSS_PORT_INTERFACE_MODE_XGMII_E      , CPSS_PORT_SPEED_10000_E  },
                         {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E , CPSS_PORT_SPEED_1000_E}
                        };
                        optionsArrayLen [3];
                        and not NULL currentModePtr.
                Expected: GT_OK.
            */
            if(portTypeOptions == PRV_CPSS_GE_PORT_GE_ONLY_E)
            {
                portModeSpeedOptionsArray[0].ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
                portModeSpeedOptionsArray[0].speed  = CPSS_PORT_SPEED_1000_E;
                portModeSpeedOptionsArray[1].ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                portModeSpeedOptionsArray[1].speed = CPSS_PORT_SPEED_1000_E;
                optionsArrayLen = 2;
            }
            else
            {
                portModeSpeedOptionsArray[0].ifMode = CPSS_PORT_INTERFACE_MODE_RXAUI_E;
                portModeSpeedOptionsArray[0].speed  = CPSS_PORT_SPEED_10000_E;
                portModeSpeedOptionsArray[1].ifMode = CPSS_PORT_INTERFACE_MODE_XGMII_E;
                portModeSpeedOptionsArray[1].speed = CPSS_PORT_SPEED_10000_E;
                portModeSpeedOptionsArray[2].ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                portModeSpeedOptionsArray[2].speed = CPSS_PORT_SPEED_1000_E;
                optionsArrayLen = 3;
            }

            st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port,
                                                          portModeSpeedOptionsArray,
                                                          optionsArrayLen, &currentMode);

            if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PP_MAC(dev)->revision <= 2))
            {/* xcat older then A2 doesn't support RXAUI */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with currentModePtr is not NULL;
                        portModeSpeedOptionsArray[] = {
                         {CPSS_PORT_INTERFACE_MODE_RXAUI_E      , CPSS_PORT_SPEED_20000_E  },
                         {CPSS_PORT_INTERFACE_MODE_XGMII_E      , CPSS_PORT_SPEED_10000_E  },
                         {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E , CPSS_PORT_SPEED_1000_E  }
                        }; (not supported)
                        and other valid parameters
                Expected: NOT GT_OK.
            */
            portModeSpeedOptionsArray[0].speed = CPSS_PORT_SPEED_20000_E;

            st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port,
                                                          portModeSpeedOptionsArray,
                                                          optionsArrayLen, &currentMode);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore speed value */
            portModeSpeedOptionsArray[0].speed =
                (portTypeOptions ==
                        PRV_CPSS_GE_PORT_GE_ONLY_E) ? CPSS_PORT_SPEED_1000_E
                                                        : CPSS_PORT_SPEED_10000_E;

            /*
               1.1.3. Call with wrong enum values portModeSpeedOptionsArray[0].speed
                                 and other valid parameters
                Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortModeSpeedAutoDetectAndConfig
                                (dev, port, portModeSpeedOptionsArray,
                                 optionsArrayLen, &currentMode),
                                portModeSpeedOptionsArray[0].speed);
            /*
               1.1.4. Call with wrong enum values portModeSpeedOptionsArray[1].ifMode
                                 and other valid parameters
                Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortModeSpeedAutoDetectAndConfig
                                (dev, port, portModeSpeedOptionsArray,
                                 optionsArrayLen, &currentMode),
                                portModeSpeedOptionsArray[0].ifMode);

            /*
               1.1.5. Call with portModeSpeedOptionsArrayPtr [NULL] (any of supported options acceptable)
                                 optionsArrayLen [0]
                                 and other valid parameters
                Expected: GT_OK
            */
            if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
                && (PRV_CPSS_PP_MAC(dev)->revision <= 2))
            {/* xcat older then A3 doesn't support RXAUI */
                portModeSpeedOptionsArray[0].ifMode = CPSS_PORT_INTERFACE_MODE_XGMII_E;
                portModeSpeedOptionsArray[0].speed = CPSS_PORT_SPEED_10000_E;
                portModeSpeedOptionsArray[1].ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                portModeSpeedOptionsArray[1].speed = CPSS_PORT_SPEED_1000_E;
                optionsArrayLen = 2;
                st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, portModeSpeedOptionsArray,
                                                                optionsArrayLen, &currentMode);
            }
            else
            {
                st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, NULL, 0, &currentMode);
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.7. Call with currentModePtr [NULL]
                                 and other valid parameters
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port,
                                                          portModeSpeedOptionsArray,
                                                          optionsArrayLen, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, currentModePtr = NULL", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /*
                1.2.1. Call function for each non-active port
                Expected: GT_BAD_PARAM
            */
            st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, portModeSpeedOptionsArray,
                                                          optionsArrayLen, &currentMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, portModeSpeedOptionsArray,
                                                      optionsArrayLen, &currentMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, portModeSpeedOptionsArray,
                                                      optionsArrayLen, &currentMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, portModeSpeedOptionsArray,
                                                      optionsArrayLen, &currentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortModeSpeedAutoDetectAndConfig(dev, port, portModeSpeedOptionsArray,
                                                  optionsArrayLen, &currentMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    IN  GT_U32                              value
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPeriodicFlowControlIntervalSet)
{
/*
    1.1. Call with value [0, 0xCCC00, 0xFFFFFFFF / 25 ].
    Expected: GT_OK.
    1.2. Call cpssDxChPortMacSaLsbGet with non-NULL value.
    Expected: GT_OK and value the same as just written.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType = CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E;
    GT_U32      value    = 0;
    GT_U32      valueGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* iterate with value = 0 */
        value = 0;
        /* 1.1. Call with value = 0.
           Expected: GT_OK.
        */
        st = cpssDxChPortPeriodicFlowControlIntervalSet(dev,PORT_CTRL_VALID_PHY_PORT_CNS,portType, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portType, value);

        /*
            1.2. Call cpssDxChPortPeriodicFlowControlIntervalGet with non-NULL value.
            Expected: GT_OK and value the same as just written.
        */
        st = cpssDxChPortPeriodicFlowControlIntervalGet(dev, PORT_CTRL_VALID_PHY_PORT_CNS,portType, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPeriodicFlowControlIntervalGet: %d, %d", dev, portType);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(value, valueGet,
                                     "got another value as was written: %d, %d", dev, portType);

        if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* iterate with value = 0xFFFFFFFF / 25 */
            value = 0xFFFFFFFF / 25 ;
        }
        else
        {
            /*max value for SIP6*/
            value = 0xFFFE;
        }

        st = cpssDxChPortPeriodicFlowControlIntervalSet(dev,PORT_CTRL_VALID_PHY_PORT_CNS, portType, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portType, value);

        /*
            1.2. Call cpssDxChPortPeriodicFlowControlIntervalGet with non-NULL value.
            Expected: GT_OK and value the same as just written.
        */
        st = cpssDxChPortPeriodicFlowControlIntervalGet(dev,PORT_CTRL_VALID_PHY_PORT_CNS, portType, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPeriodicFlowControlIntervalGet: %d, %d", dev, portType);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(value, valueGet,
                                     "got another value as was written: %d, %d", dev, portType);

        /* iterate with value = 0xCCC00 */
        value = 0xCCC00;
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            value>>=4;
        }

        st = cpssDxChPortPeriodicFlowControlIntervalSet(dev, PORT_CTRL_VALID_PHY_PORT_CNS,portType, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portType, value);

        /*
            1.2. Call cpssDxChPortPeriodicFlowControlIntervalGet with non-NULL value.
            Expected: GT_OK and value the same as just written.
        */
        st = cpssDxChPortPeriodicFlowControlIntervalGet(dev, PORT_CTRL_VALID_PHY_PORT_CNS,portType, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPeriodicFlowControlIntervalGet: %d, %d", dev, portType);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(value, valueGet,
                                     "got another value as was written: %d, %d", dev, portType);

    }
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPeriodicFlowControlIntervalGet)
{
/*
    1.1. Call with valid value
    portType[CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E/
             CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E].
    Expected: GT_OK.
    1.2. Call with out of range portType value.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL value.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType = CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E;
    GT_U32      value    = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        portType = CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E;

        st = cpssDxChPortPeriodicFlowControlIntervalGet(dev, port,portType, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portType);

        portType = CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E;

        st = cpssDxChPortPeriodicFlowControlIntervalGet(dev,port, portType, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portType);

        /* 1.2 */
        if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPeriodicFlowControlIntervalGet
                            (dev, 0,portType, &value),
                              portType);
        }

        /* 1.3 */
        st = cpssDxChPortPeriodicFlowControlIntervalGet(dev,port ,portType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, portType);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E );

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPeriodicFlowControlIntervalGet(dev,port, portType, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPeriodicFlowControlIntervalGet(dev,port, portType, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesAutoTune)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters portTuningMode
            [CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_E/
             CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E].
    Expected: GT_OK.
    1.1.2. Call with out of range portTuningMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORTS_BMP_STC  portsBmp;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode =
        CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E;
    GT_BOOL supported;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                    UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6059);

        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
        st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_NOT_APPLICABLE_DEVICE == st)
        {
            canSave = GT_FALSE;
        }
        else if(GT_OK != st)
        {
           return;
        }

        /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
            st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
        }
#endif

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceSpeedSupportGet(dev, port,
                                                      CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }


            if(!supported)
            {
                continue;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 serdes */
                                          CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                          CPSS_PORT_SPEED_10000_E);
            if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                if(st != GT_OK)
                {/* in Bcat2 interface mode configuration may not succeed, because
                    of lack of pizza slices */
#ifndef GM_USED
                    goto port_power_down;
#else
                    continue;
#endif
                }
            }

            /* 1.1.1 */

            portTuningMode = CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            portTuningMode = CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E;
            st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            portTuningMode = CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E;
            st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
            /* fail - here is legal result of tuning process,
               in simulation always will fail */
            if((st != GT_OK) && (st != GT_FAIL))
            {
                GT_CHAR     errorMsg[256]; /* error message string */
                cpssOsSprintf(errorMsg,
                              "cpssDxChPortSerdesAutoTune(TX_TRAINING_STATUS) FAILED:%d,%d\n",
                              dev, port);
                (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
            }

            portTuningMode = CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E;
            st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
            /* fail - here is legal result of tuning process,
               in simulation always will fail */
            if((st != GT_OK) && (st != GT_FAIL))
            {
                GT_CHAR     errorMsg[256]; /* error message string */
                cpssOsSprintf(errorMsg,
                              "cpssDxChPortSerdesAutoTune(RX_TRAINING) FAILED:%d,%d\n",
                              dev, port);
                (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
            }

            /* 1.1.2. */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortSerdesAutoTune
                                (dev, port, portTuningMode),
                                 portTuningMode);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
port_power_down:
            /* Deactivate tested port */
            if(GT_TRUE == canSave)
            {
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                              CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                              portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

        }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                      UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesAutoTune(dev, port, portTuningMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesLoopbackModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            laneBmp[0/max laneBmp value]
            mode [CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E/
                  CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E/
                  CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesLoopbackModeGet
    Expected: GT_OK and the same mode.
    1.1.3. Call with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneBmp   = 0;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;
    GT_U32                maxLaneBmp;

    CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT mode = CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E;
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT modeGet = CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E;
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*  find max value of laneNum  */
            maxLaneNum = 0;
            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet:%d,%d", dev, port);
            if (numOfSerdesLanes == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
                maxLaneNum = numOfSerdesLanes - 1;
            }

            /*  max relevant bitmap of lane numbers  */
            maxLaneBmp = 0;
            maxLaneBmp   = (1 << (maxLaneNum+1)) - 1;

            /* 1.1.1 */
            laneBmp = 1;
            mode = CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum = 0;
            st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
#ifndef ASIC_SIMULATION
            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet, "get another mode that expected:%d,%d", dev, port);
#endif
            /* 1.1.1 */
            laneBmp = maxLaneBmp;
            mode = CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E;
            st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
            if(0 == numOfSerdesLanes)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* 1.1.2 */
            laneNum = 0;
            st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &modeGet);
            if(0 == numOfSerdesLanes)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
#ifndef ASIC_SIMULATION
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet, "get another mode that expected:%d,%d", dev, port);
#endif
            }


            /* 1.1.3. */
            laneBmp = 0;
            UTF_ENUMS_CHECK_MAC(cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode),
                                mode);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesLoopbackModeSet(dev, port, laneBmp, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesLoopbackModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            laneNum[0/max Value for current device and port]
            and not NULL modePtr.
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum
    Expected: NOT GT_OK.
    1.1.3. Call with NULL modePtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                 dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT mode = CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E;
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet:%d,%d", dev, port);

            if (numOfSerdesLanes == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
                maxLaneNum = numOfSerdesLanes - 1;
            }

            /* 1.1.1 */
            laneNum   = 0;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum   = maxLaneNum;
            st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3 */
            laneNum   = maxLaneNum + 2; /* +1 not enough, because redundant serdes
                                            in Lion2 allowed */
            st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            laneNum   = 0;

            /* 1.1.4 */
            st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesPolaritySet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            laneBmp[0/0xFFFF/0xFFFFFFFF]
            invertTx [GT_FALSE/GT_TRUE]
            invertRx [GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesPolarityGet
    Expected: GT_OK and the same mode.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneBmp   = 0;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;
    GT_U32                maxLaneBmp;

    GT_BOOL               invertTx    = GT_FALSE;
    GT_BOOL               invertRx    = GT_FALSE;
    GT_BOOL               invertTxGet = GT_FALSE;
    GT_BOOL               invertRxGet = GT_FALSE;
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                if(devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
                    continue;
            }
            /*  find max value of laneNum  */
            maxLaneNum = 0;
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet:%d,%d", dev, port);
            if (numOfSerdesLanes == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
                maxLaneNum = numOfSerdesLanes - 1;
            }

            /*  max relevant bitmap of lane numbers  */
            maxLaneBmp = 0;
            maxLaneBmp   = (1 << (maxLaneNum+1)) - 1;

            /* 1.1.1 */
            laneBmp   = 1;
            invertTx  = GT_FALSE;
            invertRx  = GT_FALSE;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum   = 0;
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTxGet, &invertRxGet);
            if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode) && ((port & 0x3) > 0))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(invertTx, invertTxGet, "get another invertTx that expected:%d,%d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(invertRx, invertRxGet, "get another invertRx that expected:%d,%d", dev, port);

            /* 1.1.1 */
            laneBmp   = maxLaneBmp;
            invertTx  = GT_TRUE;
            invertRx  = GT_TRUE;
            st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum   = 0;
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTxGet, &invertRxGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

#ifndef ASIC_SIMULATION
            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(invertTx, invertTxGet, "get another invertTx that expected:%d,%d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(invertRx, invertRxGet, "get another invertRx that expected:%d,%d", dev, port);
#endif
            /* 1.1.2 */
            laneNum   = maxLaneNum;
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTxGet, &invertRxGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
#ifndef ASIC_SIMULATION
            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(invertTx, invertTxGet, "get another invertTx that expected:%d,%d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(invertRx, invertRxGet, "get another invertRx that expected:%d,%d", dev, port);
#endif
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesPolarityGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            laneNum[0/maxNumber of laneNum]
            and not NULL invertTx, invertRx.
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum
    Expected: NOT GT_OK.
    1.1.3. Call with NULL invertTx
    Expected: GT_BAD_PTR.
    1.1.4. Call with NULL invertRx
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;

    GT_BOOL               invertTx    = GT_FALSE;
    GT_BOOL               invertRx    = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet:%d,%d", dev, port);

            if (numOfSerdesLanes == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
                maxLaneNum = numOfSerdesLanes - 1;
            }

            /* 1.1.1 */
            laneNum = 0;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            laneNum = maxLaneNum;
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                laneNum = maxLaneNum + 2;
            else
                laneNum = maxLaneNum + 1;
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            laneNum = 0;

            /* 1.1.3 */
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, NULL, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /* 1.1.4 */
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesLaneTuningSet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  laneNum;
    GT_U32                                portGroupId         = 0;
    CPSS_DXCH_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
    GT_U32                            serdesFreq = 0;
    CPSS_DXCH_PORT_SERDES_TUNE_STC    tuneValues;
    CPSS_DXCH_PORT_SERDES_TUNE_STC    tuneValuesGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E, CPSS-6062);
        /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        lanesNumInDev = lanesNumInDevGet(dev);
        if(lanesNumInDev == 0)
        {
            return;
        }

         /* Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            for(laneNum = 0; laneNum < lanesNumInDev; laneNum++)
            {
                for(serdesFreq = 0; serdesFreq < CPSS_DXCH_PORT_SERDES_SPEED_NA_E; serdesFreq++)
                {
                    serdesFrequency = (CPSS_DXCH_PORT_SERDES_SPEED_ENT)serdesFreq;
                    cpssOsMemSet(&tuneValues, 0, sizeof(tuneValues));
                    cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
                    if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
                    {
                        tuneValues.BW = 14;
                        tuneValues.DC = 99;
                        tuneValues.HF = 14;
                        tuneValues.LB = 1;
                        tuneValues.LF = 4;
                        tuneValues.sqlch = 99;
                        tuneValues.txAmp = 6;
                        tuneValues.txEmph1 = 10;
                        tuneValues.txEmphAmp = 11;
                        st = cpssDxChPortSerdesLaneTuningSet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                        st = cpssDxChPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValuesGet);
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.BW, tuneValuesGet.BW,
                            "get another  tuneValues.BW value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.DC, tuneValuesGet.DC,
                            "get another  tuneValues.DC value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.HF, tuneValuesGet.HF,
                            "get another  tuneValues.HF value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.LB, tuneValuesGet.LB,
                            "get another  tuneValues.LB value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.LF, tuneValuesGet.LF,
                            "get another  tuneValues.LF value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.sqlch, tuneValuesGet.sqlch,
                            "get another  tuneValues.sqlch value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txAmp, tuneValuesGet.txAmp,
                            "get another  tuneValues.txAmp value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txEmph1, tuneValuesGet.txEmph1,
                            "get another  tuneValues.txEmph1 value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txEmphAmp, tuneValuesGet.txEmphAmp,
                            "get another  tuneValues.txEmphAmp value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_FALSE, tuneValuesGet.txAmpAdj,
                            "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_FALSE, tuneValuesGet.txAmpShft,
                            "get another  tuneValues.txAmpShft value that expected - %d,%d ", dev, portGroupId, laneNum);
                    }
                    else
                    {
                        tuneValues.txEmphAmp = 15;
                        tuneValues.txAmpAdj = 1;
                        tuneValues.ffeS = 3;
                        tuneValues.txEmphEn = GT_TRUE;
                        st = cpssDxChPortSerdesLaneTuningSet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                        st = cpssDxChPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValuesGet);
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txEmphAmp, tuneValuesGet.txEmphAmp,
                            "get another  tuneValues.txEmphAmp value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txAmpAdj, tuneValuesGet.txAmpAdj,
                            "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.ffeS, tuneValuesGet.ffeS,
                            "get another  tuneValues.ffeS value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txEmphEn, tuneValuesGet.txEmphEn,
                            "get another  tuneValues.txEmphEn value that expected - %d,%d ", dev, portGroupId, laneNum);
                    }
                }
            }

            /* serdesFrequency is not valid */
            st = cpssDxChPortSerdesLaneTuningSet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, laneNum);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;

        /* portGroupId is not valid */
        st = cpssDxChPortSerdesLaneTuningSet(dev, portGroupId, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, 0);

        /* pointer  is not valid */
        st = cpssDxChPortSerdesLaneTuningSet(dev, 0, 0, serdesFrequency, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesLaneTuningSet(dev, 0, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesLaneTuningSet(dev, 0, 0, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesLaneTuningGet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  laneNum;
    GT_U32                                portGroupId         = 0;
    CPSS_DXCH_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
    GT_U32                            serdesFreq = 0;
    CPSS_DXCH_PORT_SERDES_TUNE_STC    tuneValues;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E, CPSS-6062);
        /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);
        lanesNumInDev = lanesNumInDevGet(dev);
        if(lanesNumInDev == 0)
        {
            return;
        }
         /* Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            for(laneNum = 0; laneNum < lanesNumInDev; laneNum++)
            {
                for(serdesFreq = 0; serdesFreq < CPSS_DXCH_PORT_SERDES_SPEED_NA_E; serdesFreq++)
                {
                    serdesFrequency = (CPSS_DXCH_PORT_SERDES_SPEED_ENT)serdesFreq;
                    cpssOsMemSet(&tuneValues, 0, sizeof(tuneValues));
                    st = cpssDxChPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                    if((GT_NOT_IMPLEMENTED != st) && (GT_NOT_INITIALIZED != st))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                    }
                    st = cpssDxChPortSerdesLaneTuningSet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                    st = cpssDxChPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                }
            }

            /* serdesFrequency is not valid */
            st = cpssDxChPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, laneNum);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;

        /* portGroupId is not valid */
        st = cpssDxChPortSerdesLaneTuningGet(dev, portGroupId, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, 0);

        /* pointer  is not valid */
        st = cpssDxChPortSerdesLaneTuningGet(dev, 0, 0, serdesFrequency, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesLaneTuningGet(dev, 0, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesLaneTuningGet(dev, 0, 0, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesTuningSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
           laneBmp[0/0xFFFF/0xFFFFFFFF]
           serdesFrequency [CPSS_DXCH_PORT_SERDES_SPEED_1_25_E/
                            CPSS_DXCH_PORT_SERDES_SPEED_4_25_E/
                            CPSS_DXCH_PORT_SERDES_SPEED_12_5_E]
           tuneValuesPtr->dfe[0],
           tuneValuesPtr->ffeR[0],
           tuneValuesPtr->ffeC[0],
           tuneValuesPtr->sampler[0],
           tuneValuesPtr->sqlch[0],
           tuneValuesPtr->txEmphAmp[0/15],
           tuneValuesPtr->txAmp[0/31],
           tuneValuesPtr->txAmpAdj[0/1],
           tuneValuesPtr->ffeS[0/3],
           tuneValuesPtr->txEmphEn[GT_TRUE/GT_FALSE],
           tuneValuesPtr->txEmph1[0],
           tuneValuesPtr->align90[0].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesTuningGet.
    Expected: GT_OK and the same tuneValuesPtr.
    1.1.3. Call with out of range serdesFrequency.
    Expected: GT_BAD_PARAM and the same mode.
    1.1.4. Call with NULL tuneValuesPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneBmp   = 0;
    GT_U32                laneNum   = 0;
    CPSS_PORTS_BMP_STC    portsBmp;

    CPSS_DXCH_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
    CPSS_DXCH_PORT_SERDES_TUNE_STC    tuneValues;
    CPSS_DXCH_PORT_SERDES_TUNE_STC    tuneValuesGet;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  startSerdes;    /* first serdes of port */
    GT_U32  serdesNumPerPort;/* number of serdeses occupied now by port */
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    CPSS_DXCH_DETAILED_PORT_MAP_STC                      portMap;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
       st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);

       if(GT_OK != st)
       {
           return;
       }

       /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
           st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
           if(GT_OK != st)
           {
               isError = GT_TRUE;
               goto restore;
           }
        }
#endif

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        lanesNumInDev = lanesNumInDevGet(dev);
        if(lanesNumInDev == 0)
        {
            return;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,&portMap);
            if (st != GT_OK)
            {
                return;
            }
            if (portMap.valid == GT_FALSE)
            {
                continue;
            }
            if (portMap.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                continue;
            }

            if((CPSS_PP_FAMILY_DXCH_BOBCAT2_E == devFamily) &&
                (CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E == PRV_CPSS_PP_MAC(dev)->devSubFamily))
            {
                if(CPSS_NULL_PORT_NUM_CNS == portMap.portMap.macNum)
                {
                    continue;
                }
            }

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 sedes */
                                          CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                          CPSS_PORT_SPEED_1000_E);
            if(GT_OK != st)
            {
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }


            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port,
                                                   CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                                   &startSerdes, &serdesNumPerPort);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet:%d,%d", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.1 */
            laneBmp   = 1;
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
            cpssOsMemSet(&tuneValues, 0, sizeof(tuneValues));
            tuneValues.txEmphAmp = 15;
            st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum   = 0;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphAmp, tuneValuesGet.txEmphAmp,
                "get another  tuneValues.txEmphAmp value that expected - %d,%d ", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
            tuneValues.txEmphAmp = 0;

            /* 1.1.1 */
            laneBmp   = 0xFFFF;
            tuneValues.txAmpAdj = 1;
            st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 0;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txAmpAdj, tuneValuesGet.txAmpAdj,
                "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 8;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txAmpAdj, tuneValuesGet.txAmpAdj,
                    "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 15;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txAmpAdj, tuneValuesGet.txAmpAdj,
                    "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            tuneValues.txAmpAdj = 0;

            /* 1.1.1 */
            laneBmp   = 0xFFFFFFFF;
            tuneValues.ffeS = 3;
            st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 0;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.ffeS, tuneValuesGet.ffeS,
                "get another  tuneValues.ffeS value that expected - %d,%d ", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 16;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.ffeS, tuneValuesGet.ffeS,
                    "get another  tuneValues.ffeS value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
           }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            tuneValues.ffeS = 0;

            /* 1.1.1 */
            laneBmp   = 0xFFFFFFFF;
            tuneValues.txEmphEn = GT_TRUE;
            st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 0;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphEn, tuneValuesGet.txEmphEn,
                "get another  tuneValues.txEmphEn value that expected - %d,%d ", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 8;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphEn, tuneValuesGet.txEmphEn,
                    "get another  tuneValues.txEmphEn value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 16;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphEn, tuneValuesGet.txEmphEn,
                    "get another  tuneValues.txEmphEn value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

            }

            /* 1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues),
                                serdesFrequency);

            /* 1.1.4. */
            st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, NULL);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_BAD_PTR, st, dev, port);
            if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
            /* Deactivate tested port */
            if(GT_TRUE == canSave)
            {
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev, port);
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

        }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/* UTF_TEST_CASE_MAC(cpssDxChPortSerdesTuningGet) */
/*----------------------------------------------------------------------------*/
#define PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG 0
UTF_TEST_CASE_MAC(cpssDxChPortSerdesTuningGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
           laneNum[0/max value for each dev and port]
           serdesFrequency [CPSS_DXCH_PORT_SERDES_SPEED_1_25_E/
                            CPSS_DXCH_PORT_SERDES_SPEED_4_25_E/
                            CPSS_DXCH_PORT_SERDES_SPEED_12_5_E]
           and not NULL tuneValuesPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum.
    Expected: NOT GT_OK.
    1.1.3. Call with out of range serdesFrequency.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with NULL tuneValuesPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                serdesNumPerPort;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;

    CPSS_DXCH_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
    CPSS_DXCH_PORT_SERDES_TUNE_STC    tuneValues;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    CPSS_DXCH_PORT_MAP_STC   portMapArray[1];
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
        GT_U32 portMacNum;
        GT_U32 portIdx;
    #endif

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support cpssDxChPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        lanesNumInDev = lanesNumInDevGet(dev);
        if(lanesNumInDev == 0)
        {
            return;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
            cpssOsPrintf("\n    Process port:");
            portIdx = 0;
        #endif
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet:%d,%d", dev, port);

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                st = cpssDxChPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
                    portMacNum = portMapArray[0].interfaceNum;
                #endif
            }
            else
            {
                #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
                    portMacNum = port;
                #endif
            }
            #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
                if (0 == (portIdx % 20))
                {
                    cpssOsPrintf("\n");
                }
                portIdx ++;
                cpssOsPrintf("%3d",portMacNum);
            #endif
            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &serdesNumPerPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet:%d,%d", dev, port);

            if (serdesNumPerPort == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
            maxLaneNum = serdesNumPerPort - 1;
            }

            /* 1.1.1 */
            laneNum = 0;
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;

            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_4_25_E;

            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_12_5_E;

            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            laneNum = maxLaneNum;
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;

            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_4_25_E;

            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_12_5_E;

            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            serdesFrequency = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;

            /* 1.1.2 */
            laneNum = lanesNumInDev + 1;
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3. */
            laneNum = 0;
            UTF_ENUMS_CHECK_MAC(cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues),
                                serdesFrequency);

            /* 1.1.4. */
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/* UTF_TEST_CASE_MAC(cpssDxChPortSerdesTxEnableSet)*/
GT_VOID cpssDxChPortSerdesTxEnableSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            enable [GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesTxEnableGet
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORTS_BMP_STC    portsBmp;

    GT_BOOL               enable    = GT_FALSE;
    GT_BOOL               enableGet = GT_FALSE;
    GT_BOOL     supported;
    CPSS_DXCH_DETAILED_PORT_MAP_STC                      portMap;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
        st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
        {
            return;
        }

        /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
            st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
            if(GT_OK != st)
            {
                isError = GT_TRUE;
                goto restore;
            }
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
#endif

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            st = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,&portMap);
            if (st != GT_OK)
            {
                return;
            }
            if (portMap.valid == GT_FALSE)
            {
                continue;
            }
            if (portMap.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                continue;
            }

            st = cpssDxChPortInterfaceSpeedSupportGet(dev, port,
                                                      CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }


            if(!supported)
            {
                continue;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 sedes */
                                          CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                          CPSS_PORT_SPEED_10000_E);
            if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                if(st != GT_OK)
                {/* in Bcat2 interface mode configuration may not succeed, because
                    of lack of pizza slices */
                    continue;
                }
            }

            /* 1.1.1 */
            enable    = GT_FALSE;
            st = cpssDxChPortSerdesTxEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            st = cpssDxChPortSerdesTxEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(enable, enableGet, "get another enable :%d,%d", dev, port);
            if(GT_OK != st)
            {
                 isError = GT_TRUE;
                 goto restore;
            }

            /* 1.1.1 */
            enable    = GT_TRUE;
            st = cpssDxChPortSerdesTxEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            st = cpssDxChPortSerdesTxEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
            {
#ifndef ASIC_SIMULATION
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(enable, enableGet, "get another enable :%d,%d", dev, port);
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
#endif
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(enable, enableGet, "get another enable :%d,%d", dev, port);
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
            if(GT_TRUE == canSave)
            {
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                              CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                                  portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

        }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesTxEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesTxEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesTxEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesTxEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesTxEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesTxEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL               enable    = GT_FALSE;
    CPSS_PORTS_BMP_STC    portsBmp;
    GT_BOOL supported;
    CPSS_DXCH_DETAILED_PORT_MAP_STC                      portMap;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
        st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
        {
            return;
        }

        /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
            st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
            if(GT_OK != st)
            {
                isError = GT_TRUE;
                goto restore;
            }
        }
#endif

    /* 1. Go over all active devices. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            st = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,&portMap);
            if (st != GT_OK)
            {
                return;
            }
            if (portMap.valid == GT_FALSE)
            {
                continue;
            }
            if (portMap.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                continue;
            }

            st = cpssDxChPortInterfaceSpeedSupportGet(dev, port,
                                                      CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(!supported)
            {
                continue;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 sedes */
                                          CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                          CPSS_PORT_SPEED_10000_E);
            if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if(st != GT_OK)
                {/* in Bcat2 interface mode configuration may not succeed, because
                    of lack of pizza slices */
                    continue;
                }
            }

            /* 1.1.1 */
            st = cpssDxChPortSerdesTxEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            st = cpssDxChPortSerdesTxEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
            if(GT_TRUE == canSave)
            {
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                              CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                                  portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

       }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssDxChPortSerdesTxEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSerdesTxEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortSerdesTxEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesTxEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesTxEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*-----------------------------------------------------------------------*/
/*        Pizza Test                                                     */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*        For Test Only                                                  */
/*-----------------------------------------------------------------------*/
typedef struct UTPizzaTestCase_STC
{
    GT_U8                    devNum;
    GT_U32                   portGroupIdx;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_U32                   sliceNum;
    GT_BOOL                  isExtModeEnable;
    GT_U32                   isOk; /* 0 == not equal , 1 == equal  GT_OK*/
    GT_U32                   portState[12];
    int                      sliceState[16];
}UTPizzaTestCase_STC;

const UTPizzaTestCase_STC utSystemTestSeq_12_Slices[] =
{
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
     /*-------------*/
     {  0     ,  0,    0,     3,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* invalide number of slices */
    ,{  0     ,  0,    1,     5,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* invalide number of slices */
    ,{  0     ,  0,    2,     6,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* invalide number of slices */
    ,{  0     ,  0,    3,     7,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* invalide number of slices */
     /*--------------------------------------------------------------*/
     /*------------  Group of Ports 0 1 2 3                          */
     /*--------------------------------------------------------------*/
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    ,{  0     ,  0,    0,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    1,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    2,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    3,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    1,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    2,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    3,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    1,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    3,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    0,     1,  0,    1,  { 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    1,     1,  0,    1,  { 1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    2,     1,  0,    1,  { 1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    3,     1,  0,    1,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 1 */
    /*-------------*/
    ,{  0     ,  0,    1,     2,  0,    0,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 2 1 1  forbidden */
    ,{  0     ,  0,    3,     2,  0,    0,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 2  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     4,  0,    0,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 4 1 1  forbidden */
    ,{  0     ,  0,    2,     4,  0,    0,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 1 4 1  forbidden */
    ,{  0     ,  0,    3,     4,  0,    0,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 4  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     0,  0,    1,  { 1,  0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 1 */
    ,{  0     ,  0,    3,     0,  0,    1,  { 1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0 */
    /*-------------*/
    ,{  0     ,  0,    1,     0,  0,    1,  { 1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    ,{  0     ,  0,    3,     0,  0,    1,  { 1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    /*-------------*/
    ,{  0     ,  0,    0,     2,  0,    1,  { 2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 1 1 */
    ,{  0     ,  0,    2,     2,  0,    1,  { 2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  2, -1, -1,  2, -1, -1, -1, -1, -1, -1  } }  /* 2 0 2 0 */
    /*-------------*/
    ,{  0     ,  0,    2,     0,  0,    1,  { 2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 0 0 */
    ,{  0     ,  0,    0,     4,  0,    1,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 0 0 */
    /*-------------*/
    ,{  0     ,  0,    1,     1,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 1 0 0  forbidden */
    ,{  0     ,  0,    2,     1,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 1 0  forbidden */
    ,{  0     ,  0,    3,     1,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     2,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 2 0 0  forbidden */
    ,{  0     ,  0,    2,     2,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    ,{  0     ,  0,    3,     2,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     4,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 4 0 0  forbidden */
    ,{  0     ,  0,    2,     4,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 4 0  forbidden */
    ,{  0     ,  0,    3,     4,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 0 4  forbidden */
    /*----------------- release port 0                  */
    ,{  0     ,  0,    0,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*----------------- release port 0                  */
    ,{  0     ,  0,    0,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*----------------- port 0 take over slices of other ports  */
    ,{  0     ,  0,    0,     1,  0,    1,  { 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    1,     1,  0,    1,  { 1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    2,     1,  0,    1,  { 1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    3,     1,  0,    1,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    2,     2,  0,    1,  { 1,  1,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  1, -1, -1,  2, -1, -1,  2, -1, -1, -1, -1, -1, -1  } }  /* 1 1 2 0 */
    ,{  0     ,  0,    0,     2,  0,    1,  { 2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  2, -1, -1,  2, -1, -1, -1, -1, -1, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    0,     4,  0,    1,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1,  0, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1  } }  /* 4 0 0 0 */
    /*----------------- release port 0                   */
    ,{  0     ,  0,    0,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*------------------------------------------------------------------*/
    /*----------------  Group of Ports 4 5 6 7                          */
    /*------------------------------------------------------------------*/
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    ,{  0     ,  0,    4,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    5,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    6,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    5,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    6,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    5,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    4,     1,  0,    1,  { 0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    5,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    6,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    7,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 1 1 1 */
    /*-------------*/
    ,{  0     ,  0,    5,     2,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 2 1 1  forbidden */
    ,{  0     ,  0,    7,     2,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 1 1 2  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     4,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 4 1 1  forbidden */
    ,{  0     ,  0,    6,     4,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 1 4 1  forbidden */
    ,{  0     ,  0,    7,     4,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 1 1 4  forbidden */
    /*-------------- release port 5 7 */
    ,{  0     ,  0,    5,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 0 1 1 */
    ,{  0     ,  0,    7,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0 */
    /*-------------*/
    ,{  0     ,  0,    5,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    ,{  0     ,  0,    7,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    /*-------------- assign 2 slices to both ports 4 ,6   */
    ,{  0     ,  0,    4,     2,  0,    1,  { 0,  0,  0,  0,  2,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 1 0 */
    ,{  0     ,  0,    6,     2,  0,    1,  { 0,  0,  0,  0,  2,  0,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  6, -1, -1,  6, -1, -1, -1, -1  } }  /* 2 0 2 0 */
    /*-------------- release port 6 and assign 4 slices to both ports 4  */
    ,{  0     ,  0,    6,     0,  0,    1,  { 0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 0 0 */
    ,{  0     ,  0,    4,     4,  0,    1,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 0 0 */
    /*-------------*/
    ,{  0     ,  0,    5,     1,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 1 0 0  forbidden */
    ,{  0     ,  0,    6,     1,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 1 0  forbidden */
    ,{  0     ,  0,    7,     1,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     2,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 2 0 0  forbidden */
    ,{  0     ,  0,    6,     2,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    ,{  0     ,  0,    7,     2,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     4,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 4 0 0  forbidden */
    ,{  0     ,  0,    6,     4,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 4 0  forbidden */
    ,{  0     ,  0,    7,     4,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 0 4  forbidden */
    /*-------------- release port 4                              */
    ,{  0     ,  0,    4,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*-------------- port 4 take over slices of ot other ports   */
    ,{  0     ,  0,    4,     1,  0,    1,  { 0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    5,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    6,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    7,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  7, -1, -1, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    6,     2,  0,    1,  { 0,  0,  0,  0,  1,  1,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  5, -1, -1,  6, -1, -1,  6, -1, -1, -1, -1  } }  /* 1 1 2 0 */
    ,{  0     ,  0,    4,     2,  0,    1,  { 0,  0,  0,  0,  2,  0,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  6, -1, -1,  6, -1, -1, -1, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    4,     4,  0,    1,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1,  4, -1, -1, -1, -1  } }  /* 4 0 0 0 */
    /*-------------- release port 4                   */
    ,{  0     ,  0,    4,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*-----------------------------------------------------------------*/
    /*------------  Group of Ports 8 9 10 11                           */
    /*-----------------------------------------------------------------*/
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    ,{  0     ,  0,    8,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    9,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   10,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   11,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    9,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   10,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   11,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    9,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   11,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    8,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0 }, { -1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    9,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0 }, { -1,  8, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,   10,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  0 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,   11,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 1 1 1 */
    /*-------------*/
    ,{  0     ,  0,    9,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 2 1 1  forbidden */
    ,{  0     ,  0,   11,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 1 1 2  forbidden */
    /*-------------*/
    ,{  0     ,  0,    9,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 4 1 1  forbidden */
    ,{  0     ,  0,   10,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 1 4 1  forbidden */
    ,{  0     ,  0,   11,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 1 1 4  forbidden */
    /*-------------- release port 9 11  */
    ,{  0     ,  0,    9,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  1,  1 }, { -1,  8, -1, -1, -1, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 0 1 1 */
    ,{  0     ,  0,   11,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  1,  0 }, { -1,  8, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0 */
    /*-------------*/
    ,{  0     ,  0,    9,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  1,  0 }, { -1,  8, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    ,{  0     ,  0,   11,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  1,  0 }, { -1,  8, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    /*-------------- assign 2 slices to both ports 8 ,10  */
    ,{  0     ,  0,    8,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  0,  1,  0 }, { -1,  8, -1, -1,  8, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 1 0 */
    ,{  0     ,  0,   10,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  0,  2,  0 }, { -1,  8, -1, -1,  8, -1, -1, 10, -1, -1, 10, -1, -1, -1, -1, -1  } }  /* 2 0 2 0 */
    /*-------------- release port 10 and assign 4 slices to ports 8 */
    ,{  0     ,  0,   10,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 0 0 */
    ,{  0     ,  0,    8,     4,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 0 0 */
    /*-------------*/
    ,{  0     ,  0,    9,     1,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 1 0 0  forbidden */
    ,{  0     ,  0,   10,     1,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 1 0  forbidden */
    ,{  0     ,  0,   11,     1,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,    9,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 2 0 0  forbidden */
    ,{  0     ,  0,   10,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    ,{  0     ,  0,   11,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    /*-------------*/
    ,{  0     ,  0,    9,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 4 0 0  forbidden */
    ,{  0     ,  0,   10,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 4 0  forbidden */
    ,{  0     ,  0,   11,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 0 4  forbidden */
    /*-------------- release port 8                   */
    ,{  0     ,  0,    8,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*-------------- port 8 take over lices of other ports */
    ,{  0     ,  0,    8,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0 }, { -1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    9,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0 }, { -1,  8, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,   10,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  0 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,   11,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 11, -1, -1, -1, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,   10,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  2,  0 }, { -1,  8, -1, -1,  9, -1, -1, 10, -1, -1, 10, -1, -1, -1, -1, -1  } }  /* 1 1 2 0 */
    ,{  0     ,  0,    8,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  0,  2,  0 }, { -1,  8, -1, -1,  8, -1, -1, 10, -1, -1, 10, -1, -1, -1, -1, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    8,     4,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0 }, { -1,  8, -1, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }  /* 4 0 0 0 */
    /*-------------- release port 8 */
    ,{  0     ,  0,    8,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
};



const UTPizzaTestCase_STC utSystemTestSeq_16_Slices[] =
{
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
     /*-------------*/
     /*--------------------------------------------------------------*/
     /*------------  Group of Ports 0 1 2 3                          */
     /*--------------------------------------------------------------*/
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-------------*/
     {  0     ,  0,    1,     4,  0,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{  0     ,  0,    2,     4,  0,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{  0     ,  0,    3,     4,  0,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*-------------*/
    ,{  0     ,  0,    1,     2,  0,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{  0     ,  0,    3,     2,  0,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*-------------*/
    ,{  0     ,  0,    1,     4,  1,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{  0     ,  0,    2,     4,  1,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{  0     ,  0,    3,     4,  1,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*-------------*/
    ,{  0     ,  0,    1,     2,  1,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{  0     ,  0,    3,     2,  1,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*-------------*/
    ,{  0     ,  0,    1,     0,  0,    1,  { 1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    ,{  0     ,  0,    3,     0,  0,    1,  { 1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 1 0  not error , already done */
    /*---------------  Reg Mode      */
    ,{  0     ,  0,    0,     2,  0,    1,  { 2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 1 0 */
    ,{  0     ,  0,    2,     2,  0,    1,  { 2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  0, -1, -1, -1,  2, -1, -1, -1  } }  /* 2 0 2 0 */
    /*---------------  Ext Mode      */
    ,{  0     ,  0,    0,     2,  1,    1,  { 2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  0, -1, -1, -1,  2, -1, -1, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    2,     2,  1,    1,  { 2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  0, -1, -1, -1,  2, -1, -1, -1  } }  /* 2 0 2 0 */
    /*-------------*/
    ,{  0     ,  0,    2,     0,  0,    1,  { 2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    0,     4,  0,    1,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 0 0 */
    /*---------------  Reg Mode      */
    ,{  0     ,  0,    1,     1,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 1 0 0  forbidden */
    ,{  0     ,  0,    2,     1,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 1 0  forbidden */
    ,{  0     ,  0,    3,     1,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     2,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 2 0 0  forbidden */
    ,{  0     ,  0,    2,     2,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    ,{  0     ,  0,    3,     2,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     4,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 4 0 0  forbidden */
    ,{  0     ,  0,    2,     4,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 4 0  forbidden */
    ,{  0     ,  0,    3,     4,  0,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 0 4  forbidden */
    /*---------------  Ext Mode      */
    ,{  0     ,  0,    1,     1,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 1 0 0  forbidden */
    ,{  0     ,  0,    2,     1,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 1 0  forbidden */
    ,{  0     ,  0,    3,     1,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     2,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 2 0 0  forbidden */
    ,{  0     ,  0,    2,     2,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    ,{  0     ,  0,    3,     2,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 2 0  forbidden */
    /*-------------*/
    ,{  0     ,  0,    1,     4,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 4 0 0  forbidden */
    ,{  0     ,  0,    2,     4,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 4 0  forbidden */
    ,{  0     ,  0,    3,     4,  1,    0,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 0 4  forbidden */
    /*----------------- release port 0      */
    ,{  0     ,  0,    0,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*----------------- release port 0      */
    ,{  0     ,  0,    0,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*----------------- port 0 take over slices of other ports reg mode */
    ,{  0     ,  0,    0,     1,  0,    1,  { 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    1,     1,  0,    1,  { 1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    2,     1,  0,    1,  { 1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    3,     1,  0,    1,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  1, -1, -1, -1,  3, -1, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    2,     2,  0,    1,  { 1,  1,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  1, -1, -1, -1,  2, -1, -1, -1  } }  /* 1 1 2 0 */
    ,{  0     ,  0,    0,     2,  0,    1,  { 2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  0, -1, -1, -1,  2, -1, -1, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    0,     4,  0,    1,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 0 0 */
    /*----------------- release port 0 */
    ,{  0     ,  0,    0,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*----------------- port 0 take over slices of other ports ext mode */
    ,{  0     ,  0,    0,     1,  1,    1,  { 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    1,     1,  1,    1,  { 1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    2,     1,  1,    1,  { 1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    3,     1,  1,    1,  { 1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  1, -1, -1, -1,  3, -1, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    2,     2,  1,    1,  { 1,  1,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  1, -1, -1, -1,  2, -1, -1, -1  } }  /* 1 1 2 0 */
    ,{  0     ,  0,    0,     2,  1,    1,  { 2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  2, -1, -1, -1,  0, -1, -1, -1,  2, -1, -1, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    0,     4,  1,    1,  { 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1,  0, -1, -1, -1  } }  /* 4 0 0 0 */
    /*----------------- release port 0                   */
    ,{  0     ,  0,    0,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*------------------------------------------------------------------*/
    /*----------------  Group of Ports 4 5 6 7                          */
    /*------------------------------------------------------------------*/
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    ,{  0     ,  0,    4,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    5,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    6,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    5,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    6,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    5,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,    4,    10,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    5,    10,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    6,    10,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,    10,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    5,     4,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    6,     4,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,     4,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    5,     2,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    7,     2,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*------------ Reg Mode */
    ,{  0     ,  0,    4,     1,  0,    1,  { 0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    5,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    6,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    7,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 1 */
    /*------------ Ext Mode */
    ,{  0     ,  0,    4,     1,  1,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    5,     1,  1,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    6,     1,  1,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    7,     1,  1,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 1 */
    /*-------------*/
    ,{  0     ,  0,    5,     2,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 2 1 1  forbidden */
    ,{  0     ,  0,    7,     2,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 2  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     4,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 4 1 1  forbidden */
    ,{  0     ,  0,    6,     4,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 4 1  forbidden */
    ,{  0     ,  0,    7,     4,  0,    0,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 4  forbidden */
    /*-------------- release port 5 7 */
    ,{  0     ,  0,    5,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1, -1, -1, -1, -1,  6, -1  } }  /* 1 0 1 1 */
    ,{  0     ,  0,    7,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, -1  } }  /* 1 0 1 0 */
    /*-------------*/
    ,{  0     ,  0,    5,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, -1  } }  /* 1 0 1 0  not error , already done */
    ,{  0     ,  0,    7,     0,  0,    1,  { 0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, -1  } }  /* 1 0 1 0  not error , already done */
    /*-------------- assign 2 slices to both ports 4 ,6 */
    ,{  0     ,  0,    4,     2,  0,    1,  { 0,  0,  0,  0,  2,  0,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1, -1,  6, -1  } }  /* 2 0 1 0 */
    ,{  0     ,  0,    6,     2,  0,    1,  { 0,  0,  0,  0,  2,  0,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  6, -1, -1, -1,  4, -1, -1, -1,  6, -1  } }  /* 2 0 2 0 */
    /*-------------- release port 6 and assign 4 slices to both ports 4 */
    ,{  0     ,  0,    6,     0,  0,    1,  { 0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1  } }  /* 2 0 0 0 */
    ,{  0     ,  0,    4,     4,  0,    1,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 0 0 */
    /*------------ Reg Mode */
    ,{  0     ,  0,    5,     1,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 1 0 0  forbidden */
    ,{  0     ,  0,    6,     1,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 1 0  forbidden */
    ,{  0     ,  0,    7,     1,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     2,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 2 0 0  forbidden */
    ,{  0     ,  0,    6,     2,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 2 0  forbidden */
    ,{  0     ,  0,    7,     2,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 2 0  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     4,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 4 0 0  forbidden */
    ,{  0     ,  0,    6,     4,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 4 0  forbidden */
    ,{  0     ,  0,    7,     4,  0,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 0 4  forbidden */
    /*------------ Ext Mode */
    ,{  0     ,  0,    5,     1,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 1 0 0  forbidden */
    ,{  0     ,  0,    6,     1,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 1 0  forbidden */
    ,{  0     ,  0,    7,     1,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     2,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 2 0 0  forbidden */
    ,{  0     ,  0,    6,     2,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 2 0  forbidden */
    ,{  0     ,  0,    7,     2,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 2 0  forbidden */
    /*-------------*/
    ,{  0     ,  0,    5,     4,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 4 0 0  forbidden */
    ,{  0     ,  0,    6,     4,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 4 0  forbidden */
    ,{  0     ,  0,    7,     4,  1,    0,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 0 4  forbidden */
    /*-------------- release port 4 in Ext Mode                 */
    ,{  0     ,  0,    4,     0,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*-------------- port 4 take over slices of ot other ports Reg mode */
    ,{  0     ,  0,    4,     1,  0,    1,  { 0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    5,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    6,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    7,     1,  0,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    6,     2,  0,    1,  { 0,  0,  0,  0,  1,  1,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  6, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 2 0 */
    ,{  0     ,  0,    4,     2,  0,    1,  { 0,  0,  0,  0,  2,  0,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  6, -1, -1, -1,  4, -1, -1, -1,  6, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    4,     4,  0,    1,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 0 0 */
    /*-------------- release port 4                   */
    ,{  0     ,  0,    4,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*-------------- port 4 take over slices of ot other ports Ext Mode */
    ,{  0     ,  0,    4,     1,  1,    1,  { 0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    5,     1,  1,    1,  { 0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,    6,     1,  1,    1,  { 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,    7,     1,  1,    1,  { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  7, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    6,     2,  1,    1,  { 0,  0,  0,  0,  1,  1,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  6, -1, -1, -1,  5, -1, -1, -1,  6, -1  } }  /* 1 1 2 0 */
    ,{  0     ,  0,    4,     2,  1,    1,  { 0,  0,  0,  0,  2,  0,  2,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  6, -1, -1, -1,  4, -1, -1, -1,  6, -1  } }  /* 2 0 2 0 */
    ,{  0     ,  0,    4,     4,  1,    1,  { 0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1, -1, -1,  4, -1  } }  /* 4 0 0 0 */
    /*-------------- release port 4                   */
    ,{  0     ,  0,    4,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
    /*-------------------------------------------------------------------*/
    /*------------  Group of Ports 8 9 10 11                             */
    /*-------------------------------------------------------------------*/
     /*                                                         port state                                          slice state                                 */
     /* devIdx, PG, port,sliceN, EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    ,{  0     ,  0,    8,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,    9,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   10,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   11,    10,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*-------------*/
    ,{  0     ,  0,   10,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   11,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   10,     4,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{  0     ,  0,   11,     4,  1,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    /*------------ Reg Mode */
    ,{  0     ,  0,    8,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0 }, { -1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 0 0 0 */
    ,{  0     ,  0,    9,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0 }, { -1,  8, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,   10,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  0 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,   11,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 1 1 */
    /*------------ Reg Mode Test 4 slices on port 8 and return to */
    ,{  0     ,  0,    8,     4,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  1,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1, -1, -1,  8, -1, -1, -1,  8, -1, -1  } }  /* 4 1 0 0 */
    ,{  0     ,  0,    8,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0 }, { -1,  8, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 0 0 */
    ,{  0     ,  0,   10,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  0 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 0 */
    ,{  0     ,  0,   11,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 1 1 */
    /*-------------*/
    ,{  0     ,  0,   10,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 4 1  forbidden */
    ,{  0     ,  0,   11,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 1 4  forbidden */
    /*----------- Ext Mode */
    ,{  0     ,  0,    8,     1,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,    9,     1,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,   10,     1,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,   11,     1,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, 11, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 1 1 1 1 */
    ,{  0     ,  0,   11,     1,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1  } }  /* 1 1 1 1 */
    /*-------------- assign 2 slices to both ports 8 9,10,11 Reg Mode */
    ,{  0     ,  0,    8,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  1,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1,  8, -1, -1, -1, 11, -1, -1  } }  /* 2 1 1 1 */
    ,{  0     ,  0,    9,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  1,  1 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1,  8, -1,  9, -1, 11, -1, -1  } }  /* 2 2 1 1 */
    ,{  0     ,  0,   10,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  0 }, { -1,  8, -1,  9, -1, 10, -1, -1, -1,  8, -1,  9, -1, 10, -1, -1  } }  /* 2 2 2 0 */
    ,{  0     ,  0,   11,     2,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2 }, { -1,  8, -1,  9, -1, 10, -1, 11, -1,  8, -1,  9, -1, 10, -1, 11  } }  /* 2 2 2 2 */
    /*-------------- assign 2 slices to both ports 8 9,10,11 Ext Mode */
    ,{  0     ,  0,    8,     2,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2 }, { -1,  8, -1,  9, -1, 10, -1, 11, -1,  8, -1,  9, -1, 10, -1, 11  } }  /* 2 2 2 2 */
    ,{  0     ,  0,    9,     2,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2 }, { -1,  8, -1,  9, -1, 10, -1, 11, -1,  8, -1,  9, -1, 10, -1, 11  } }  /* 2 2 2 2 */
    ,{  0     ,  0,   10,     2,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2 }, { -1,  8, -1,  9, -1, 10, -1, 11, -1,  8, -1,  9, -1, 10, -1, 11  } }  /* 2 2 2 2 */
    ,{  0     ,  0,   11,     2,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2 }, { -1,  8, -1,  9, -1, 10, -1, 11, -1,  8, -1,  9, -1, 10, -1, 11  } }  /* 2 2 2 2 */
    /*-------------- assign 4 slices to both ports 8 9 Reg Mode */
    ,{  0     ,  0,    8,     4,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  2,  0,  2 }, { -1,  8, -1,  9, -1,  8, -1, 11, -1,  8, -1,  9, -1,  8, -1, 11  } }  /* 4 2 0 2 */
    ,{  0     ,  0,    9,     4,  1,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }  /* 4 4 0 0 */
    /*-------------*/
    ,{  0     ,  0,   10,     1,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }  /* 4 4 1 0  forbidden */
    ,{  0     ,  0,   11,     1,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }  /* 4 4 0 1  forbidden */
    /*-------------*/
    ,{  0     ,  0,   10,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }  /* 4 4 2 0  forbidden */
    ,{  0     ,  0,   11,     2,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }  /* 4 4 0 2  forbidden */
    /*-------------*/
    ,{  0     ,  0,   10,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }  /* 4 4 4 0  forbidden */
    ,{  0     ,  0,   11,     4,  0,    0,  { 0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0,  0 }, { -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }  /* 4 4 0 4  forbidden */
    /*-------------- release port 8 ,9 */
    ,{  0     ,  0,    8,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0 }, { -1, -1, -1,  9, -1, -1, -1,  9, -1, -1, -1,  9, -1, -1, -1,  9  } }  /* 0 0 0 0 */
    ,{  0     ,  0,    9,     0,  0,    1,  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }  /* 0 0 0 0 */
};

static void utPizzaSimTestSystemTestPrintHdr(void)
{
    PRV_UTF_LOG0_MAC("\n------------------------------------------------------------------------------------------------------------------------");
    PRV_UTF_LOG0_MAC("\n                                         port State                   :            slice State                          ");
    PRV_UTF_LOG0_MAC("\ntest : dev pg port ext slN : rc :  0  1  2  3  4  5  6  7  8  9 10 11 :  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 ");
    PRV_UTF_LOG0_MAC("\n------------------------------------------------------------------------------------------------------------------------");
}

static void utPizzaSimTestSystemPrintCase(
    IN int testN,
    IN UTPizzaTestCase_STC * p_testCase
)
{
    PRV_UTF_LOG1_MAC("\n%4d :",testN);
    PRV_UTF_LOG5_MAC(" %3d %2d %4d %3d %3d :",(int)p_testCase->devNum,
                                         (int)p_testCase->portGroupIdx,
                                         (int)p_testCase->portNum,
                                         (int)p_testCase->isExtModeEnable,
                                         (int)p_testCase->sliceNum);
    PRV_UTF_LOG1_MAC(" %2d :",(int)p_testCase->isOk);
}

static void utPizzaSimTestSystemPrintPortsState(
    IN UTPizzaTestCase_STC * p_testCase,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_PHYSICAL_PORT_NUM portIdx;

    for (portIdx  = 0 ; portIdx < portNum ; portIdx ++)
    {
        if (p_testCase->portState[portIdx] > 0)
        {
            PRV_UTF_LOG1_MAC(" %2d",(int)p_testCase->portState[portIdx]);
        }
        else
        {
            PRV_UTF_LOG0_MAC("  -");
        }
    }
    PRV_UTF_LOG0_MAC(" :");
}

static void utPizzaSimTestSystemPrintSlicesState(
    IN UTPizzaTestCase_STC * p_testCase,
    IN GT_U32 sliceNum
)
{
    GT_U32 sliceIdx;
    for (sliceIdx = 0 ; sliceIdx < sliceNum ; sliceIdx++)
    {
        if(p_testCase->sliceState[sliceIdx] >= 0)
        {
            PRV_UTF_LOG1_MAC(" %2d",p_testCase->sliceState[sliceIdx]);
        }
        else
        {
            PRV_UTF_LOG0_MAC(" --");
        }
    }
}

static GT_STATUS utPizzaSimTestSystemTestPortsState(
    IN UTPizzaTestCase_STC * p_testCase
)
{
    GT_STATUS             rc;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                sliceNum;

    for (portNum = 0 ; portNum < 12 ; portNum++)
    {
        sliceNum = CPSS_PA_INVALID_SLICE;
        rc = cpssDxChPortPizzaArbiterIfPortStateGet(p_testCase->devNum, p_testCase->portGroupIdx, portNum, /*OUT*/&sliceNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (p_testCase->portState[portNum] != sliceNum)
        {
            utPizzaSimTestSystemPrintPortsState(p_testCase, portNum+1);
            return GT_FAIL;
        }
    }
    return GT_OK;
}

static GT_STATUS utPizzaSimTestSystemTestSlicesState(
    IN UTPizzaTestCase_STC * p_testCase
)
{
    GT_STATUS             rc;
    GT_U32                slice;
    GT_BOOL               isOccupied;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                mul; /* TEMPORARY shall be changed in future*/

    mul = 1;

    for (slice = 0 ; slice < sizeof(p_testCase->sliceState)/sizeof(p_testCase->sliceState[0]); slice++)
    {
        rc = cpssDxChPortPizzaArbiterIfSliceStateGet(/*IN*/p_testCase->devNum,
                                             /*IN*/p_testCase->portGroupIdx,
                                             /*IN*/mul*slice,
                                             /*OUT*/&isOccupied,
                                             /*OUT*/&portNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (p_testCase->sliceState[slice] == -1  && isOccupied == 1)
        {
            utPizzaSimTestSystemPrintSlicesState(p_testCase, slice+1);
            return GT_FAIL;
        }
        if (p_testCase->sliceState[slice] != -1  && isOccupied == 0)
        {
            utPizzaSimTestSystemPrintSlicesState(p_testCase, slice+1);
            return GT_FAIL;
        }
        if (isOccupied == 1)
        {
            if ((GT_PHYSICAL_PORT_NUM)p_testCase->sliceState[slice] != portNum)
            {
                utPizzaSimTestSystemPrintSlicesState(p_testCase, slice+1);
                return GT_FAIL;
            }
        }
    }
    return GT_OK;
}


static GT_STATUS utPizzaSimTestSystemSelectTestSuite
(
    IN GT_U8 devNum,
    GT_U32               * totalTestsPtr,
    UTPizzaTestCase_STC ** testSuitePtrPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr = NULL;

    if (totalTestsPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if (testSuitePtrPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);

    rc = cpssDxChPortPizzaArbiterIfUserTableGet(0,10,&profilePtr, NULL);
    if (GT_OK != rc)
    {
        return rc;
    }

    switch (profilePtr->maxSliceNum)
    {
         case 12:
            *testSuitePtrPtr = (UTPizzaTestCase_STC *)&utSystemTestSeq_12_Slices[0];
            *totalTestsPtr   = sizeof(utSystemTestSeq_12_Slices)/sizeof(utSystemTestSeq_12_Slices[0]);
         break;
         case 16:
            *testSuitePtrPtr = (UTPizzaTestCase_STC *)&utSystemTestSeq_16_Slices[0];
            *totalTestsPtr   = sizeof(utSystemTestSeq_16_Slices)/sizeof(utSystemTestSeq_16_Slices[0]);
         break;
         default:
         {
             *testSuitePtrPtr = NULL;
             *totalTestsPtr   = 0;
             return GT_NOT_SUPPORTED;
         }
    }
    return GT_OK;
}

static GT_STATUS utPizzaSimTestSystem(IN GT_U8 devNum)
{
    GT_STATUS rc ;

    GT_U32         testCaseIdx;
    UTPizzaTestCase_STC * p_testCase;
    GT_U32         totalTests;
    GT_U32         testN;
    GT_PHYSICAL_PORT_NUM globalPortNum;
    CPSS_PORTS_BMP_STC initPortsBmp;/* bitmap of ports to init */
    CPSS_PORT_SPEED_ENT arrSpeed[] =
    {
        /* 0 */  CPSS_PORT_SPEED_NA_E
        /* 1 */ ,CPSS_PORT_SPEED_10000_E
        /* 2 */ ,CPSS_PORT_SPEED_20000_E
        /* 3 */ ,CPSS_PORT_SPEED_NA_E
        /* 4 */ ,CPSS_PORT_SPEED_40000_E
        /* 5 */ ,CPSS_PORT_SPEED_NA_E
        /* 6 */ ,CPSS_PORT_SPEED_NA_E
        /* 7 */ ,CPSS_PORT_SPEED_NA_E
        /* 8 */ ,CPSS_PORT_SPEED_NA_E
        /* 9 */ ,CPSS_PORT_SPEED_NA_E
        /*10 */ ,CPSS_PORT_SPEED_NA_E
    };


    rc = utPizzaSimTestSystemSelectTestSuite(devNum, /*OUT*/&totalTests,&p_testCase);
    if (rc != GT_OK)
    {
        return rc;
    }


    testN = 175;

    utPizzaSimTestSystemTestPrintHdr();
    for (testCaseIdx = 0 ; testCaseIdx < totalTests; testCaseIdx++, p_testCase++)
    {
        utPizzaSimTestSystemPrintCase(testCaseIdx, p_testCase);


        if (testCaseIdx == testN)
        {
            testCaseIdx = testCaseIdx;
        }

        globalPortNum = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(p_testCase->devNum, p_testCase->portGroupIdx, p_testCase->portNum);

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,globalPortNum);

        if( (p_testCase->portNum == 9) || (p_testCase->portNum == 11))
        {
            rc = cpssDxChPortExtendedModeEnableSet(p_testCase->devNum, globalPortNum, p_testCase->isExtModeEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        rc = cpssDxChPortModeSpeedSet(p_testCase->devNum,
                                      &initPortsBmp,
                                      GT_TRUE,
                                      CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                      arrSpeed[p_testCase->sliceNum]);

        if (1 == p_testCase->isOk)
        {
            if (rc != GT_OK)
            {
                return GT_FAIL;
            }
        }
        else
        {
            if (rc == GT_OK)
            {
                return GT_FAIL;
            }
        }
        rc = utPizzaSimTestSystemTestPortsState(p_testCase);
        if (rc != GT_OK)
        {
            return rc;
        }
        utPizzaSimTestSystemPrintPortsState(p_testCase, 12);

        /* Test Slice */
        rc = utPizzaSimTestSystemTestSlicesState(p_testCase);
        if (rc != GT_OK)
        {
            return rc;
        }
        utPizzaSimTestSystemPrintSlicesState(p_testCase, 16);
    }

    PRV_UTF_LOG0_MAC("\nHello. Pizza !\n");
    return GT_OK;
}



UTF_TEST_CASE_MAC(cpssDxChPortPizzaArbiterIfPortStateGet)
{
    GT_U8       dev;

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_UTF_LOG0_MAC("\nPrivet UT");

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        utPizzaSimTestSystem(dev);
    }
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortFlowControlPacketsCntGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChPortFlowControlPacketsCntGet
                with not NULL receivedCntPtr, droppedCntPtr.
    Expected: GT_OK.
    1.2. Call with NULL receivedCntPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL droppedCntPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      receivedCnt = 0;
    GT_U32      droppedCnt = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChPortFlowControlPacketsCntGet(dev, &receivedCnt, &droppedCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortFlowControlPacketsCntGet(dev, NULL, &droppedCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChPortFlowControlPacketsCntGet(dev, &receivedCnt, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFlowControlPacketsCntGet(dev, &receivedCnt, &droppedCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortFlowControlPacketsCntGet(dev, &receivedCnt, &droppedCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortUnknownMacControlFramesCmdSet)
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
    1.2. Call cpssDxChPortUnknownMacControlFramesCmdGet.
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

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_ROUTE_E;

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortUnknownMacControlFramesCmdSet(dev, command),
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
        st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortUnknownMacControlFramesCmdSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortUnknownMacControlFramesCmdGet)
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
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortUnknownMacControlFramesCmdGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortMruProfileSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS
    1.1.1. Call with cpssDxChPortMruProfileSet profileId [0/3/7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortMruProfileGet.
    Expected: GT_OK and the same profileId.
    1.1.3. Call with out of range profileId [8].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_U32           profileId = 0;
    GT_U32           profileIdGet = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*  1.1.1.  */
            profileId = 0;

            st = cpssDxChPortMruProfileSet(dev, port, profileId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChPortMruProfileGet(dev, port, &profileIdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortMruProfileGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(profileId, profileIdGet,
                       "get another profileId than was set: %d, %d", dev, port);

            /*  1.1.1.  */
            profileId = 3;

            st = cpssDxChPortMruProfileSet(dev, port, profileId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChPortMruProfileGet(dev, port, &profileIdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortMruProfileGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(profileId, profileIdGet,
                       "get another profileId than was set: %d, %d", dev, port);

            /*  1.1.1.  */
            profileId = 7;

            st = cpssDxChPortMruProfileSet(dev, port, profileId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChPortMruProfileGet(dev, port, &profileIdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortMruProfileGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(profileId, profileIdGet,
                       "get another profileId than was set: %d, %d", dev, port);

            /*  1.1.3.  */
            profileId = 8;

            st = cpssDxChPortMruProfileSet(dev, port, profileId);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            profileId = 0;
        }

        profileId = 0;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMruProfileSet(dev, port, profileId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChPortMruProfileSet(dev, port, profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    profileId = 0;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMruProfileSet(dev, port, profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMruProfileSet(dev, port, profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortMruProfileGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS
    1.1.1. Call with cpssDxChPortMruProfileGet with not NULL profileIdPtr
    Expected: GT_OK.
    1.1.2. Call with NULL profileIdPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_U32      profileId = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*  1.1.1.  */
            st = cpssDxChPortMruProfileGet(dev, port, &profileId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChPortMruProfileGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMruProfileGet(dev, port, &profileId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChPortMruProfileGet(dev, port, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMruProfileGet(dev, port, &profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMruProfileGet(dev, port, &profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortProfileMruSizeSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChPortProfileMruSizeSet with relevant values
         profile[0/3/7],
         mruSize[0/0x1FFF/0x3FFF].
    Expected: GT_OK.
    1.2. Call cpssDxChPortProfileMruSizeGet.
    Expected: GT_OK and the same mruSize.
    1.3. Call with out of range profile[8].
    Expected: NOT GT_OK.
    1.4. Call with out of range mruSize[0x4000].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32   profile = 0;
    GT_U32   mruSize = 0;
    GT_U32   mruSizeGet = 0;

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
        mruSize = 0x3FFF;

        st = cpssDxChPortProfileMruSizeSet(dev, profile, mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mruSize, mruSizeGet,
                   "get another mruSize than was set: %d", dev);

        /*  1.1.  */
        profile = 3;
        mruSize = 0;

        st = cpssDxChPortProfileMruSizeSet(dev, profile, mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mruSize, mruSizeGet,
                   "get another mruSize than was set: %d", dev);

        /*  1.1.  */
        profile = 7;
        mruSize = 0x1FFF;

        st = cpssDxChPortProfileMruSizeSet(dev, profile, mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mruSize, mruSizeGet,
                   "get another mruSize than was set: %d", dev);

        /*  1.3.  */
        profile = 8;

        st = cpssDxChPortProfileMruSizeSet(dev, profile, mruSize);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        profile = 0;

        /*  1.4.  */
        mruSize = 0x4000;

        st = cpssDxChPortProfileMruSizeSet(dev, profile, mruSize);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        mruSize = 0;
    }
    profile = 0;
    mruSize = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortProfileMruSizeSet(dev, profile, mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortProfileMruSizeSet(dev, profile, mruSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortProfileMruSizeGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChPortProfileMruSizeGet with relevant values
         profile[0/3/7],
         and not NULL mruSizePtr.
    Expected: GT_OK.
    1.2. Call with out of range profile[8].
    Expected: NOT GT_OK.
    1.3. Call with NULL mruSizePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32   profile = 0;
    GT_U32   mruSize = 0;

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

        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        profile = 3;

        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        profile = 7;

        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        profile = 8;

        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSize);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        profile = 0;

        /*  1.3.  */
        st = cpssDxChPortProfileMruSizeGet(dev, profile, NULL);
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
        st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortProfileMruSizeGet(dev, profile, &mruSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortMruExceptionCommandSet)
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
    1.2. Call cpssDxChPortMruExceptionCommandGet.
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

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCommandGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCommandGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCommandGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCommandGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCommandGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_ROUTE_E;

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortMruExceptionCommandSet(dev, command),
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
        st = cpssDxChPortMruExceptionCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMruExceptionCommandSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortMruExceptionCommandGet)
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
        st = cpssDxChPortMruExceptionCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCommandGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMruExceptionCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMruExceptionCommandGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortMruExceptionCpuCodeSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with cpuCode[CPSS_NET_ETH_BRIDGED_LLT_E /
                           CPSS_NET_CONTROL_E /
                           CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E /
                           CPSS_NET_UDP_BC_MIRROR_TRAP2_E /
                           CPSS_NET_IPV6_ROUTE_TRAP_E],
    Expected: GT_OK.
    1.2. Call cpssDxChPortMruExceptionCpuCodeGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong cpuCode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_CONTROL_E;
    CPSS_NET_RX_CPU_CODE_ENT cpuCodeGet = CPSS_NET_CONTROL_E;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        cpuCode = CPSS_NET_ETH_BRIDGED_LLT_E;

        st = cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_CONTROL_E;

        st = cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E;

        st = cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP2_E;

        st = cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_IPV6_ROUTE_TRAP_E;

        st = cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode),
            cpuCode);
    }

    cpuCode = CPSS_NET_CONTROL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMruExceptionCpuCodeSet(dev, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortMruExceptionCpuCodeGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with not NULL cpuCodePtr.
    Expected: GT_OK.
    1.2. Call with NULL cpuCodePt.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_CONTROL_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMruExceptionCpuCodeGet(dev, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEomBaudRateGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEomBaudRateGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2)
    1.1.1. Call with serdesNum[0 / maxSerdesNum/2 / maxSerdesNum] and
        valid baudRatePtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range serdesNum[maxSerdesNum + 1] and other
        valid parameters same as 1.1.1
    Expected: GT_BAD_PARAM.
    1.1.3. Call with invalid baudRatePtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    CPSS_PORT_INTERFACE_MODE_ENT ifMode  = CPSS_PORT_INTERFACE_MODE_SR_LR_E;

    GT_STATUS                   st               = GT_OK;
    GT_U8                       devNum           = 0;
    GT_PHYSICAL_PORT_NUM        portNum          = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_U32                      serdesNum        = 0;
    GT_U32                      maxSerdesNum     = 0;
    GT_U32                      startSerdes      = 0;
    GT_U32                      numOfSerdesLanes = 0;
    GT_U32                      baudRate         = 0;




    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E  |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E  |
                                      UTF_LION_E | UTF_XCAT2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            GT_BOOL                            valid;

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

                valid = GT_FALSE;
                st = cpssDxChPortPhysicalPortDetailedMapGet(devNum,portNum,/*OUT*/portMapShadowPtr);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (GT_TRUE == portMapShadowPtr->valid)
                {
                    st = prvUtfPortInterfaceModeGet(devNum,portNum,&ifMode);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", devNum, portNum);
                    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    {
                        valid = GT_FALSE;
                    }
                    else /* (numOfSerdesLanes > 0) */
                    {
                        valid = GT_TRUE;
                    }
                }
            }
            else
            {
                valid = GT_TRUE;
                st = prvUtfPortInterfaceModeGet(devNum,portNum,&ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", devNum, portNum);
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    valid = GT_FALSE;
                }
                else /* (numOfSerdesLanes > 0) */
                {
                    valid = GT_TRUE;
                }
            }

            if (valid == GT_TRUE)
            {
                st = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                                        &startSerdes, &numOfSerdesLanes);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvCpssDxChPortIfModeSerdesNumGet() : : dev = %d port = %d",devNum, portNum);

                /* Get maxSerdesNum for specific devNum and portNum*/
                maxSerdesNum = numOfSerdesLanes - 1;
                /*
                    1.1.1. Call with serdesNum [0] and valid baudRatePtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = 0;
                st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                        1.1.1. Call with serdesNum [maxSerdesNum/2] and
                                valid baudRatePtr [non-NULL].
                        Expected: GT_OK.
                */
                serdesNum = maxSerdesNum/2;
                st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                        1.1.1. Call with serdesNum [maxSerdesNum] and
                                valid baudRatePtr [non-NULL].
                        Expected: GT_OK.
                */
                serdesNum = maxSerdesNum;
                st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                        1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and
                                other valid parameters same as 1.1.1
                        Expected: GT_BAD_PARAM.
                */
                serdesNum = maxSerdesNum + 1;
                st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                                                    serdesNum);

                /* restore valid values */
                serdesNum = 0;

                /*
                    1.1.3. Call with invalid baudRatePtr [NULL] and other
                            valid parameters same as 1.1.1.
                    Expected: GT_BAD_PTR.
                */

                st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum, serdesNum);
            }
            else
            {
                serdesNum = 0;
                st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, serdesNum);
            }
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum, &baudRate);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    serdesNum = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                      UTF_LION_E | UTF_XCAT2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum, &baudRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum, &baudRate);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEomDfeResGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEomDfeResGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2)
    1.1.1. Call with serdesNum [0 / maxSerdesNum/2 / maxSerdesNum] and
        valid dfeResPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with invalid dfeResPtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;

    GT_STATUS                st                 = GT_OK;
    GT_U8                    devNum             = 0;
    GT_PHYSICAL_PORT_NUM     portNum            = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                   serdesNum          = 0;
    GT_U32                   maxSerdesNum       = 0;
    GT_U32                   startSerdes        = 0;
    GT_U32                   numOfSerdesLanes   = 0;
    GT_U32                   dfeRes[CPSS_DXCH_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS] = {0};

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                      UTF_LION_E | UTF_XCAT2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            GT_BOOL                            valid;
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
                CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

                valid = GT_FALSE;
                st = cpssDxChPortPhysicalPortDetailedMapGet(devNum,portNum,/*OUT*/portMapShadowPtr);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (GT_TRUE == portMapShadowPtr->valid)
                {
                    st = prvUtfPortInterfaceModeGet(devNum,portNum,&ifMode);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", devNum, portNum);
                    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    {
                        valid = GT_FALSE;
                    }
                    else /* (numOfSerdesLanes > 0) */
                    {
                        valid = GT_TRUE;
                    }
                }
            }
            else
            {
                valid = GT_TRUE;
                st = prvUtfPortInterfaceModeGet(devNum,portNum,&ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", devNum, portNum);
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    valid = GT_FALSE;
                }
                else /* (numOfSerdesLanes > 0) */
                {
                    valid = GT_TRUE;
                }
            }

            if (valid == GT_TRUE)
            {
                st = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                                &startSerdes, &numOfSerdesLanes);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvCpssDxChPortIfModeSerdesNumGet() : : dev = %d port = %d",devNum, portNum);
                maxSerdesNum = numOfSerdesLanes - 1;

                /*
                    1.1.1. Call with serdesNum [0] and valid dfeResPtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = 0;
                st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                    1.1.1. Call with serdesNum [maxSerdesNum/2] and
                            valid dfeResPtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = maxSerdesNum/2;
                st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                    1.1.1. Call with serdesNum [maxSerdesNum] and
                            valid dfeResPtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = maxSerdesNum;
                st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                    1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and
                            other valid parameters same as 1.1.1.
                    Expected: GT_BAD_PARAM.
                */
                serdesNum = maxSerdesNum + 1;
                st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                                                serdesNum);

                /* restore valid values */
                serdesNum = 0;

                /*
                    1.1.3. Call with invalid dfeResPtr [NULL] and other
                            valid parameters same as 1.1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum,
                                                                    serdesNum);
            }
            else
            {
                serdesNum = 0;
                st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                                                    serdesNum);
            }
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        serdesNum = 0;

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        serdesNum = 0;
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    serdesNum = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                      UTF_LION_E | UTF_XCAT2_E );
    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEomMatrixGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  GT_U32                           serdesNum,
    IN  GT_U32                           samplingTime,
    OUT GT_U32                           *rowSizePtr,
    OUT CPSS_DXCH_PORT_EOM_MATRIX_STC    *matrixPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEomMatrixGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2)
    1.1.1. Call with serdesNum [0 / maxSerdesNum/2 / maxSerdesNum],
                     samplingTime [10],
                     valid rowSizePtr [non-NULL],
                     valid matrixPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with invalid samplingTime [0] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with samplingTime [0xFFFFFFFF] and other
        valid parameters same as 1.1.1.
    Expected: GT_OK.
    1.1.5. Call with invalid rowSizePtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR
    1.1.6. Call with invalid matrixPtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
*/
    GT_U8                         devNum           = 0;

#ifndef ASIC_SIMULATION
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode  = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
    GT_PHYSICAL_PORT_NUM          portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_STATUS                     st               = GT_OK;
    GT_U32                        maxSerdesNum     = 0;
    GT_U32                        serdesNum        = 0;
    GT_U32                        startSerdes      = 0;

    GT_U32                        samplingTime     = 10;
    GT_U32                        numOfSerdesLanes = 0;
    GT_U32                        seed;
    GT_U32                        rowSize          = 0;
    CPSS_DXCH_PORT_EOM_MATRIX_STC matrix           = {{{0}}};
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT serdesLbMode;
    GT_BOOL                       isLinkUp;
#endif
    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                      UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* get random seed */
    seed = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(seed);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /* Get maxSerdesNum for specific devNum and portNum*/
            st = prvUtfPortInterfaceModeGet(devNum,portNum,&ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", devNum, portNum);

            st = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                             &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* Save port's lb mode and enable loopback for testing*/
            st = cpssDxChPortSerdesLoopbackModeGet(devNum, portNum, 0, &serdesLbMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, 0);

            st = cpssDxChPortSerdesLoopbackModeSet(devNum, portNum,
                                                   /* we interested in loopback on all serdes of port
                                                      at this moment 8 serdes is max per port,
                                                      if there would be less, function inside
                                                      will solve it */
                                                   0xff,
                                                   CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, 0xff,
                                        CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);

            /* If there is no link up return port to previous state and skip test*/
            st = cpssDxChPortLinkStatusGet(devNum,portNum, &isLinkUp);
            if ((st != GT_OK) || (!isLinkUp))
            {
                st = cpssDxChPortSerdesLoopbackModeSet(devNum, portNum, 0xff,
                                                       serdesLbMode);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, 0xff,
                                            serdesLbMode);
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, isLinkUp, devNum, portNum);

            if (numOfSerdesLanes == 0)
            {
                maxSerdesNum = 0;
            }
            else
            {
                maxSerdesNum = numOfSerdesLanes - 1;
            }

            /*
                1.1.1. Call with serdesNum [0],
                                 samplingTime [10],
                                 valid rowSizePtr [non-NULL],
                                 valid matrixPtr [non-NULL].
                Expected: GT_OK.
            */
            if ((cpssOsRand()%PORT_EOM_TOTAL_PORTS_PER_CORE) <
                                        PORT_EOM_TOTAL_PORTS_PER_CORE_TO_CHECK)
            {
                serdesNum    = 0;
                samplingTime = 10;

                st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum,
                                                  samplingTime, &rowSize, &matrix);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);
            }


            /*
                1.1.1. Call with serdesNum [maxSerdesNum/2],
                                 samplingTime [10],
                                 valid rowSizePtr [non-NULL],
                                 valid matrixPtr [non-NULL].
                Expected: GT_OK.
            */
            if ((serdesNum != maxSerdesNum/2) &&
                ((cpssOsRand()%PORT_EOM_TOTAL_PORTS_PER_CORE) <
                                        PORT_EOM_TOTAL_PORTS_PER_CORE_TO_CHECK))
            {
                serdesNum = maxSerdesNum/2;
                st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum,
                                                  samplingTime, &rowSize, &matrix);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);
            }


            /*
                1.1.1. Call with serdesNum [maxSerdesNum],
                                 samplingTime [10],
                                 valid rowSizePtr [non-NULL],
                                 valid matrixPtr [non-NULL].
                Expected: GT_OK.
            */
            if ((serdesNum != maxSerdesNum) &&
                ((cpssOsRand()%PORT_EOM_TOTAL_PORTS_PER_CORE) <
                                        PORT_EOM_TOTAL_PORTS_PER_CORE_TO_CHECK))
            {
                serdesNum = maxSerdesNum;
                st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum,
                                                  samplingTime, &rowSize, &matrix);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);
            }

            /*
                1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and
                    other valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            serdesNum = maxSerdesNum + 1;

            st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum,
                                              samplingTime, &rowSize, &matrix);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, serdesNum);

            /* restore valid values */
            serdesNum = 0;

            /*
                1.1.3. Call with invalid samplingTime [0] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            samplingTime = 0;

            st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum,
                                              samplingTime, &rowSize, &matrix);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, serdesNum);


            /* restore valid values */
            samplingTime = 10;

            /*
                1.1.5. Call with invalid rowSizePtr [NULL] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PTR
            */
            st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum, samplingTime,
                                          NULL, &matrix);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum, serdesNum);

            /*
                1.1.6. Call with invalid matrixPtr [NULL] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum,
                                              samplingTime, &rowSize, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum, serdesNum);

            /* Return port lb to its rpevious state*/
            st = cpssDxChPortSerdesLoopbackModeSet(devNum, portNum, 0xff,
                                                   serdesLbMode);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, 0xff,
                                        serdesLbMode);
        }

        /* restore valid values */
        serdesNum = 0;
        samplingTime = 10;

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum,
                                              samplingTime, &rowSize, &matrix);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        serdesNum = 0;
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum, samplingTime, &rowSize, &matrix);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    serdesNum = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                      UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                      UTF_LION_E | UTF_XCAT2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum, samplingTime,
                                      &rowSize, &matrix);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum, samplingTime,
                                  &rowSize, &matrix);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
#endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesAutoTuneStatusGet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesAutoTuneStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2, SIP5)
    1.1.1. Call with valid rxTuneStatusPtr [non-NULL] and
        valid txTuneStatusPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid rxTuneStatusPtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.3. Call with invalid txTuneStatusPtr [NULL] and other
        valid parameters same as 1.1.1..
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st          =   GT_OK;
    GT_U8                       devNum      =   0;
    GT_PHYSICAL_PORT_NUM        portNum     =   PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus =
                                        CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus =
                                        CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                         UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                     &rxTuneStatus, &txTuneStatus);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,portNum);
                continue;
            }
            /*
                1.1.1. Call with valid rxTuneStatusPtr [non-NULL] and
                    valid txTuneStatusPtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
            if(GT_NOT_INITIALIZED != st)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,portNum);
            }

            /*
                1.1.2. Call with invalid rxTuneStatusPtr [NULL] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                          NULL, &txTuneStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,portNum);

            /*
                1.1.3. Call with invalid txTuneStatusPtr [NULL] and other
                    valid parameters same as 1.1.1..
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                          &rxTuneStatus, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,portNum);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                         UTF_LION_E | UTF_XCAT2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesPpmGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_32                *ppmValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesPpmGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2, SIP5)
    1.1.1. Call with valid ppmValuePtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid ppmValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st          =  GT_OK;
    GT_U8                   devNum      =  0;
    GT_PHYSICAL_PORT_NUM    portNum     =  PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_32                   ppmValue    =  0;
    GT_32                   ppmValueGet    =  0;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", devNum, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

             /*
                1.1.1. Call with valid ppmValue [-100].
                Expected: GT_OK.
            */
            ppmValue = -100;

            st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValueGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                continue;
            }
           /*
                1.1.1. Call with valid ppmValuePtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValueGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
                 /* verifying values */
                UTF_VERIFY_EQUAL3_PARAM_MAC(((ppmValue/30)*30), ppmValueGet,
                                                 devNum, portNum, ppmValueGet);

            /*
                1.1.2. Call with invalid ppmValuePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSerdesPpmGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesPpmSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                ppmValue
)
 */
UTF_TEST_CASE_MAC(cpssDxChPortSerdesPpmSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2, SIP5)
    1.1.1. Call with valid ppmValue [-100 / 0 / 100].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSerdesPpmGet().
    Expected: GT_OK and closest rounded value was set in 1.1.1((ppmValue/30)*30).
    1.1.3. Call with out of range ppmValue [-101 / 101]
    Expected: GT_OUT_OF_RANGE.

*/
    GT_STATUS               st          =   GT_OK;
    GT_U8                   devNum      =   0;
    GT_PHYSICAL_PORT_NUM    portNum     =   PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_32                   ppmValue    =   0;
    GT_32                   ppmValueGet =   0;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6128);

        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", devNum, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            /*
                1.1.1. Call with valid ppmValue [-100].
                Expected: GT_OK.
            */
            ppmValue = -100;
            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                continue;
            }

            st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortSerdesPpmGet().
                Expected: GT_OK and closest rounded value was set
                    in 1.1.1((ppmValue/30)*30).
            */
            st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValueGet);
            if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode ) && ((portNum & 0x3) > 0))
            {
                if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    continue;
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL3_PARAM_MAC(((ppmValue/30)*30), ppmValueGet,
                                                 devNum, portNum, ppmValueGet);

            }
            /*
                1.1.1. Call with ppmValue [0].
                Expected: GT_OK.
            */
            ppmValue = 0;

            st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortSerdesPpmGet().
                Expected: Expected: GT_OK and closest rounded value was set
                    in 1.1.1((ppmValue/30)*30).
            */
            st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValueGet);
            if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode ) && ((portNum & 0x3) > 0))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL3_PARAM_MAC(((ppmValue/30)*30), ppmValueGet,
                                                 devNum, portNum, ppmValueGet);

            }

            /*
                1.1.1. Call with ppmValue [100].
                Expected: GT_OK.
            */
            ppmValue = 100;

            st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortSerdesPpmGet().
                Expected: Expected: GT_OK and closest rounded value was set
                    in 1.1.1((ppmValue/30)*30).
            */
            st = cpssDxChPortSerdesPpmGet(devNum, portNum, &ppmValueGet);
            if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode ) && ((portNum & 0x3) > 0))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL3_PARAM_MAC(((ppmValue/30)*30), ppmValueGet,
                                                 devNum, portNum, ppmValueGet);

            }

            /*
                1.1.3. Call with out of range ppmValue [-101].
                Expected: GT_OUT_OF_RANGE.
            */
            ppmValue = -101;

            st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum);

            /*
                1.1.3. Call with out of range ppmValue [101].
                Expected: GT_OUT_OF_RANGE.
            */
            ppmValue = 101;

            st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    ppmValue = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGePrbsIntReplaceEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGePrbsIntReplaceEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat2)
    1.1.1. Call with valid enablePtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st      = GT_OK;
    GT_U8                   devNum  = 0;
    GT_PHYSICAL_PORT_NUM    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable  = GT_FALSE;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                      UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with valid enablePtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with invalid enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                      UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGePrbsIntReplaceEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGePrbsIntReplaceEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat2)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortGePrbsIntReplaceEnableGet().
    Expected: GT_OK and the same parameters value as was set in 1.1.1.
*/
    GT_STATUS            st         = GT_OK;
    GT_U8                devNum     = 0;
    GT_PHYSICAL_PORT_NUM portNum    = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL              enable     = GT_FALSE;
    GT_BOOL              enableGet  = GT_FALSE;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                      UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChPortGePrbsIntReplaceEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortGePrbsIntReplaceEnableGet().
                Expected: GT_OK and the same parameters value as was set
                    in 1.1.1.
            */
            st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum,
                                                                   &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(enable, enableGet, devNum, portNum);

            /*
                1.1.1. Call with enable [GT_FALSE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChPortGePrbsIntReplaceEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortGePrbsIntReplaceEnableGet().
                Expected: GT_OK and the same parameters value as was set
                    in 1.1.1.
            */
            st = cpssDxChPortGePrbsIntReplaceEnableGet(devNum, portNum,
                                                                   &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(enable, enableGet, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortGePrbsIntReplaceEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortGePrbsIntReplaceEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                      UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortGePrbsIntReplaceEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGePrbsIntReplaceEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInbandAutoNegRestart
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortInbandAutoNegRestart)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat, Lion, xCat2, Lion2, SIP5)
*/
    GT_STATUS               st          =   GT_OK;
    GT_U8                   devNum      =   0;
    GT_PHYSICAL_PORT_NUM    portNum     =   PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,
                    UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                st = cpssDxChPortInbandAutoNegRestart(devNum, portNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                continue;
            }

            st = cpssDxChPortInbandAutoNegRestart(devNum, portNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortInbandAutoNegRestart(devNum, portNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.*/

        portNum =UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);


        st = cpssDxChPortInbandAutoNegRestart(devNum, portNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInbandAutoNegRestart(devNum, portNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,
                    UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortInbandAutoNegRestart(devNum, portNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInbandAutoNegRestart(devNum, portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPcsLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPcsLoopbackModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2; SIP5)
    1.1.1. Call with valid modePtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                            st      = GT_OK;
    GT_U8                                devNum  = 0;
    GT_PHYSICAL_PORT_NUM                 portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT mode;
    CPSS_PORT_INTERFACE_MODE_ENT         ifMode;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_XCAT2_E | UTF_LION_E
    /* in HWS for Lion2 only PCS loopback set implemented, get - not implemented */
    | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", devNum, portNum);

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                continue;
            }
           /*
                1.1.1. Call with valid modePtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with invalid modePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                physical ports.
        */
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPcsLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPcsLoopbackModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2; SIP5)
    1.1.1. Call with mode [CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E,
                           CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E,
                           CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortPcsLoopbackModeGet().
    Expected: GT_OK and the same parameters value as was set in 1.1.1.
    1.1.3. Call with out of range enum value mode and other parameters as 1.1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS            st         = GT_OK;
    GT_U8                devNum     = 0;
    GT_PHYSICAL_PORT_NUM portNum    = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    mode    = 0;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    modeRet = 0;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
    GT_BOOL valid;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_XCAT2_E | UTF_LION_E
    /* in HWS for Lion2 only PCS loopback set implemented, get - not implemented */
    | UTF_LION2_E);

        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                st = cpssDxChPortPhysicalPortDetailedMapGet(devNum,portNum,&portMapShadow);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                valid = portMapShadow.valid;
            }
            else
            {
                valid = GT_TRUE;
            }

            if(valid == GT_TRUE)
            {
                st = prvUtfPortTypeGet(devNum, portNum, &portType);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d", devNum, portNum);

                st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPortInterfaceModeGet: %d, %d", devNum, portNum);

                if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
                {
                    continue;
                }
                /*
                    1.1.1. Call with mode [CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E,
                                           CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E,
                                           CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E].
                    Expected: GT_OK.
                */
                mode = CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E;

                if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
                {
                    st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                    continue;
                }

                st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
                if(portType == PRV_CPSS_PORT_GE_E || portType==PRV_CPSS_PORT_CG_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, devNum, portNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                    /*
                        1.1.2. Call cpssDxChPortPcsLoopbackModeGet().
                        Expected: GT_OK and the same parameters value as was set.
                    */
                    st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &modeRet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                    /* verifying values */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeRet, devNum, portNum);
                }

                /*
                    1.1.1. Call with mode [CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E,
                                           CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E,
                                           CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E].
                    Expected: GT_OK.
                */
                mode = CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E;

                st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*
                    1.1.2. Call cpssDxChPortPcsLoopbackModeGet().
                    Expected: GT_OK and the same parameters value as was set.
                */
                st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &modeRet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeRet, devNum, portNum);

                /*
                    1.1.1. Call with mode [CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E,
                                           CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E,
                                           CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E].
                    Expected: GT_OK.
                */
                mode = CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E;

                st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*
                    1.1.2. Call cpssDxChPortPcsLoopbackModeGet().
                    Expected: GT_OK and the same parameters value as was set.
                */
                st = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &modeRet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeRet, devNum, portNum);

                /*
                    1.1.3. Call with out of range enum value mode
                        and other parameters as 1.1.1.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChPortPcsLoopbackModeSet
                                    (devNum, portNum, mode),
                                    mode);
            }
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_XCAT2_E | UTF_LION_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPeriodicFlowControlCounterSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPeriodicFlowControlCounterSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with value [0 / 250].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortPeriodicFlowControlCounterSet with non-NULL value.
    Expected: GT_OK and value the same as just written.
*/
    GT_STATUS   st       = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      value    = 0;
    GT_U32      valueGet = 0;

    /* previous tests generate link change events. Provide time to take care of events. */
    cpssOsTimerWkAfter(1000);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support FCA registers */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with value [0 / 250].
               Expected: GT_OK.
            */
            /* iterate with value = 0 */
            value = 0;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortPeriodicFlowControlCounterSet(dev, port, value);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, value);
                continue;
            }

            st = cpssDxChPortPeriodicFlowControlCounterSet(dev, port, value);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, value);

            /*
                1.1.2. Call cpssDxChPortPeriodicFlowControlCounterGet with non-NULL value.
                Expected: GT_OK and value the same as just written.
            */
            st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &valueGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPeriodicFlowControlCounterGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(value, valueGet,
                                         "got another value as was written: %d, %d", dev, port);

            /* iterate with value = 250 */
            value = 250;

            st = cpssDxChPortPeriodicFlowControlCounterSet(dev, port, value);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, value);

            /*
                1.1.2. Call cpssDxChPortMacSaLsbGet with non-NULL macSaLsb.
                Expected: GT_OK and macSaLsb the same as just written.
            */
            st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &valueGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPeriodicFlowControlCounterGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(value, valueGet,
                                         "got another value as was written: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        value = 0;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPeriodicFlowControlCounterSet(dev, port, value);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPeriodicFlowControlCounterSet(dev, port, value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    value = 0;
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPeriodicFlowControlCounterSet(dev, port, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssDxChPortPeriodicFlowControlCounterSet(dev, port, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static GT_STATUS prvCpssDxChPortEyeResultMemoryFree
(
    OUT CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
{
    if(NULL != eye_resultsPtr->matrixPtr)
    {
        cpssOsFree(eye_resultsPtr->matrixPtr);
        eye_resultsPtr->matrixPtr = NULL;
    }
    if(NULL != eye_resultsPtr->vbtcPtr)
    {
        cpssOsFree(eye_resultsPtr->vbtcPtr);
        eye_resultsPtr->vbtcPtr = NULL;
    }
    if(NULL != eye_resultsPtr->hbtcPtr)
    {
        cpssOsFree(eye_resultsPtr->hbtcPtr);
        eye_resultsPtr->hbtcPtr = NULL;
    }
    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSerdesEyeMatrixGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN  CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC  *eye_inputPtr,
    OUT CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSerdesEyeMatrixGet)
{
    GT_STATUS                                 st       = GT_OK;
    GT_U8                                     dev      = 0;
    GT_PHYSICAL_PORT_NUM                      port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                                    serdes   = 0;
    GT_U32                                    portMacNum = 0;
    GT_U32                                    numOfSerdesLanes = 0;
    GT_U32                                    startSerdes = 0;
    OUT CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC  eye_results;
    CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC       eye_input;
   /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E
                                          | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E);

    eye_results.matrixPtr = NULL;
    eye_results.vbtcPtr = NULL;
    eye_results.hbtcPtr = NULL;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                eye_input.min_dwell_bits = 100000;
                eye_input.max_dwell_bits = 100000000;
                st = cpssDxChPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                prvCpssDxChPortEyeResultMemoryFree(&eye_results);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                continue;
            }

            if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(dev) == GT_TRUE)
            {
                st = prvCpssDxChPortPhysicalPortMapCheckAndConvert(dev, port, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortPhysicalPortMapCheckAndConvert: device: %d, port: %d", dev, port);
            }
            else
            {
                if((port >= PRV_CPSS_PP_MAC(dev)->numOfPorts) ||
                    (0 == (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&(PRV_CPSS_PP_MAC(dev)->existingPorts) , port))))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, GT_BAD_PARAM, "prvCpssDxChPortPhysicalPortMapCheckAndConvert: device: %d, port: %d", dev, port);
                }
                else
                {
                    portMacNum = port;
                }
            }

            st = prvCpssDxChPortIfModeSerdesNumGet(dev, port,
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(dev, portMacNum),
                            &startSerdes, &numOfSerdesLanes);
            if(GT_OK == st)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet: device: %d, port: %d", dev, port);
                for(serdes = 0; serdes < numOfSerdesLanes; serdes++)
                {
                    eye_input.min_dwell_bits = 100000;
                    eye_input.max_dwell_bits = 100000000;
                    st = cpssDxChPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssDxChPortEyeResultMemoryFree(&eye_results);
#ifndef ASIC_SIMULATION
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
#else
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_SUPPORTED, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
#endif
#ifndef ASIC_SIMULATION
                    if((NULL == eye_results.matrixPtr) || (NULL == eye_results.vbtcPtr) || (NULL == eye_results.hbtcPtr)
                        || (0 == eye_results.x_points) || (0 == eye_results.y_points))
                    {
                        st = GT_FAIL;
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_FAIL, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    }
                    st = cpssDxChPortSerdesEyeMatrixGet(dev, port, serdes, NULL, &eye_results);
                    prvCpssDxChPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    st = cpssDxChPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, NULL);
                    prvCpssDxChPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    eye_input.min_dwell_bits = 10000;
                    eye_input.max_dwell_bits = 100000000;
                    st = cpssDxChPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssDxChPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    eye_input.min_dwell_bits = 100000;
                    eye_input.max_dwell_bits = 1000000000;
                    st = cpssDxChPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssDxChPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    eye_input.min_dwell_bits = 1000000;
                    eye_input.max_dwell_bits = 100000;
                    st = cpssDxChPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssDxChPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
#endif
                }
            }
        }

#ifndef ASIC_SIMULATION
        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        eye_input.min_dwell_bits = 100000;
        eye_input.max_dwell_bits = 100000000;
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
            prvCpssDxChPortEyeResultMemoryFree(&eye_results);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChPortSerdesEyeMatrixGet: device: %d, port: %d", dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssDxChPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
        prvCpssDxChPortEyeResultMemoryFree(&eye_results);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* 1.4. For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = CPSS_CPU_PORT_NUM_CNS;
       st = cpssDxChPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
       prvCpssDxChPortEyeResultMemoryFree(&eye_results);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

#endif

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E
                                          | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
        prvCpssDxChPortEyeResultMemoryFree(&eye_results);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
    prvCpssDxChPortEyeResultMemoryFree(&eye_results);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPeriodicFlowControlCounterGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPeriodicFlowControlCounterGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with value [0 / 250].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortPeriodicFlowControlCounterGet with non-NULL value.
    Expected: GT_OK and value the same as just written.
*/
    GT_STATUS   st       = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      value    = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation() && PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {/* GM doesn't support FCA registers */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &value);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChPortPeriodicFlowControlCounterGet: %d, %d", dev, port);
                continue;
            }

            st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &value);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPeriodicFlowControlCounterGet: %d, %d", dev, port);

        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        value = 0;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &value);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    value = 0;
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssDxChPortPeriodicFlowControlCounterGet(dev, port, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

GT_STATUS prvcpssDxChPortAutoNegMasterModeEnableSetCheck
(
    GT_U8                dev,
    GT_PHYSICAL_PORT_NUM port,
    GT_BOOL              enable,
    GT_CHAR             *info
)
{
    GT_BOOL            isEnable;
    GT_STATUS          st = GT_OK;
    st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChPortAutoNegMasterModeEnableSet %s: Device %d   Port %d", info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    st = cpssDxChPortAutoNegMasterModeEnableGet(dev, port, &isEnable);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChPortAutoNegMasterModeEnableGet %s: Device %d   Port %d", info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(enable, isEnable, "cpssDxChPortAutoNegMasterModeEnableSet wrong setting or getting %s: Device %d   Port %d", info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    return st;
}


GT_STATUS prvcpssDxChPortAutoNegAdvertismentConfigSetCheck
(
    GT_U8                dev,
    GT_PHYSICAL_PORT_NUM port,
    GT_BOOL              link,
    CPSS_PORT_DUPLEX_ENT duplex_mode,
    CPSS_PORT_SPEED_ENT  speed,
    GT_CHAR              *info
)
{
    GT_STATUS                                st  = GT_OK;
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC  portAnAdvertisment;
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC  portAnAdvertismentGet;

    portAnAdvertisment.link = link;
    portAnAdvertisment.duplex = duplex_mode;
    portAnAdvertisment.speed = speed;
    st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChPortAutoNegAdvertismentConfigSet %s: Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    st = cpssDxChPortAutoNegAdvertismentConfigGet(dev, port, &portAnAdvertismentGet);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChPortAutoNegAdvertismentConfigGet %s: Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(portAnAdvertisment.link, portAnAdvertismentGet.link,
                "cpssDxChPortAutoNegMasterModeEnableSet wrong setting or getting port link - %s : Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(portAnAdvertisment.duplex, portAnAdvertismentGet.duplex,
                "cpssDxChPortAutoNegMasterModeEnableSet wrong setting or getting port duplex mode - %s : Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(portAnAdvertisment.speed, portAnAdvertismentGet.speed,
                "cpssDxChPortAutoNegMasterModeEnableSet wrong setting or getting port speed - %s : Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChIsMapAndNotCPUCheck
(
    GT_U8                              dev,
    GT_PHYSICAL_PORT_NUM               port
)
{
    GT_STATUS                          st;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;

    /* Checking for port mapping */
    st = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapPtr);
    if (st != GT_OK)
    {
        return GT_FAIL;
    }
    if (portMapPtr->valid == GT_FALSE)
    {
        return GT_FAIL;
    }
    if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        return GT_FAIL;
    }
    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*static GT_STATUS prvCpssDxChAutoNegMasterModeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)*/
UTF_TEST_CASE_MAC(cpssDxChPortAutoNegMasterModeEnableSet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

       /* Save active ports parameters */
       st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);
       if(GT_OK != st)
       {
           return;
       }

       /* Deactivation ports */
       st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
       if(GT_OK != st)
       {
           isError = GT_TRUE;
           goto restore;
       }

       for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
       {

           if((portActive.Data[portActive.Current].macNum > 47) || (0 == portActive.Data[portActive.Current].macNum % 4))
           {
              /* Activate tested port */
               st = prvPortActivateDeactivate(dev, GT_TRUE,
                                            portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                            portActive.Data[portActive.Current].speed);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev,
                   portActive.Data[portActive.Current].portNum );
               if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
               {
                   isError = GT_TRUE;
                   goto restore;
               }

               /*CPU port 32 in Aldrin is not supported*/
               if ((PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev)) && portActive.Data[portActive.Current].portNum == 32)
               {
                   continue;
               }

               /* Checking for port mapping */
               st = prvCpssDxChIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
               if (st != GT_OK)
               {
                   continue;
               }

               /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
               st = cpssDxChPortAutoNegMasterModeEnableSet(dev, portActive.Data[portActive.Current].portNum, GT_TRUE);
               if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data->ifMode)
               {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d",
                         dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
               }
               else if((portActive.Data->speed != CPSS_PORT_SPEED_10_E) && (portActive.Data->speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data->speed != CPSS_PORT_SPEED_1000_E))
               {
                   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d",
                       dev, portActive.Data[portActive.Current].portNum );
                   if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                   {
                       isError = GT_TRUE;
                       goto restore;
                   }
               }

               CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
               CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

               /* Set port interface QSGMII and port speed 1000 Mbps */
               st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
               if(GT_OK == st)
               {
                    st = cpssDxChPortAutoNegMasterModeEnableSet(dev, portActive.Data[portActive.Current].portNum, GT_TRUE);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
               }

               /* Set port interface SGMII and port speed 2500 Mbps */
               st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
               if(GT_OK == st)
               {
                    st = cpssDxChPortAutoNegMasterModeEnableSet(dev, portActive.Data[portActive.Current].portNum, GT_TRUE);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
               }

               /* Set port interface SGMII and port speed 1000 Mbps */
               st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
               if(GT_OK == st)
               {
                   /* Setting Master Mode enable and checking */
                   st = prvcpssDxChPortAutoNegMasterModeEnableSetCheck( dev, portActive.Data[portActive.Current].portNum, GT_TRUE, "enable");
                   if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                   {
                        isError = GT_TRUE;
                        goto restore;
                   }

                   /* Setting Master Mod disable and checking */
                   st = prvcpssDxChPortAutoNegMasterModeEnableSetCheck( dev, portActive.Data[portActive.Current].portNum, GT_FALSE, "disable");
                   if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                   {
                        isError = GT_TRUE;
                        goto restore;
                   }
               }
               /* Deactivate tested port */
               st = prvPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                   portActive.Data[portActive.Current].portNum );
               if(GT_OK != st)
               {
                   isError = GT_TRUE;
                   goto restore;
               }
           }
       }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                    */
       port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                       */
       port = CPSS_CPU_PORT_NUM_CNS;
       st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

   }

   port = PORT_CTRL_VALID_PHY_PORT_CNS;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
   /* Go over all non active devices. */
   while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
   {
       st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
       st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, GT_FALSE);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
   }

   /* Call function with out of bound value for device id */
   dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
   st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
   st = cpssDxChPortAutoNegMasterModeEnableSet(dev, port, GT_FALSE);
   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChPortAutoNegMasterModeEnableGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPortAutoNegMasterModeEnableGet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    GT_BOOL                                              enable;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;

    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Save active ports parameters */
        st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
        {
            return;
        }

        /* Deactivation ports */
        st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
        if(GT_OK != st)
        {
            isError = GT_TRUE;
            goto restore;
        }

        /* For all active devices go over all available physical ports. */
        for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
        {

            if((portActive.Data[portActive.Current].macNum > 47) || (0 == portActive.Data[portActive.Current].macNum % 4))
            {
                /* Activate tested port */
                st = prvPortActivateDeactivate(dev, GT_TRUE,
                                            portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                            portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* Checking for Bad pointer */
                st = cpssDxChPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, NULL);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_BAD_PTR, st,"Bad Pointer   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* For all active devices go over all non available physical ports. */
                st = prvCpssDxChIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
                if (st != GT_OK)
                {
                    continue;
                }

                 /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
                st = cpssDxChPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data[portActive.Current].ifMode)
                {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
                }
                else if((portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_10_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_1000_E))
                {
                   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d",
                       dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

                /* Set port interface QSGMII and port speed 1000 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    st = cpssDxChPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d",
                        dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 2500 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
                if(GT_OK == st)
                {
                    st = cpssDxChPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 1000 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);

                if(GT_OK == st)
                {
                     /* Getting Master Mode */
                    st = cpssDxChPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChPortAutoNegMasterModeEnableGet : Device %d  Port %d",
                        dev, portActive.Data[portActive.Current].portNum);
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Deactivate tested port */
                st = prvPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                    portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
        }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssDxChPortAutoNegMasterModeEnableGet(dev, port, &enable);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                         */
       port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssDxChPortAutoNegMasterModeEnableGet(dev, port, &enable);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = CPSS_CPU_PORT_NUM_CNS;
       st = cpssDxChPortAutoNegMasterModeEnableGet(dev, port, &enable);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortAutoNegMasterModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortAutoNegMasterModeEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortAutoNegAdvertismentConfigSet
(
    IN   GT_U8                               devNum,
    IN   GT_PHYSICAL_PORT_NUM                portNum,
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPortAutoNegAdvertismentConfigSet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC              portAnAdvertisment;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

       /* Save active ports parameters */
       st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);
       if(GT_OK != st)
       {
           return;
       }

       /* Deactivation ports */
       st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
       if(GT_OK != st)
       {
           isError = GT_TRUE;
           goto restore;
       }

       /* For all active devices go over all available physical ports. */
       for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
       {

           if((portActive.Data[portActive.Current].macNum > 47) || (0 == portActive.Data[portActive.Current].macNum % 4))
           {

                /* Activate tested port */
                st = prvPortActivateDeactivate(dev, GT_TRUE,
                                               portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                               portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* Checking for port mapping */
                st = prvCpssDxChIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
                if (st != GT_OK)
                {
                    continue;
                }

                /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
                portAnAdvertisment.link = GT_TRUE;
                portAnAdvertisment.duplex = CPSS_PORT_FULL_DUPLEX_E;
                portAnAdvertisment.speed = CPSS_PORT_SPEED_1000_E;
                st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data[portActive.Current].ifMode)
                {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
                }
                else if((portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_10_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_1000_E))
                {
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

                /* Set port interface QSGMII and port speed 1000 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

             /* Set port interface SGMII and port speed 2500 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
                if(GT_OK == st)
                {
                    st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 1000 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    /* Setting Advertisment Configuration and checking */
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_100_E, " link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_10_E, " link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_100_E, " link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_10_E, " link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " no link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_100_E, " no link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_10_E, " no link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " no link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_100_E, " no link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssDxChPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_10_E, " no link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }
                /* Deactivate tested port */
                st = prvPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
           }
      }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                         */
       port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = CPSS_CPU_PORT_NUM_CNS;
       st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

   }

   port = PORT_CTRL_VALID_PHY_PORT_CNS;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
   /* Go over all non active devices. */
   while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
   {
       st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
   }

   /* Call function with out of bound value for device id */
   dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
   st = cpssDxChPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortAutoNegAdvertismentConfigGet
(
    IN   GT_U8                                    devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    OUT  CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPortAutoNegAdvertismentConfigGet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC              portAnAdvertisment;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Save active ports parameters */
        st = prvCpssDxChSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
         {
            return;
        }

        /* Deactivation ports */
        st = prvPortsActivateDeactivate(dev, GT_FALSE, &portActive);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
        if(GT_OK != st)
        {
            isError = GT_TRUE;
            goto restore;
        }

        /* For all active devices go over all available physical ports. */
        for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
        {

            if((portActive.Data[portActive.Current].macNum > 47) || (0 == portActive.Data[portActive.Current].macNum % 4))
            {
                /* Activate tested port */
                st = prvPortActivateDeactivate(dev, GT_TRUE,
                                            portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                            portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* Checking for Bad pointer */
                st = cpssDxChPortAutoNegAdvertismentConfigGet(dev, portActive.Data[portActive.Current].portNum, NULL);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_BAD_PTR, st,"Bad Pointer   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* For all active devices go over all non available physical ports. */
                st = prvCpssDxChIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
                if (st != GT_OK)
                {
                    continue;
                }

                 /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
                st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data[portActive.Current].ifMode)
                {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
                }
                else if((portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_10_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_1000_E))
                {
                   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

                /* Set port interface QSGMII and port speed 1000 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 2500 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
                if(GT_OK == st)
                {
                    st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 1000 Mbps */
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                     /* Getting Advertisement Config */
                    st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChPortAutoNegAdvertismentConfigGet : Device %d  Port %d", dev, portActive.Data[portActive.Current].portNum);
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                            isError = GT_TRUE;
                            goto restore;
                    }
                }

                /* Deactivate tested port */
                st = prvPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev, portActive.Data->portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
        }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                         */
       port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = CPSS_CPU_PORT_NUM_CNS;
       st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortVosOverrideControlModeGet
(
    IN   GT_U8       devNum,
    OUT  GT_BOOL     *vosOverridePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortVosOverrideControlModeGet)
{
    GT_U8    devNum;
    GT_BOOL  vosOverride;
    GT_STATUS st;

    vosOverride = GT_FALSE;

    /* prepare not applicable device iterator (only Caelum is applicable) */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, (UTF_TWISTC_E | UTF_TWISTD_E | UTF_SAMBA_E | UTF_TIGER_E | UTF_SALSA_E |
             UTF_PUMA2_E | UTF_PUMA3_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
             UTF_XCAT_E | UTF_LION_E  | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E ));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortVosOverrideControlModeGet(devNum, &vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* prepare applicable device iterator (Caelum v1 and above, Aldrin, AC3X and Bobcat3) */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, (UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E));

    /* API applicable for only Bobk rev 1 and above, Aldrin, AC3X and Bobcat3 */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*PRV_CPSS_PP_MAC(devNum)->revision = 1; */
        /* Only BobK revision 1 and above, Aldrin, AC3X and Bobcat3 are applicable */
        if ( PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E &&
             PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E &&
             PRV_CPSS_PP_MAC(devNum)->revision > 0) )
        {
            /* Setting VOS override mode to true */
            vosOverride = GT_TRUE;
            st = cpssDxChPortVosOverrideControlModeSet(devNum, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            st = cpssDxChPortVosOverrideControlModeGet(devNum, &vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, vosOverride, vosOverride);
            /* Setting VOS override mode to false */
            vosOverride = GT_FALSE;
            st = cpssDxChPortVosOverrideControlModeSet(devNum, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            st = cpssDxChPortVosOverrideControlModeGet(devNum, &vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, vosOverride, vosOverride);
        }
        else
        {
            st = cpssDxChPortVosOverrideControlModeGet(devNum, &vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
    }

    /* Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortVosOverrideControlModeGet(devNum, &vosOverride);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortVosOverrideControlModeSet
(
    IN   GT_U8       devNum,
    IN  GT_BOOL     vosOverride
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortVosOverrideControlModeSet)
{
    GT_U8    devNum;
    GT_BOOL  vosOverride;
    GT_STATUS st;

    vosOverride = GT_FALSE;

    /* prepare not applicable device iterator (only Caelum is applicable) */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, (UTF_TWISTC_E | UTF_TWISTD_E | UTF_SAMBA_E | UTF_TIGER_E | UTF_SALSA_E |
             UTF_PUMA2_E | UTF_PUMA3_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
            UTF_XCAT_E | UTF_LION_E  | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E ));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortVosOverrideControlModeSet(devNum, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* prepare applicable device iterator (Caelum v1 and above, Aldrin, AC3X and Bobcat3) */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, (UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E));

    /* API applicable for only Bobk rev 1 and above, Aldrin, AC3X and Bobcat3 */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*PRV_CPSS_PP_MAC(devNum)->revision = 1;*/
        /* Only BobK revision 1 and above, Aldrin, AC3X and Bobcat3 are applicable */
        if ( PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E &&
             PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E &&
             PRV_CPSS_PP_MAC(devNum)->revision > 0) )
        {
            /* Setting VOS override mode to true */
            vosOverride = GT_TRUE;
            st = cpssDxChPortVosOverrideControlModeSet(devNum, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            st = cpssDxChPortVosOverrideControlModeGet(devNum, &vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, vosOverride, vosOverride);
            /* Setting VOS override mode to false */
            vosOverride = GT_FALSE;
            st = cpssDxChPortVosOverrideControlModeSet(devNum, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            st = cpssDxChPortVosOverrideControlModeGet(devNum, &vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, vosOverride, vosOverride);
        }
        else
        {
            st = cpssDxChPortVosOverrideControlModeSet(devNum, GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
    }

    /* Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortVosOverrideControlModeSet(devNum, vosOverride);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*GT_STATUS internal_cpssDxChPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U16                          value
)*/
UTF_TEST_CASE_MAC(cpssDxChPortXlgReduceAverageIPGSet)
{
    GT_STATUS                       st   = GT_OK;
    GT_U8                           dev  = 0;
    GT_PHYSICAL_PORT_NUM            port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                         isFlexLink = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    GT_U32                          valueSource = 0x0000000C;
    GT_U32                          valueResult = 0;

    /* prepare not applicable device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ( UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
            UTF_XCAT_E | UTF_LION_E  | UTF_XCAT2_E | UTF_LION2_E | UTF_XCAT3_E ));

        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifdef GM_USED
        /* test is not ready for GM for Bobk */
        if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
        {
            continue;
        }
#endif

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            if(CPSS_PORT_INTERFACE_MODE_NA_E != portIfMode)
            {
                st = prvUtfPortIsFlexLinkGet(dev, port, &isFlexLink);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                st = cpssDxChPortXlgReduceAverageIPGSet(dev, port, valueSource);
                if(GT_TRUE == isFlexLink)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(valueSource, valueResult, dev, port);
                    st = cpssDxChPortXlgReduceAverageIPGSet(dev, port, 0);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(0, valueResult, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
            }
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortXlgReduceAverageIPGSet(dev, port, valueSource);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXlgReduceAverageIPGSet(dev, port, valueSource);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortXlgReduceAverageIPGSet(dev, port, valueSource);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXlgReduceAverageIPGSet(dev, port, valueSource);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXlgReduceAverageIPGSet(dev, port, valueSource);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*GT_STATUS cpssDxChPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U16                          value
)*/
UTF_TEST_CASE_MAC(cpssDxChPortXlgReduceAverageIPGGet)
{
    GT_STATUS                       st   = GT_OK;
    GT_U8                           dev  = 0;
    GT_PHYSICAL_PORT_NUM            port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                         isFlexLink = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    GT_U32                          valueResult = 0;

    /* prepare not applicable device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ( UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
            UTF_XCAT_E | UTF_LION_E  | UTF_XCAT2_E | UTF_LION2_E | UTF_XCAT3_E ));
        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            if(CPSS_PORT_INTERFACE_MODE_NA_E != portIfMode)
            {
                st = prvUtfPortIsFlexLinkGet(dev, port, &isFlexLink);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
                if(GT_TRUE == isFlexLink)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, NULL);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
            }
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXlgReduceAverageIPGGet(dev, port, &valueResult);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortCtrl suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortCtrl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortAutoNegMasterModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortAutoNegMasterModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortAutoNegAdvertismentConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortAutoNegAdvertismentConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPeriodicFlowControlIntervalGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPeriodicFlowControlIntervalSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortModeSpeedAutoDetectAndConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortAttributesOnPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacSaLsbSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacSaLsbGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDuplexModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDuplexModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSpeedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSpeedGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDuplexAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFlowCntrlAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSpeedAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFlowControlEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFlowControlEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPeriodicFcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortBackPressureEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLinkStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInterfaceModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInterfaceModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForceLinkPassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForceLinkPassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForceLinkDownEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForceLinkDownEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCrcCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXGmiiModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortIpgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortIpgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortExtraIpgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXgmiiLocalFaultGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXgmiiRemoteFaultGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInternalLoopbackEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInternalLoopbackEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInbandAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPreambleLengthSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPreambleLengthGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacSaBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacSaBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesComboModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesComboModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPaddingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPaddingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortExcessiveCollisionDropEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortExcessiveCollisionDropEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXgLanesSwapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXgLanesSwapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInBandAutoNegBypassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInBandAutoNegBypassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacErrorIndicationPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacErrorIndicationPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacErrorIndicationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXgPscLanesSwapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXgPscLanesSwapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortBackPressureEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCrcCheckEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDuplexAutoNegEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortExtraIpgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFlowCntrlAutoNegEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInbandAutoNegEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortIpgBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortIpgBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPeriodicFcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSpeedAutoNegEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXGmiiModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupSerdesPowerStatusSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesGroupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesPowerStatusSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFlowControlModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFlowControlModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesCalibrationStartSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacResetStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesResetStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForward802_3xEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForward802_3xEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForwardUnknownMacControlFramesEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortForwardUnknownMacControlFramesEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesAutoTune)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesLoopbackModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesLoopbackModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesPolaritySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesPolarityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesTuningSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesTuningGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesTxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesTxEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterIfPortStateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesManualTxConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesManualTxConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEomBaudRateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEomDfeResGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEomMatrixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesAutoTuneStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesPpmGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesPpmSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGePrbsIntReplaceEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGePrbsIntReplaceEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInbandAutoNegRestart)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesManualRxConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesManualRxConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFlowControlPacketsCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortUnknownMacControlFramesCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortUnknownMacControlFramesCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortProfileMruSizeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortProfileMruSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruExceptionCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruExceptionCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruExceptionCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMruExceptionCpuCodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPcsLoopbackModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPcsLoopbackModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPeriodicFlowControlCounterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPeriodicFlowControlCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesEyeMatrixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesLaneTuningSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSerdesLaneTuningGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortVosOverrideControlModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortVosOverrideControlModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXlgReduceAverageIPGSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXlgReduceAverageIPGGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortCtrl)

