/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChFabricHGLinkUT.C
*
* DESCRIPTION:
*       Unit tests for cpssDxChFabricHGLink, that provides
*       CPSS DxCh Fabric Connectivity HyperG.Link interface API.
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*******************************************************************************/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/fabric/cpssDxChFabricHGLink.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Defines */

/* Default valid value for port id */
#define FABRIC_VALID_PHY_PORT_CNS  0

#define FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port)  \
    expectedOk = ((port%4==0) || ((port&0xF)==9)) ? GT_TRUE : GT_FALSE

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkPcsLoopbackEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with enable [GT_FALSE \ GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_TRUE;
    GT_BOOL     enableGet = GT_TRUE;
    GT_BOOL     expectedOk;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            enable = GT_FALSE;

            st = cpssDxChFabricHGLinkPcsLoopbackEnableSet(dev, port, enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            else
                 UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable than was set: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
            }

            /* 1.1.1. */
            enable = GT_TRUE;

            st = cpssDxChFabricHGLinkPcsLoopbackEnableSet(dev, port, enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                 "get another enable than was set: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
            }
        }
        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkPcsLoopbackEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkPcsLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkPcsLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsLoopbackEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkPcsLoopbackEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with idleCntEnable [GT_FALSE \ GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     idleCntEnable    = GT_TRUE;
    GT_BOOL     idleCntEnableGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            idleCntEnable = GT_FALSE;

            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(dev, port, idleCntEnable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnable);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(idleCntEnable, idleCntEnableGet,
                                             "get another idleCntEnable than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnableGet);

            /* 1.1.1. */
            idleCntEnable = GT_TRUE;

            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(dev, port, idleCntEnable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnable);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(idleCntEnable, idleCntEnableGet,
                                             "get another idleCntEnable than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, idleCntEnableGet);

        }

        idleCntEnable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(dev, port, idleCntEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(dev, port, idleCntEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(dev, port, idleCntEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    idleCntEnable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(dev, port, idleCntEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(dev, port, idleCntEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL idleCntEnablePtr.
    Expected: GT_OK.
    1.1.2. Call with idleCntEnablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     idleCntEnable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, idleCntEnablePtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(dev, port, &idleCntEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with maxIdleCnt [0\1000\32768].
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.1.3. Call with out of range maxIdleCnt [32769].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_U32      maxIdleCnt    = 0;
    GT_U32      maxIdleCntGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            maxIdleCnt = 0;

            st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCnt);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCnt);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCntGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCntGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(maxIdleCnt, maxIdleCntGet,
                                             "get another maxIdleCnt than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCntGet);


            /* 1.1.1. */
            maxIdleCnt = 1000;

            st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCnt);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCnt);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCntGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCntGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(maxIdleCnt, maxIdleCntGet,
                                             "get another maxIdleCnt than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCntGet);

            /* 1.1.1. */
            maxIdleCnt = 32768;

            st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCnt);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCnt);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCntGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCntGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(maxIdleCnt, maxIdleCntGet,
                                             "get another maxIdleCnt than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCntGet);

            /* 1.1.3. */
            maxIdleCnt = 131072;

            st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, maxIdleCnt);

            maxIdleCnt = 0;
        }

        maxIdleCnt = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    maxIdleCnt = 0;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsMaxIdleCntSet(dev, port, maxIdleCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL maxIdleCntPtr.
    Expected: GT_OK.
    1.1.2. Call with maxIdleCntPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_U32      maxIdleCnt = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCnt);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maxIdleCntPtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsMaxIdleCntGet(dev, port, &maxIdleCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkPcsRxStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL laneSyncOkArr.
    Expected: GT_OK.
    1.1.2. Call with laneSyncOkArr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     laneSyncOkArr[CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS] = {GT_FALSE};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkPcsRxStatusGet(dev, port, laneSyncOkArr);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* 1.1.2. */
            st = cpssDxChFabricHGLinkPcsRxStatusGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, laneSyncOkArr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkPcsRxStatusGet(dev, port, laneSyncOkArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkPcsRxStatusGet(dev, port, laneSyncOkArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkPcsRxStatusGet(dev, port, laneSyncOkArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsRxStatusGet(dev, port, laneSyncOkArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsRxStatusGet(dev, port, laneSyncOkArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkRxErrorCntrGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL rxErrorsCntrsPtr.
    Expected: GT_OK.
    1.1.2. Call with rxErrorsCntrsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC rxErrorsCntrs = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkRxErrorCntrGet(dev, port, &rxErrorsCntrs);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);



            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxErrorCntrGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rxErrorsCntrsPtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkRxErrorCntrGet(dev, port, &rxErrorsCntrs);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkRxErrorCntrGet(dev, port, &rxErrorsCntrs);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkRxErrorCntrGet(dev, port, &rxErrorsCntrs);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkRxErrorCntrGet(dev, port, &rxErrorsCntrs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkRxErrorCntrGet(dev, port, &rxErrorsCntrs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkConfigEccTypeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with eccType [CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E \
                              CPSS_DXCH_FABRIC_HGL_ECC_CRC8_E].
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.1.3. Call with out of range eccType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT   eccType    = CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E;
    CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT   eccTypeGet = CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            eccType = CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E;

            st = cpssDxChFabricHGLinkConfigEccTypeSet(dev, port, eccType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccType);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccType);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccTypeGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccTypeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(eccType, eccTypeGet,
                                             "get another eccType than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccTypeGet);

            /* 1.1.1. */
            eccType = CPSS_DXCH_FABRIC_HGL_ECC_CRC8_E;

            st = cpssDxChFabricHGLinkConfigEccTypeSet(dev, port, eccType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccType);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccType);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccTypeGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccTypeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(eccType, eccTypeGet,
                    "get another eccType than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, eccTypeGet);

            /* 1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssDxChFabricHGLinkConfigEccTypeSet
                                (dev, port, eccType),
                                eccType);
        }

        eccType = CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkConfigEccTypeSet(dev, port, eccType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkConfigEccTypeSet(dev, port, eccType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkConfigEccTypeSet(dev, port, eccType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    eccType = CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkConfigEccTypeSet(dev, port, eccType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkConfigEccTypeSet(dev, port, eccType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkConfigEccTypeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL eccTypePtr.
    Expected: GT_OK.
    1.1.2. Call with eccTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT   eccType = CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, eccTypePtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkConfigEccTypeGet(dev, port, &eccType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkRxCrcCheckEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with enable [GT_FALSE \ GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     enable    = GT_TRUE;
    GT_BOOL     enableGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            enable = GT_FALSE;

            st = cpssDxChFabricHGLinkRxCrcCheckEnableSet(dev, port, enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxCrcCheckEnableGet(dev, port, &enableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                             "get another enable than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);

            /* 1.1.1. */
            enable = GT_TRUE;

            st = cpssDxChFabricHGLinkRxCrcCheckEnableSet(dev, port, enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxCrcCheckEnableGet(dev, port, &enableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
         }

        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkRxCrcCheckEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkRxCrcCheckEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkRxCrcCheckEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkRxCrcCheckEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkRxCrcCheckEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkRxCrcCheckEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkRxCrcCheckEnableGet(dev, port, &enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxCrcCheckEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkRxCrcCheckEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkRxCrcCheckEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkRxCrcCheckEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkPcsLoopbackEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkLbiEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with enable [GT_FALSE \ GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     enable    = GT_TRUE;
    GT_BOOL     enableGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            enable = GT_FALSE;

            st = cpssDxChFabricHGLinkLbiEnableSet(dev, port, enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                             "get another enable than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);

            /* 1.1.1. */
            enable = GT_TRUE;

            st = cpssDxChFabricHGLinkLbiEnableSet(dev, port, enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enableGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                             "get another enable than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);

        }

        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkLbiEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkLbiEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkLbiEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkLbiEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkLbiEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkLbiEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL     enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enable);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkLbiEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkCellsCntrTypeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with cellDirection [CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E \
                                    CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E],
                     cellType [CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E \
                               CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_MC_E \
                               CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_BC_E \
                               CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_OTHER_E]
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.1.3. Call with out of range cellDirection.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range cellType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;

    CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT       cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;
    CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT cellType    = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E;
    CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT cellTypeGet = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;
            cellType      = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E;

            st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
            if (expectedOk==GT_TRUE)
                 UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellTypeGet);
            if (expectedOk==GT_TRUE)
            {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(cellType, cellTypeGet,
                         "get another cellType than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);


            /* 1.1.1. */
            cellType = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_MC_E;

            st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellTypeGet);

            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(cellType, cellTypeGet,
                    "get another cellType than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);

            /* 1.1.1. */
            cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E;
            cellType      = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_BC_E;

            st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellTypeGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(cellType, cellTypeGet,
                                             "get another cellType than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);

            /* 1.1.1. */
            cellType = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_OTHER_E;

            st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellType);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellTypeGet);
            if (expectedOk==GT_TRUE)
            {
                     UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);
                     UTF_VERIFY_EQUAL2_STRING_MAC(cellType, cellTypeGet,
                                                  "get another cellType than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, cellDirection, cellTypeGet);

            /* 1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssDxChFabricHGLinkCellsCntrTypeSet
                                (dev, port, cellDirection, cellType),
                                cellDirection);

            /* 1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssDxChFabricHGLinkCellsCntrTypeSet
                                (dev, port, cellDirection, cellType),
                                cellType);
        }

        cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;
        cellType      = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;
    cellType      = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkCellsCntrTypeSet(dev, port, cellDirection, cellType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkCellsCntrTypeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with cellDirection [CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E \
                                    CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E],
                     non-NULL cellTypePtr.
    Expected: GT_OK.
    1.1.2. Call with out of range cellDirection.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with cellTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;

    CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT       cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;
    CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT cellType    = CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;

            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);

            /* 1.1.1. */
            cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E;

            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellType);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);

            /* 1.1.2. */
            UTF_ENUMS_CHECK_MAC(cpssDxChFabricHGLinkCellsCntrTypeGet
                                (dev, port, cellDirection, &cellType),
                                cellDirection);

            /* 1.1.3. */
            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cellTypePtr = NULL", dev);;
        }

        cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkCellsCntrTypeGet(dev, port, cellDirection, &cellType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkCellsCntrGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with cellDirection [CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E \
                                    CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E],
                     non-NULL cellCntrPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range cellDirection.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with cellCntrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;

    CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;
    GT_U32                             cellCntr      = 0;
    GT_BOOL     expectedOk;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;

            st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, &cellCntr);
            if (expectedOk==GT_TRUE)
                 UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);
            else
                 UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);


            /* 1.1.1. */
            cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E;

            st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, &cellCntr);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cellDirection);

            /* 1.1.2. */
            UTF_ENUMS_CHECK_MAC(cpssDxChFabricHGLinkCellsCntrGet
                                (dev, port, cellDirection, &cellCntr),
                                cellDirection);

            /* 1.1.3. */
            st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cellCntrPtr = NULL", dev);
        }

        cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, &cellCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, &cellCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, &cellCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    cellDirection = CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, &cellCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkCellsCntrGet(dev, port, cellDirection, &cellCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkFcMacSaSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with macPtr [01:00:00:00:00:01 \
                             AB:CD:EF:00:00:01].
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.1.3. Call with macPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL              isEqual  = GT_FALSE;
    GT_ETHERADDR         macSa    = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    GT_ETHERADDR         macSaGet = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            cpssOsBzero((GT_VOID*) &macSa,    sizeof(macSa));
            cpssOsBzero((GT_VOID*) &macSaGet, sizeof(macSaGet));

            /* 1.1.1. */
            macSa.arEther[0] = 0x1; macSa.arEther[5] = 0x1;

            st = cpssDxChFabricHGLinkFcMacSaSet(dev, port, &macSa);
            if (expectedOk)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSaGet);
            if (expectedOk)
            {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&macSa, (GT_VOID*)&macSaGet, sizeof(macSa))) ?
                               GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                            "get another mac than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* 1.1.1. */
            macSa.arEther[0] = 0xAB; macSa.arEther[1] = 0xCD; macSa.arEther[2] = 0xEF;

            st = cpssDxChFabricHGLinkFcMacSaSet(dev, port, &macSa);
            if (expectedOk)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSaGet);
            if (expectedOk)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&macSa, (GT_VOID*)&macSaGet, sizeof(macSa))) ?
                       GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                    "get another mac than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3. */
            st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macPtr = NULL", dev);;
        }

        cpssOsBzero((GT_VOID*) &macSa, sizeof(macSa));

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkFcMacSaSet(dev, port, &macSa);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkFcMacSaSet(dev, port, &macSa);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkFcMacSaSet(dev, port, &macSa);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    cpssOsBzero((GT_VOID*) &macSa, sizeof(macSa));

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkFcMacSaSet(dev, port, &macSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkFcMacSaSet(dev, port, &macSa);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkFcMacSaGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL macPtr.
    Expected: GT_OK.
    1.1.2. Call with macPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_ETHERADDR         macSa = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSa);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macPtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSa);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSa);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSa);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkFcMacSaGet(dev, port, &macSa);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkRxDsaParamsSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with dsaParams{vid[100\1000\4095], srcId[0\10\31]}.
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.1.3. Call with out of range dsaParamsPtr->vid [4096].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range dsaParamsPtr->srcId [32].
    Expected: GT_BAD_PARAM.
    1.1.5. Call with dsaParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    CPSS_DXCH_FABRIC_HGL_RX_DSA_STC dsaParams    = {0};
    CPSS_DXCH_FABRIC_HGL_RX_DSA_STC dsaParamsGet = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            dsaParams.vid   = 100;
            dsaParams.srcId = 0;

            st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
            if (expectedOk)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParamsGet);
            if (expectedOk)
            {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(dsaParams.vid, dsaParamsGet.vid,
                                                 "get another dsaParamsPtr->vid than was set: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(dsaParams.srcId, dsaParamsGet.srcId,
                                                 "get another dsaParamsPtr->srcId than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* 1.1.1. */
            dsaParams.vid   = 1000;
            dsaParams.srcId = 10;

            st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
            if (expectedOk)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParamsGet);
            if (expectedOk)
            {
                 UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                 UTF_VERIFY_EQUAL2_STRING_MAC(dsaParams.vid, dsaParamsGet.vid,
                                              "get another dsaParamsPtr->vid than was set: %d, %d", dev, port);
                 UTF_VERIFY_EQUAL2_STRING_MAC(dsaParams.srcId, dsaParamsGet.srcId,
                                              "get another dsaParamsPtr->srcId than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. */
            dsaParams.vid   = 4095;
            dsaParams.srcId = 31;

            st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
            if (expectedOk)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParamsGet);
            if (expectedOk)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(dsaParams.vid, dsaParamsGet.vid,
                                             "get another dsaParamsPtr->vid than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(dsaParams.srcId, dsaParamsGet.srcId,
                                             "get another dsaParamsPtr->srcId than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3. */
            dsaParams.vid = 4096;

            st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, dsaParams.vid);

            dsaParams.vid = 100;

            /* 1.1.4. */
            dsaParams.srcId = 0x20;

            st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, dsaParams.srcId);

            dsaParams.srcId = 0;

            /* 1.1.5. */
            st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dsaParamsPtr = NULL", dev);
        }

        dsaParams.vid   = 100;
        dsaParams.srcId = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    dsaParams.vid   = 100;
    dsaParams.srcId = 0;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkRxDsaParamsSet(dev, port, &dsaParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkRxDsaParamsGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL dsaParamsPtr.
    Expected: GT_OK.
    1.1.2. Call with dsaParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    CPSS_DXCH_FABRIC_HGL_RX_DSA_STC dsaParams = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dsaParamsPtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkRxDsaParamsGet(dev, port, &dsaParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkFcRxE2eParamsSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with e2eParams{etherType[0\0xAAAA\0xFFFF],
                               prioVlanId[100\1000\0x4095],
                               cpId[0\100\0xFFFF],
                               version[0\120\0xFFFF],
                               macLowByte[0\150\0xFF]}.
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.1.3. Call with out of range e2eParamsPtr->prioVlanId [4096].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with e2eParamsPtr->cpId [0xFFFFFFFF] (no any ranges).
    Expected: GT_OK.
    1.1.5. Call with e2eParamsPtr->version [0xFFFFFFFF] (no any ranges).
    Expected: GT_OK.
    1.1.6. Call with e2eParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL                                   isEqual      = GT_FALSE;
    CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC e2eParams    = {0};
    CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC e2eParamsGet = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            e2eParams.etherType  = 0;
            e2eParams.prioVlanId = 100;
            e2eParams.cpId       = 0;
            e2eParams.version    = 0;
            e2eParams.macLowByte = 0;

            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParamsGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&e2eParams, (GT_VOID*)&e2eParamsGet, sizeof(e2eParams))) ?
                       GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                    "get another e2eParams than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. */
            e2eParams.etherType  = 0xAAAA;
            e2eParams.prioVlanId = 1000;
            e2eParams.cpId       = 100;
            e2eParams.version    = 5;
            e2eParams.macLowByte = 150;

            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParamsGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&e2eParams, (GT_VOID*)&e2eParamsGet, sizeof(e2eParams))) ?
                       GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                    "get another e2eParams than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. */
            e2eParams.etherType  = 0xFFFF;
            e2eParams.prioVlanId = 4095;
            e2eParams.cpId       = 0xFFFF;
            e2eParams.version    = 0xF;
            e2eParams.macLowByte = 0xFF;

            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParamsGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&e2eParams, (GT_VOID*)&e2eParamsGet, sizeof(e2eParams))) ?
                       GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                    "get another e2eParams than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3. */
            e2eParams.prioVlanId = 4096;

            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, e2eParams.prioVlanId);

            e2eParams.prioVlanId = 100;

            /* 1.1.4. */
            e2eParams.cpId = 0xFFFFFFFF;

            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, e2eParams.cpId);
            else
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, e2eParams.cpId);

            e2eParams.cpId = 0;

            /* 1.1.5. */
            e2eParams.version = 0xFFFFFFFF;

            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, e2eParams.version);

            e2eParams.version = 0;

            /* 1.1.6. */
            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, e2eParamsPtr = NULL", dev);
        }

        e2eParams.etherType  = 0;
        e2eParams.prioVlanId = 100;
        e2eParams.cpId       = 0;
        e2eParams.version    = 0;
        e2eParams.macLowByte = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    e2eParams.etherType  = 0;
    e2eParams.prioVlanId = 100;
    e2eParams.cpId       = 0;
    e2eParams.version    = 0;
    e2eParams.macLowByte = 0;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkFcRxE2eParamsSet(dev, port, &e2eParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkFcRxE2eParamsGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL e2eParamsPtr.
    Expected: GT_OK.
    1.1.2. Call with e2eParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC e2eParams = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, e2eParamsPtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkFcRxE2eParamsGet(dev, port, &e2eParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkRxLinkLayerFcParamsSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with llParams{etherType[0\0xAAAA\0xFFFF],
                              pfcOpcode[0\200\0xFFFF],
                              up[0\5\7],
                              qosProfile[0\100\0xFFFF],
                              trgDev[0\dev\maxDevNum-1],
                              trgPort[0\(maxPortNum-1)2\maxPortNum-1]}.
    Expected: GT_OK.
    1.1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.1.3. Call with out of range llParamsPtr->up[8].
    Expected: NOT GT_OK.
    1.1.4. Call with out of range llParamsPtr->trgDev[maxDevNum].
    Expected: GT_BAD_PARAM.
    1.1.5. Call with out of range llParamsPtr->trgPort[maxPortNum].
    Expected: GT_BAD_PARAM.
    1.1.6. Call with llParamsPtr->pfcOpcode [0xFFFFFFFF] (no any ranges).
    Expected: GT_OK.
    1.1.7. Call with llParamsPtr->qosProfile [0xFFFFFFFF] (no any ranges).
    Expected: GT_OK.
    1.1.8. Call with llParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    GT_BOOL                                          isEqual     = GT_FALSE;
    CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC llParams    = {0};
    CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC llParamsGet = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            llParams.etherType  = 0;
            llParams.pfcOpcode  = 0;
            llParams.up         = 0;
            llParams.qosProfile = 0;
            llParams.trgDev     = 0;
            llParams.trgPort    = 0;

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParamsGet);
            if (expectedOk==GT_TRUE)
            {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&llParams, (GT_VOID*)&llParamsGet, sizeof(llParams))) ?
                               GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                            "get another llParams than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* 1.1.1. */
            llParams.etherType  = 0xAAAA;
            llParams.pfcOpcode  = 200;
            llParams.up         = 5;
            llParams.qosProfile = 100;
            llParams.trgDev     = dev;
            llParams.trgPort    = (port & 0x3F);

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParamsGet);
            if (expectedOk==GT_TRUE)
            {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&llParams, (GT_VOID*)&llParamsGet, sizeof(llParams))) ?
                                           GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                                                 "get another llParams than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* 1.1.1. */
            llParams.etherType  = 0xAAAA;
            llParams.pfcOpcode  = 200;
            llParams.up         = 5;
            llParams.qosProfile = 100;
            llParams.trgDev     = BIT_5 - 1;
            llParams.trgPort    = BIT_6 - 1;

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParamsGet);
            if (expectedOk==GT_TRUE)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&llParams, (GT_VOID*)&llParamsGet, sizeof(llParams))) ?
                       GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                                             "get another llParams than was set: %d, %d", dev, port);
            }
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3. */
            llParams.up = 8;

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, llParams.up);

            llParams.up = 0;

            /* 1.1.4. */
            llParams.trgDev = PRV_CPSS_MAX_PP_DEVICES_CNS;

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, llParams.trgDev);

            llParams.trgDev = 0;

            /* 1.1.5. */
            llParams.trgPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, llParams.trgPort);

            llParams.trgPort = 0;

            /* 1.1.6. */
            llParams.pfcOpcode = 0xFFFFFFFF;

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, llParams.pfcOpcode);

            llParams.pfcOpcode = 0;

            /* 1.1.7. */
            llParams.qosProfile = 0xFFFFFFFF;

            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, llParams.qosProfile);

            llParams.qosProfile = 0;

            /* 1.1.8. */
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, llParamsPtr = NULL", dev);
        }

        llParams.etherType  = 0;
        llParams.pfcOpcode  = 0;
        llParams.up         = 0;
        llParams.qosProfile = 0;
        llParams.trgDev     = 0;
        llParams.trgPort    = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    llParams.etherType  = 0;
    llParams.pfcOpcode  = 0;
    llParams.up         = 0;
    llParams.qosProfile = 0;
    llParams.trgDev     = 0;
    llParams.trgPort    = 0;

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(dev, port, &llParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChFabricHGLinkRxLinkLayerFcParamsGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with non-NULL llParamsPtr.
    Expected: GT_OK.
    1.1.2. Call with llParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PHYSICAL_PORT_NUM port = FABRIC_VALID_PHY_PORT_CNS;
    GT_BOOL     expectedOk;
    CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC llParams = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            FABRIC_CALCULATE_EXPECTED_RESULT_MAC(port);
            /* 1.1.1. */
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParams);
            if (expectedOk==GT_TRUE)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);



            /* 1.1.2. */
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, llParamsPtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    port = FABRIC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(dev, port, &llParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChFabricHGLink suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChFabricHGLink)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkPcsLoopbackEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkPcsLoopbackEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkPcsMaxIdleCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkPcsRxStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkRxErrorCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkConfigEccTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkConfigEccTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkRxCrcCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkRxCrcCheckEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkLbiEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkLbiEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkCellsCntrTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkCellsCntrTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkCellsCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkFcMacSaSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkFcMacSaGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkRxDsaParamsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkRxDsaParamsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkFcRxE2eParamsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkFcRxE2eParamsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkRxLinkLayerFcParamsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFabricHGLinkRxLinkLayerFcParamsGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChFabricHGLink)
