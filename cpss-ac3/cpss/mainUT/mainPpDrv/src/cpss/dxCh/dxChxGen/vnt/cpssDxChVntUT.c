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
* @file cpssDxChVntUT.c
*
* @brief Unit tests for cpssDxChBrgVlan, that provides
* CPSS DXCH Virtual Network Tester (VNT) Technology facility API.
* VNT features:
* - Operation, Administration, and Maintenance (OAM).
* - Connectivity Fault Management (CFM).
*
* @version   17
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for physical port id */
#define VNT_VALID_PHY_PORT_CNS 0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntOamPortLoopBackModeEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    port,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntOamPortLoopBackModeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK
    1.1.2. Call function cpssDxChVntOamPortLoopBackModeEnableGet with not NULL enablePtr
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = VNT_VALID_PHY_PORT_CNS;

    GT_BOOL enable    = GT_FALSE;
    GT_BOOL enableGet = GT_FALSE;


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
                1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
                Expected: GT_OK
            */

            /* Call with enable [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChVntOamPortLoopBackModeEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call function cpssDxChVntOamPortLoopBackModeEnableGet with not NULL enablePtr
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChVntOamPortLoopBackModeEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);

            /* Call with enable [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChVntOamPortLoopBackModeEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call function cpssDxChVntOamPortLoopBackModeEnableGet with not NULL enablePtr
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChVntOamPortLoopBackModeEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChVntOamPortLoopBackModeEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChVntOamPortLoopBackModeEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChVntOamPortLoopBackModeEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = VNT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntOamPortLoopBackModeEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntOamPortLoopBackModeEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntOamPortLoopBackModeEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    port,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntOamPortLoopBackModeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call function with not-NULL enablePtr
    Expected: GT_OK
    1.1.2. Call function with enablePtr [NULL]
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = VNT_VALID_PHY_PORT_CNS;

    GT_BOOL enable = GT_FALSE;


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
                1.1.1. Call function with not-NULL enablePtr
                Expected: GT_OK
            */
            st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call function with enablePtr [NULL]
                Expected: GT_BAD_PTR
            */
            st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = VNT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntOamPortLoopBackModeEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntOamPortUnidirectionalEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    port,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntOamPortUnidirectionalEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK
    1.1.2. Call function cpssDxChVntOamPortUnidirectionalEnableGet with not NULL enablePtr
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = VNT_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    GT_BOOL enable    = GT_FALSE;
    GT_BOOL enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
                Expected: GT_OK
            */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /* Call with enable [GT_FALSE] */
            enable = GT_FALSE;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                continue;
            }

            st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            if(portType != PRV_CPSS_PORT_CG_E)
            {
                /*
                    1.1.2. Call function cpssDxChVntOamPortUnidirectionalEnableGet with not NULL enablePtr
                    Expected: GT_OK and enable the same as was set.
                */
                st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChVntOamPortUnidirectionalEnableGet: %d, %d", dev, port);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);

                /* Call with enable [GT_TRUE] */
                enable = GT_TRUE;

                st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                /*
                    1.1.2. Call function cpssDxChVntOamPortUnidirectionalEnableGet with not NULL enablePtr
                    Expected: GT_OK and enable the same as was set.
                */
                st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChVntOamPortUnidirectionalEnableGet: %d, %d", dev, port);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    enable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = VNT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntOamPortUnidirectionalEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntOamPortUnidirectionalEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    port,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntOamPortUnidirectionalEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call function with not-NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = VNT_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    GT_BOOL enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

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
                st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /*
                1.1.1. Call function with not-NULL enablePtr
                Expected: GT_OK
            */
            st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call function with enablePtr [NULL]
                Expected: GT_BAD_PTR
            */
            st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = VNT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntOamPortUnidirectionalEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntCfmEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntCfmEtherTypeSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call function with etherType [0 / 0xFFFF].
    Expected: GT_OK.
    1.2. Call function cpssDxChVntCfmEtherTypeGet with no NULL etherTypePtr.
    Expected: GT_OK and etherType the same as was set.
    1.3. Call function with out of range etherType [0x10000].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      etherType    = 0;
    GT_U32      etherTypeGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with etherType [0 / 0xFFFF].
            Expected: GT_OK.
        */

        /* Call with etherType = 0 */
        etherType = 0;

        st = cpssDxChVntCfmEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherType);

        /*
            1.2. Call function cpssDxChVntCfmEtherTypeGet with no NULL etherTypePtr.
            Expected: GT_OK and etherType the same as was set.
        */
        st = cpssDxChVntCfmEtherTypeGet(dev, &etherTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChVntCfmEtherTypeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
                   "get another etherType than was set: %d", dev);

        /* Call with etherType = 0xFFFF */
        etherType = 0xFFFF;

        st = cpssDxChVntCfmEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherType);

        /*
            1.2. Call function cpssDxChVntCfmEtherTypeGet with no NULL etherTypePtr.
            Expected: GT_OK and etherType the same as was set.
        */
        st = cpssDxChVntCfmEtherTypeGet(dev, &etherTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChVntCfmEtherTypeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
                   "get another etherType than was set: %d", dev);

        /*
            1.3. Call function with out of range etherType [0x10000].
            Expected: NOT GT_OK.
        */
        etherType = 0x10000;

        st = cpssDxChVntCfmEtherTypeSet(dev, etherType);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherType);
    }

    etherType = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntCfmEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntCfmEtherTypeSet(dev, etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntCfmEtherTypeGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *etherTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntCfmEtherTypeGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call function with not NULL etherTypePtr
    Expected: GT_OK.
    1.2. Call function with etherTypePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      etherType = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL etherTypePtr
            Expected: GT_OK.
        */
        st = cpssDxChVntCfmEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with etherTypePtr [NULL]
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChVntCfmEtherTypeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, etherTypePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntCfmEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntCfmEtherTypeGet(dev, &etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntCfmLbrOpcodeSet
(
    IN GT_U8    devNum,
    IN GT_U32   opcode
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntCfmLbrOpcodeSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call function with opcode [0 / 0xFF].
    Expected: GT_OK.
    1.2. Call function cpssDxChVntCfmLbrOpcodeGet with no NULL opcodePtr.
    Expected: GT_OK and opcode the same as was set.
    1.3. Call function with out of range opcode [0x100].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      opcode    = 0;
    GT_U32      opcodeGet = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with opcode [0 / 0xFF].
            Expected: GT_OK.
        */

        /* Call with opcode = 0 */
        opcode = 0;

        st = cpssDxChVntCfmLbrOpcodeSet(dev, opcode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, opcode);

        /*
            1.2. Call function cpssDxChVntCfmLbrOpcodeGet with no NULL opcodePtr.
            Expected: GT_OK and opcode the same as was set.
        */
        st = cpssDxChVntCfmLbrOpcodeGet(dev, &opcodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChVntCfmLbrOpcodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(opcode, opcodeGet,
                   "get another opcode than was set: %d", dev);

        /* Call with opcode = 0xFF */
        opcode = 0xFF;

        st = cpssDxChVntCfmLbrOpcodeSet(dev, opcode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, opcode);

        /*
            1.2. Call function cpssDxChVntCfmLbrOpcodeGet with no NULL opcodePtr.
            Expected: GT_OK and opcode the same as was set.
        */
        st = cpssDxChVntCfmLbrOpcodeGet(dev, &opcodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChVntCfmLbrOpcodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(opcode, opcodeGet,
                   "get another opcode than was set: %d", dev);

        /*
            1.3. Call function with out of range opcode [0x100].
            Expected: NOT GT_OK.
        */
        opcode = 0x100;

        st = cpssDxChVntCfmLbrOpcodeSet(dev, opcode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, opcode);
    }

    opcode = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntCfmLbrOpcodeSet(dev, opcode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntCfmLbrOpcodeSet(dev, opcode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntCfmLbrOpcodeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *opcodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntCfmLbrOpcodeGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call function with not NULL opcodePtr
    Expected: GT_OK.
    1.2. Call function with opcodePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      opcode = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL opcodePtr
            Expected: GT_OK.
        */
        st = cpssDxChVntCfmLbrOpcodeGet(dev, &opcode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with opcodePtr [NULL]
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChVntCfmLbrOpcodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, opcodePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntCfmLbrOpcodeGet(dev, &opcode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntCfmLbrOpcodeGet(dev, &opcode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntLastReadTimeStampGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *timeStampValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntLastReadTimeStampGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with not NULL timeStampValuePtr
    Expected: GT_OK.
    1.2. Call function with timeStampValuePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      timeStampValue = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL timeStampValuePtr.
            Expected: GT_OK.
        */
        st = cpssDxChVntLastReadTimeStampGet(dev, &timeStampValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with timeStampValuePtr [NULL]
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChVntLastReadTimeStampGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, timeStampValuePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntLastReadTimeStampGet(dev, &timeStampValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntLastReadTimeStampGet(dev, &timeStampValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntOamPortPduTrapEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    port,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntOamPortPduTrapEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK
    1.1.2. Call function cpssDxChVntOamPortPduTrapEnableGet with not NULL enablePtr
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = VNT_VALID_PHY_PORT_CNS;

    GT_BOOL enable    = GT_FALSE;
    GT_BOOL enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
                Expected: GT_OK
            */

            /* Call with enable [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChVntOamPortPduTrapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call function cpssDxChVntOamPortPduTrapEnableGet with not NULL enablePtr
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChVntOamPortPduTrapEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);

            /* Call with enable [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChVntOamPortPduTrapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call function cpssDxChVntOamPortPduTrapEnableGet with not NULL enablePtr
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChVntOamPortPduTrapEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChVntOamPortPduTrapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(dev);

        st = cpssDxChVntOamPortPduTrapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChVntOamPortPduTrapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = VNT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntOamPortPduTrapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntOamPortPduTrapEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntOamPortPduTrapEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    port,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntOamPortPduTrapEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call function with not-NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = VNT_VALID_PHY_PORT_CNS;

    GT_BOOL enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with not-NULL enablePtr
                Expected: GT_OK
            */
            st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call function with enablePtr [NULL]
                Expected: GT_BAD_PTR
            */
            st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(dev);

        st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = VNT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntOamPortPduTrapEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntPortGroupLastReadTimeStampGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *timeStampValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntPortGroupLastReadTimeStampGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call with not-NULL timeStampValuePtr.
    Expected: GT_OK.
    1.1.2. Call with timeStampValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st  = GT_OK;
    GT_U8                   dev = 0;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId   = 0;

    GT_U32  timeStampValue = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with not-NULL timeStampValuePtr.
                Expected: GT_OK.
            */
            st = cpssDxChVntPortGroupLastReadTimeStampGet(dev, portGroupsBmp, &timeStampValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with timeStampValuePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChVntPortGroupLastReadTimeStampGet(dev, portGroupsBmp, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, timeStampValuePtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChVntPortGroupLastReadTimeStampGet(dev, portGroupsBmp, &timeStampValue);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChVntPortGroupLastReadTimeStampGet(dev, portGroupsBmp, &timeStampValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVntPortGroupLastReadTimeStampGet(dev, portGroupsBmp, &timeStampValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChVntPortGroupLastReadTimeStampGet(dev, portGroupsBmp, &timeStampValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntCfmReplyModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntCfmReplyModeSet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1 Call with
        mode [CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E |
              CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E]
    Expected: GT_OK.
    1.2. Call cpssDxChVntCfmReplyModeGet().
    Expected: GT_OK and same mode as was set in 1.1.1.
    1.3 Call with mode out of range
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st  = GT_OK;
    GT_U8                               devNum  = 0;
    CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT    mode;
    CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT    modeGet;
    GT_BOOL                             notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with
                mode [CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E]
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E;
        st = cpssDxChVntCfmReplyModeSet(devNum, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssDxChVntCfmReplyModeGet().
            Expected: GT_OK and same mode as was set in 1.1.1.
        */
        st = cpssDxChVntCfmReplyModeGet(devNum, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_PARAM_MAC(mode, modeGet, devNum);

        /*
            1.1 Call with
                mode [CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E]
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E;
        st = cpssDxChVntCfmReplyModeSet(devNum, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssDxChVntCfmReplyModeGet().
            Expected: GT_OK and same mode as was set in 1.1.1.
        */
        st = cpssDxChVntCfmReplyModeGet(devNum, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_PARAM_MAC(mode, modeGet, devNum);

        /*
            1.3 Call with mode out of range
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChVntCfmReplyModeSet
                            (devNum, mode), mode);
    }

    /*set valid parameters*/
    mode = CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E;

    /* prepare iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChVntCfmReplyModeSet(devNum, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChVntCfmReplyModeSet(devNum, mode);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVntCfmReplyModeGet
(
    IN GT_U8                            devNum,
    OUT CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVntCfmReplyModeGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1 Call api with wrong enable modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st  = GT_OK;
    GT_U8                               devNum  = 0;
    CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT    mode;
    GT_BOOL                             notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call api with wrong enable modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChVntCfmReplyModeGet(
                devNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
    }

    /* prepare iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChVntCfmReplyModeGet(
                devNum, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChVntCfmReplyModeGet(
                devNum, &mode);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChVnt suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChVnt)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntOamPortLoopBackModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntOamPortLoopBackModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntOamPortUnidirectionalEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntOamPortUnidirectionalEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntCfmEtherTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntCfmEtherTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntCfmLbrOpcodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntCfmLbrOpcodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntLastReadTimeStampGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntOamPortPduTrapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntOamPortPduTrapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntPortGroupLastReadTimeStampGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntCfmReplyModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVntCfmReplyModeGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChVnt)


