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
* @file cpssDxChPortEeeUT.c
*
* @brief CPSS DxCh Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
*
* @note 1. The device support relevant when the PHY works in EEE slave mode.
* When the PHY works in EEE master mode, the device is not involved in
* EEE processing.
* 2. The feature not relevant when port is not 'tri-speed'
* 3. LPI is short for 'Low Power Idle'
*
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

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortEee.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* EEE : prepare iterator for go over all active devices */
#define UT_EEE_RESET_DEV(_devPtr)  \
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC((_devPtr),     \
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | \
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |      \
        UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E)

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEeeLpiRequestEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEeeLpiRequestEnableSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet;

    /* there is no EEE in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortEeeLpiRequestEnableSet(dev,port,enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,enable);
                continue;
            }

            for(enable = GT_FALSE ; enable <= GT_TRUE ; enable ++)
            {
                st = cpssDxChPortEeeLpiRequestEnableSet(dev,port,enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,enable);

                st = cpssDxChPortEeeLpiRequestEnableGet(dev,port,&enableGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                /* compare results */
                UTF_VERIFY_EQUAL2_PARAM_MAC(enable, enableGet, dev,port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        enable = GT_TRUE;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEeeLpiRequestEnableSet(dev,port,enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,enable);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEeeLpiRequestEnableSet(dev, port,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEeeLpiRequestEnableSet(dev, port,enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEeeLpiRequestEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEeeLpiRequestEnableGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     enableGet;

    /* there is no EEE in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortEeeLpiRequestEnableGet(dev,port,&enableGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                continue;
            }

            st = cpssDxChPortEeeLpiRequestEnableGet(dev,port,&enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiRequestEnableGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEeeLpiRequestEnableGet(dev,port,&enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiRequestEnableGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEeeLpiRequestEnableGet(dev, port,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEeeLpiRequestEnableGet(dev, port,&enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEeeLpiManualModeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  mode
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEeeLpiManualModeSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  mode = CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E;
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  modeGet;

    /* there is no EEE in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortEeeLpiManualModeSet(dev,port,mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,mode);
                continue;
            }

            for(mode = CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E ;
                mode <= CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_FORCED_E ;
                mode ++)
            {
                st = cpssDxChPortEeeLpiManualModeSet(dev,port,mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,mode);

                st = cpssDxChPortEeeLpiManualModeGet(dev,port,&modeGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                /* compare results */
                UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeGet, dev,port);
            }
        }

        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        /* init mode to avoid diab compiler :
           "warning: 'mode' might be used uninitialized in this function"*/
        mode = CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E;
        UTF_ENUMS_CHECK_MAC(cpssDxChPortEeeLpiManualModeSet
                            (dev,port,mode),
                            mode);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        mode = CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEeeLpiManualModeSet(dev,port,mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,mode);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    mode = CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEeeLpiManualModeSet(dev, port,mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEeeLpiManualModeSet(dev, port,mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEeeLpiManualModeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  *modePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEeeLpiManualModeGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT     modeGet;

    /* there is no EEE in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortEeeLpiManualModeGet(dev,port,&modeGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                continue;
            }

            st = cpssDxChPortEeeLpiManualModeGet(dev,port,&modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiManualModeGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEeeLpiManualModeGet(dev,port,&modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiManualModeGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEeeLpiManualModeGet(dev, port,&modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEeeLpiManualModeGet(dev, port,&modeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEeeLpiTimeLimitsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  twLimit,
    IN  GT_U32                  liLimit,
    IN  GT_U32                  tsLimit
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEeeLpiTimeLimitsSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32                  twLimit;
    GT_U32                  liLimit;
    GT_U32                  tsLimit;
    GT_U32                  twLimitGet;
    GT_U32                  liLimitGet;
    GT_U32                  tsLimitGet;
    struct{
        GT_U32                  twLimit;
        GT_U32                  liLimit;
        GT_U32                  tsLimit;
    }ok_valuesArr[] = {
        {BIT_12 - 1, 254     , BIT_8 - 1},/* max values */
        {BIT_12 / 2, 254 / 2 , BIT_8 / 2},/* mid values */
        {0         , 0       , 0        },/* zero values */

        /* last entry */
        {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    };
    struct{
        GT_U32                  twLimit;
        GT_U32                  liLimit;
        GT_U32                  tsLimit;
    }bad_valuesArr[] = {
        {BIT_12, 0      , 0     },/* twLimit - out of range */
        {0     , 255    , 0     },/* liLimit - out of range */
        {0     , 0      , BIT_8 },/* tsLimit - out of range */

        /* last entry */
        {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    };
    GT_U32  ii;

    /* there is no EEE in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                twLimit = ok_valuesArr[0].twLimit;
                liLimit = ok_valuesArr[0].liLimit;
                tsLimit = ok_valuesArr[0].tsLimit;
                st = cpssDxChPortEeeLpiTimeLimitsSet(dev,port,twLimit,liLimit,tsLimit);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev,port,twLimit,liLimit,tsLimit);
                continue;
            }

            /* check 'GT_OK' values */
            for(ii = 0 ; ok_valuesArr[ii].twLimit != 0xFFFFFFFF ; ii++)
            {
                twLimit = ok_valuesArr[ii].twLimit;
                liLimit = ok_valuesArr[ii].liLimit;
                tsLimit = ok_valuesArr[ii].tsLimit;

                st = cpssDxChPortEeeLpiTimeLimitsSet(dev,port,twLimit,liLimit,tsLimit);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev,port,twLimit,liLimit,tsLimit);

                st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,&liLimitGet,&tsLimitGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                /* compare results */
                UTF_VERIFY_EQUAL2_PARAM_MAC(twLimit, twLimitGet, dev,port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(liLimit, liLimitGet, dev,port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(tsLimit, tsLimitGet, dev,port);
            }

            /* check 'GT_OUT_OF_RANGE' values */
            for(ii = 0 ; bad_valuesArr[ii].twLimit != 0xFFFFFFFF ; ii++)
            {
                twLimit = bad_valuesArr[ii].twLimit;
                liLimit = bad_valuesArr[ii].liLimit;
                tsLimit = bad_valuesArr[ii].tsLimit;

                st = cpssDxChPortEeeLpiTimeLimitsSet(dev,port,twLimit,liLimit,tsLimit);
                UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev,port,twLimit,liLimit,tsLimit);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        ii = 0;
        twLimit = ok_valuesArr[ii].twLimit;
        liLimit = ok_valuesArr[ii].liLimit;
        tsLimit = ok_valuesArr[ii].tsLimit;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEeeLpiTimeLimitsSet(dev,port,twLimit,liLimit,tsLimit);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev,port,twLimit,liLimit,tsLimit);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    ii = 0;
    twLimit = ok_valuesArr[ii].twLimit;
    liLimit = ok_valuesArr[ii].liLimit;
    tsLimit = ok_valuesArr[ii].tsLimit;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEeeLpiTimeLimitsSet(dev, port,twLimit,liLimit,tsLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEeeLpiTimeLimitsSet(dev, port,twLimit,liLimit,tsLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEeeLpiTimeLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *twLimitPtr,
    OUT GT_U32                  *liLimitPtr,
    OUT GT_U32                  *tsLimitPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEeeLpiTimeLimitsGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32                  twLimitGet;
    GT_U32                  liLimitGet;
    GT_U32                  tsLimitGet;

    /* there is no EEE in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,&liLimitGet,&tsLimitGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                continue;
            }

            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,&liLimitGet,&tsLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,NULL,&liLimitGet,&tsLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,NULL,&tsLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,&liLimitGet,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,&liLimitGet,&tsLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,NULL,&liLimitGet,&tsLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,NULL,&tsLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            st = cpssDxChPortEeeLpiTimeLimitsGet(dev,port,&twLimitGet,&liLimitGet,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEeeLpiTimeLimitsGet(dev, port,&twLimitGet,&liLimitGet,&tsLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEeeLpiTimeLimitsGet(dev, port,&twLimitGet,&liLimitGet,&tsLimitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEeeLpiStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_STATUS_STC *statusPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEeeLpiStatusGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    CPSS_DXCH_PORT_EEE_LPI_STATUS_STC     statusGet;

    /* there is no EEE in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortEeeLpiStatusGet(dev,port,&statusGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev,port);
                continue;
            }

            st = cpssDxChPortEeeLpiStatusGet(dev,port,&statusGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiStatusGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortEeeLpiStatusGet(dev,port,&statusGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);

            /* called with NULL */
            st = cpssDxChPortEeeLpiStatusGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_EEE_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEeeLpiStatusGet(dev, port,&statusGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEeeLpiStatusGet(dev, port,&statusGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChPortEee suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortEee)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEeeLpiRequestEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEeeLpiRequestEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEeeLpiManualModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEeeLpiManualModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEeeLpiTimeLimitsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEeeLpiTimeLimitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEeeLpiStatusGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortEee)


