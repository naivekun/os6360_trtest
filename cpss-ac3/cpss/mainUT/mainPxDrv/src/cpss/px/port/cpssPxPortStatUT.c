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
* @file cpssPxPortStatUT.c
*
* @brief Unit tests for cpssPxPortStat, that provides
* CPSS implementation for port MAC MIB counters.
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

#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


/* defines */

/* Default valid value for port id */
#define PORT_STAT_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCounterGet)
{
    GT_STATUS                   st     = GT_OK;

    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port     = PORT_STAT_VALID_PHY_PORT_CNS;
    CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;
    GT_U64                      cntrVal;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with cntrName []
                                and non-null cntrValuePtr.
               Expected: GT_OK.
            */
            for(cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;
                cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E;
                cntrName++)
            {
                st = cpssPxPortMacCounterGet(dev, port, cntrName, &cntrVal);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
            }

            /*
               1.1.2. Call with out of range ctrlName [CPSS_PX_PORT_MAC_COUNTER____LAST____E /
                                                       wrong enum values]
                                and non-null cntrValuePtr.
               Expected: GT_BAD_PARAM.
            */

            /* Call with ctrlName [CPSS_PX_PORT_MAC_COUNTER____LAST____E] */
            cntrName = CPSS_PX_PORT_MAC_COUNTER____LAST____E;

            st = cpssPxPortMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, cntrName);

            /* Call with ctrlName [wrong enum values] */
            UTF_ENUMS_CHECK_MAC(cpssPxPortMacCounterGet
                                (dev, port, cntrName, &cntrVal),
                                cntrName);
        }

        cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. Call for CPU port [PRV_CPSS_PX_CPU_DMA_NUM_CNS]
        Expected: GT_BAD_PARAM. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
        cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;
        st = cpssPxPortMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, cntrName == CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E */

    st = cpssPxPortMacCounterGet(dev, port, cntrName, &cntrVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS   cpssPxPortMacCountersOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArray
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersOnPortGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Px)
    1.1.1. Call with non-null portMacCounterSetArray.
    Expected: GT_OK.
    1.1.2. Call with portMacCounterSetArray [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS                     st   = GT_OK;

    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM          port = PORT_STAT_VALID_PHY_PORT_CNS;
    CPSS_PX_PORT_MAC_COUNTERS_STC counter;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null portMacCounterSetArray.
               Expected: GT_OK.
            */
            st = cpssPxPortMacCountersOnPortGet(dev, port, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with portMacCounterSetArray [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortMacCountersOnPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, portMacCounterSetArray = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortMacCountersOnPortGet(dev, port, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersOnPortGet(dev, port, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM        */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
        st = cpssPxPortMacCountersOnPortGet(dev, port, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersOnPortGet(dev, port, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersOnPortGet(dev, port, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersClearOnReadSet(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersClearOnReadSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Px)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2 Call cpssPxPortMacCountersClearOnReadGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1. Call function for with enable = GT_FALSE and GT_TRUE.
               Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssPxPortMacCountersClearOnReadSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.2 Call cpssPxPortMacCountersClearOnReadGet.
                Expected: GT_OK and the same value.
            */
            st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d", dev);

            /*
               1.1. Call function for with enable = GT_FALSE and GT_TRUE.
               Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssPxPortMacCountersClearOnReadSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.2 Call cpssPxPortMacCountersClearOnReadGet.
                Expected: GT_OK and the same value.
            */
            st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d", dev);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssPxPortMacCountersClearOnReadSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersClearOnReadSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersClearOnReadSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_TRUE;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersClearOnReadSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortMacCountersClearOnReadSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersClearOnReadGet(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersClearOnReadGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Px)
    1.1. Call with  not null enablePtr.
    Expected: GT_OK.
    1.2 Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call with  not null enablePtr.
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.2 Call with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortMacCountersClearOnReadGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersClearOnReadGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacOversizedPacketsCounterModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    OUT CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacOversizedPacketsCounterModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat2) or ITERATE_DEVICES_PHY_PORTS(xCat3)
    1.1.1 Call with  not null counterModePtr.
    Expected: GT_OK.
    1.1.2 Call with wrong counterModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterMode =
                      CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1 Call with  not null enablePtr.
                Expected: GT_OK.
            */
            st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2 Call with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacOversizedPacketsCounterModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacOversizedPacketsCounterModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat2) or ITERATE_DEVICES_PHY_PORTS(xCat3)
    1.1.1. Call with counterMode [CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E
           / CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E
           / CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortMacOversizedPacketsCounterModeGet.
    Expected: GT_OK and the same counterMode value as was set.
    1.1.3. Call with wrong enum values counterMode and other parameters form 1.1.1
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st     = GT_OK;
    GT_STATUS   expectedSt = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_STAT_VALID_PHY_PORT_CNS;

    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterMode =
            CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterModeGet =
            CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;

    /* Prepare device iterator */
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
                1.1.1. Call with counterMode [CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E
                       / CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E
                       / CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E].
                Expected: GT_OK.
            */
            /* Call function with counterMode [CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E] */
            counterMode = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
            st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxPortMacOversizedPacketsCounterModeGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortMacOversizedPacketsCounterModeGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(counterMode, counterModeGet,
                    "get another enable than was set: %d, %d", dev, port);

            /* Call function with counterMode [CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E] */
            counterMode = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;
            st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxPortMacOversizedPacketsCounterModeGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortMacOversizedPacketsCounterModeGet: %d", dev);
#ifndef GM_USED /* don't test in GM */
            UTF_VERIFY_EQUAL2_STRING_MAC(counterMode, counterModeGet,
                    "get another enable than was set: %d, %d", dev, port);
#endif
            /* Call function with counterMode [CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E] */
            counterMode = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E;
            st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, port);

            /*
                1.1.2. Call cpssPxPortMacOversizedPacketsCounterModeGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssPxPortMacOversizedPacketsCounterModeGet(dev, port, &counterModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortMacOversizedPacketsCounterModeGet: %d", dev);
#ifndef GM_USED /* don't test in GM */
            UTF_VERIFY_EQUAL2_STRING_MAC(counterMode, counterModeGet,
                    "get another enable than was set: %d, %d", dev, port);
#endif

            /*
                1.1.3. Call with wrong enum values counterMode and other parameters form 1.1.1
                Expected: GT_BAD_PARAM.
            */
            if (expectedSt == GT_OK)
            {
                UTF_ENUMS_CHECK_MAC(cpssPxPortMacOversizedPacketsCounterModeSet
                                    (dev, port, counterMode),
                                    counterMode);
            }

        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        counterMode = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    counterMode = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersEnableSet)
{
/*
    1. Go over all active devices.
    1.1 For all active devices go over all available physical ports.
    1.1.1 Try to change MAC counters update enable state and check the results.
          Expected: GT_OK.
    1.2 For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM.
    1.3 For all active devices check function on out of bound value for port number.
        Expected: GT_BAD_PARAM.
    1.4 For all active devices check function for CPU port number.
        Expected: GT_BAD_PARAM.
    2. Go over all non-active devices and devices from non-applicable family
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev = 0;
    GT_PHYSICAL_PORT_NUM    port = 0;
    GT_BOOL                 macCountersEnableStateBeforeTest;
    GT_BOOL                 enableState;
    GT_BOOL                 expectedEnableState;
    PRV_CPSS_PORT_TYPE_ENT  portType = PRV_CPSS_PORT_NOT_APPLICABLE_E;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            portType = PRV_CPSS_PX_PORT_TYPE_MAC(dev, port);

            /* 1.1.1 Try to change MAC counters update enable state and check the results */

            /* Backup MAC counters update enable state */
            st = cpssPxPortMacCountersEnableGet(dev, port, &macCountersEnableStateBeforeTest);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "device %d, port %d", dev, port);

            expectedEnableState = (macCountersEnableStateBeforeTest) ? GT_FALSE : GT_TRUE;

            /* Try to change MAC counter update enable state */
            st = cpssPxPortMacCountersEnableSet(dev, port, expectedEnableState);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "device %d, port %d", dev, port);

            st = cpssPxPortMacCountersEnableGet(dev, port, &enableState);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "device %d, port %d", dev, port);

            /* For CG port type MAC counters update always enabled */
            if (portType != PRV_CPSS_PORT_CG_E)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(expectedEnableState, enableState,
                        "Could not change MAC counters update enable state");
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, enableState,
                        "Uncorrect MAC counters update enable state");
            }

            /* Restore MAC counters update enable state */
            st = cpssPxPortMacCountersEnableSet(dev, port, macCountersEnableStateBeforeTest);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "device %d, port %d", dev, port);

            st = cpssPxPortMacCountersEnableGet(dev, port, &enableState);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "device %d, port %d", dev, port);

            /* For CG port type MAC counters update always enabled */
            if (portType != PRV_CPSS_PORT_CG_E)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(macCountersEnableStateBeforeTest, enableState,
                        "Could not repair MAC counters update enable state");
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, enableState,
                        "Uncorrect MAC counters update enable state");
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "device %d, port %d", dev, port);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortMacCountersEnableSet(dev, port, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "device %d, port %d", dev, port);

            st = cpssPxPortMacCountersEnableGet(dev, port, &enableState);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "device %d, port %d", dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersEnableSet(dev, port, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "device %d, port %d", dev, port);

        st = cpssPxPortMacCountersEnableGet(dev, port, &enableState);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "device %d, port %d", dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersEnableSet(dev, port, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "device %d, port %d", dev, port);

        st = cpssPxPortMacCountersEnableGet(dev, port, &enableState);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "device %d, port %d", dev, port);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersEnableSet(dev, port, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssPxPortMacCountersEnableGet(dev, port, &enableState);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersEnableSet(dev, port, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssPxPortMacCountersEnableGet(dev, port, &enableState);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCounterCaptureGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT    cntrName,
    OUT GT_U64                          *cntrValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCounterCaptureGet)
{
/*
    1. Go over all active devices
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call with cntrName[ CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E
                               ...
                               CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E ],
           non-null cntrValuePtr.
           Expected: GT_OK.
    1.1.2. Call with cntrValuePtr [NULL].
           Expected: GT_BAD_PTR
    1.1.3. Call with ctrlName [CPSS_PX_PORT_MAC_COUNTER____LAST____E]
           and non-null cntrValuePtr.
           Expected: NOT GT_OK.
    1.1.4. Call with cntrName[wrong enum values] and other params from 1.1.3.
           Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For active device check function on out of bound value for port number.
         Expected: GT_BAD_PARAM.
    1.4. For active device check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all non active devices and devices from non applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                     st  = GT_OK;
    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM          port;
    CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName;
    GT_U64                        cntrValue;

    cpssOsBzero((GT_VOID*) &cntrValue, sizeof(cntrValue));

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with cntrName[ CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E,
                                           ...
                                           CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E ],
                       non-null cntrValuePtr.
                Expected: GT_OK.
            */
            for(cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;
                cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E;
                cntrName++)
            {
                st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
            }

            /*
                1.1.2. Call with cntrValuePtr [NULL].
                Expected: GT_BAD_PTR
            */
            cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;

            st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, cntrValuePtr = NULL", dev, port);
            /*
                1.1.3. Call with ctrlName [CPSS_PX_PORT_MAC_COUNTER____LAST____E]
                       and non-null cntrValuePtr.
                Expected: NOT GT_OK.
            */
            cntrName = CPSS_PX_PORT_MAC_COUNTER____LAST____E;

            st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /*
                1.1.4. Call with cntrName[wrong enum values] and other params from 1.1.3.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(
                    cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue),
                    cntrName);
        }

        cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersCaptureOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portNum,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *portMacCounterSetArray
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersCaptureOnPortGet)
{
/*
    1. Go over all active devices
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call with non-null portMacCounterSetArray.
           Expected: GT_OK.
    1.1.2. Call with portMacCounterSetArray [NULL].
           Expected: GT_BAD_PTR
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM
    1.3. For active device check function on out of bound value for port number.
         Expected: GT_BAD_PARAM.
    1.4. For active device check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all non active devices and devices from non applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                      st      = GT_OK;
    GT_U8                          dev;
    GT_PHYSICAL_PORT_NUM           port    = PORT_STAT_VALID_PHY_PORT_CNS;
    CPSS_PX_PORT_MAC_COUNTERS_STC  portMacCounterSetArray;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null portMacCounterSetArray.
                Expected: GT_OK.
            */
            st = cpssPxPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with portMacCounterSetArray [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssPxPortMacCountersCaptureOnPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                    "%d, %d, portMacCounterSetArray = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersCaptureTriggerSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersCaptureTriggerSet)
{
/*
    1. Go over all active devices
    1.1. For all active devices go over all available physical ports.
         Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM
    1.3. For active device check function on out of bound value for port number.
         Expected: GT_BAD_PARAM.
    1.4. For active device check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all non active devices and devices from non applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortMacCountersCaptureTriggerSet(dev, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortMacCountersCaptureTriggerSet(dev, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersCaptureTriggerSet(dev, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersCaptureTriggerSet(dev, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersCaptureTriggerSet(dev, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersCaptureTriggerSet(dev, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersCaptureTriggerGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersCaptureTriggerGet)
{
/*
    1. Go over all active devices
    1.1. For all active devices go over all available physical ports.
         Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM
    1.3. For active device check function on out of bound value for port number.
         Expected: GT_BAD_PARAM.
    1.4. For active device check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all non active devices and devices from non applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st    = GT_OK;
    GT_U8                   dev   = 0;
    GT_PHYSICAL_PORT_NUM    port  = PORT_STAT_VALID_PHY_PORT_CNS;
    GT_BOOL                 isCaptureDone;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortMacCountersCaptureTriggerGet(dev, port, &isCaptureDone);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortMacCountersCaptureTriggerGet(dev, port, &isCaptureDone);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersCaptureTriggerGet(dev, port, &isCaptureDone);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersCaptureTriggerGet(dev, port, &isCaptureDone);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For non-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersCaptureTriggerGet(dev, port, &isCaptureDone);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersCaptureTriggerGet(dev, port, &isCaptureDone);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortStatTxDebugCountersGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortStatTxDebugCountersGet)
{
/*
    1. Go over all active devices
    1.1. Call with non-null dropCntrStcPtr.
         Expected: GT_OK.
    1.2. Call with egrCntrPtr [NULL].
         Expected: GT_BAD_PTR
    2. Go over all non active devices and devices from non applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st;
    GT_U8      dev;
    CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC  dropCntrStc;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null dropCntrStc */
        st = cpssPxPortStatTxDebugCountersGet(dev, &dropCntrStc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with dropCntrStc [NULL]   */
        st = cpssPxPortStatTxDebugCountersGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For non-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortStatTxDebugCountersGet(dev, &dropCntrStc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortStatTxDebugCountersGet(dev, &dropCntrStc);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssPxPortEgressCntrModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_U8                           portNum,
    IN  GT_U32                          tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEgressCntrModeSet)
{
/*
    1. Go over all applicable devices.
    1.1. For all active devices go over all applicable physical ports.
    1.1.1. Call with cntrSetNum[0, 0, 1, 1],
                     setModeBmp [ CPSS_EGRESS_CNT_PORT_E,
                                  CPSS_EGRESS_CNT_TC_E,
                                  CPSS_EGRESS_CNT_DP_E ],
                     tc[1, 2, 5, 7],
                     and dpLevel [ CPSS_DP_GREEN_E,
                                   CPSS_DP_RED_E ]
           Expected:  GT_OK
           Call cpssPxPortEgressCntrModeGet with no NULL pointers and cntrSetNum
           as in 1.1.
           Expected:  GT_OK and the same params as was set
    1.1.2. Call with cntrSetNum [1],
                     wrong enum values setModeBmp,
                     tc [1],
                     dpLevel [CPSS_DP_RED_E].
           Expected:  NOT GT_OK.
    1.1.3. Call with cntrSetNum [1],
                     setModeBmp [CPSS_EGRESS_CNT_TC_E],
                     out of range tc [CPSS_TC_RANGE_CNS=8],
                     dpLevel [CPSS_DP_GREEN_E].
           Expected:  GT_BAD_PARAM.
    1.1.4. Call with cntrSetNum [0],
                     setModeBmp CPSS_EGRESS_CNT_DP_E],
                     tc [5],
                     wrong enum values dpLevel.
           Expected:  GT_BAD_PARAM.
    1.1.5. Call with cntrSetNum [0],
                     setModeBmp [CPSS_EGRESS_CNT_DP_E],
                     tc [5],
                     out of range dpLevel [CPSS_DP_YELLOW_E] (check yellow dpLevel).
           Expected: NOT GT_OK.
    1.1.6. Call with cntrSetNum[200] (check big value for counter set),
                     setModeBmp [CPSS_EGRESS_CNT_DP_E],
                     tc[1],
                     dpLevel [CPSS_DP_GREEN_E].
           Expected: NOT GT_OK.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function for out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all non active devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st  = GT_OK;

    GT_U8                           dev;
    GT_PHYSICAL_PORT_NUM            port        = PORT_STAT_VALID_PHY_PORT_CNS;
    GT_U32                          cntrSetNum  = 0;

    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp  = CPSS_EGRESS_CNT_PORT_E;
    GT_U32                          tc          = 0;
    CPSS_DP_LEVEL_ENT               dpLevel     = CPSS_DP_GREEN_E;

    GT_PHYSICAL_PORT_NUM            portGet        = PORT_STAT_VALID_PHY_PORT_CNS;
    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmpGet  = CPSS_EGRESS_CNT_PORT_E;
    GT_U32                          tcGet          = 0;
    CPSS_DP_LEVEL_ENT               dpLevelGet     = CPSS_DP_GREEN_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with cntrSetNum[0, 0, 1, 1],
                                setModeBmp [ CPSS_EGRESS_CNT_PORT_E,
                                             CPSS_EGRESS_CNT_TC_E,
                                             CPSS_EGRESS_CNT_DP_E ],
                                tc[1, 2, 5, 7],
                                and dpLevel [ CPSS_DP_GREEN_E,
                                              CPSS_DP_RED_E ]
                Expected: GT_OK
            */

            /* Call with:                              */
            /*     cntrSetNum = 0                      */
            /*     setModeBmp = CPSS_EGRESS_CNT_PORT_E */
            /*     tc         = 1                      */
            /*     dpLevel    = CPSS_DP_GREEN_E        */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_PORT_E;
            tc = 1;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);
            UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);

            st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                    &setModeBmpGet, &portGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(setModeBmp, setModeBmpGet,
                    "got another setModeBmp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(port, portGet,
                    "got another port than was set: %d", dev);

            /* Call with:                              */
            /*     cntrSetNum = 1                      */
            /*     setModeBmp = CPSS_EGRESS_CNT_TC_E   */
            /*     tc         = 5                      */
            /*     dpLevel    = CPSS_DP_RED_E          */
            cntrSetNum = 1;
            setModeBmp = CPSS_EGRESS_CNT_TC_E;
            tc = 5;
            dpLevel = CPSS_DP_RED_E;

            st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);
            UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);

            st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                    &setModeBmpGet, &portGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(setModeBmp, setModeBmpGet,
                    "got another setModeBmp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tc, tcGet,
                    "got another tc than was set: %d", dev);

            /* Call with:                              */
            /*     cntrSetNum = 1                      */
            /*     setModeBmp = CPSS_EGRESS_CNT_DP_E   */
            /*     tc         = 7                      */
            /*     dpLevel    = CPSS_DP_GREEN_E        */
            cntrSetNum = 1;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            tc = 7;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);
            UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);

            st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                    &setModeBmpGet, &portGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(setModeBmp, setModeBmpGet,
                    "got another setModeBmp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(dpLevel, dpLevelGet,
                    "got another dpLevel than was set: %d", dev);

            /*
                1.1.2. Call with cntrSetNum [1]
                                 wrong enum values setModeBmp ,
                                 tc [1],
                                 dpLevel [CPSS_DP_RED_E].
                Expected: NOT GT_OK.
            */
            cntrSetNum = 1;
            tc = 1;
            dpLevel = CPSS_DP_RED_E;

            UTF_ENUMS_CHECK_MAC(cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                        setModeBmp, port, tc, dpLevel), setModeBmp);

            /*
                1.1.3. Call with cntrSetNum [1],
                                 setModeBmp [CPSS_EGRESS_CNT_TC_E],
                                 out of range tc [CPSS_TC_RANGE_CNS = 8]
                                 dpLevel [CPSS_DP_GREEN_E].
                Expected: GT_BAD_PARAM.
            */
            cntrSetNum = 1;
            setModeBmp = CPSS_EGRESS_CNT_TC_E;
            tc = CPSS_TC_RANGE_CNS;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tc);

            /*
                1.1.4. Call with cntrSetNum [0],
                                 setModeBmp CPSS_EGRESS_CNT_DP_E],
                                 tc [5]
                                 wrong enum values dpLevel.
                Expected: GT_BAD_PARAM.
            */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            tc = 5;

            UTF_ENUMS_CHECK_MAC(cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                        setModeBmp, port, tc, dpLevel), dpLevel);

            /*
                1.1.5. Call with cntrSetNum [0],
                                 setModeBmp [CPSS_EGRESS_CNT_DP_E],
                                 tc [5],
                                 out of range dpLevel [CPSS_DP_YELLOW_E] (check yellow dpLevel).
                Expected: GT_OK.
            */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            tc = 5;
            dpLevel = CPSS_DP_YELLOW_E;

            st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, dpLevel);

            st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                    &setModeBmpGet, &portGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(dpLevel, dpLevelGet,
                    "got another dpLevel than was set: %d", dev);

            /*
                1.1.6. Call with cntrSetNum[2] (check big value for counter set),
                                 setModeBmp [CPSS_EGRESS_CNT_DP_E],
                                 tc[1],
                                 dpLevel [CPSS_DP_GREEN_E]
                Expected: NOT GT_OK.
            */
            cntrSetNum = 2;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            tc = 1;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, dpLevel);
        }

        cntrSetNum = 0;
        setModeBmp = CPSS_EGRESS_CNT_PORT_E;
        tc = 10;
        dpLevel = CPSS_DP_GREEN_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                    setModeBmp, port, tc, dpLevel);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number                          */
        port = UTF_CPSS_PX_MAX_PORT_NUM_CNS(dev);

        st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                setModeBmp, port, tc, dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                setModeBmp, port, tc, dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function return GT_NOT_APPLICABLE_DEVICE */
    cntrSetNum = 0;
    setModeBmp = CPSS_EGRESS_CNT_PORT_E;
    tc = 10;
    dpLevel = CPSS_DP_GREEN_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
                setModeBmp, port, tc, dpLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEgressCntrModeSet(dev, cntrSetNum,
            setModeBmp, port, tc, dpLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U32                          cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_U8                           *portNumPtr,
    OUT  GT_U32                          *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEgressCntrModeGet)
{
/*
    1. Go over all active devices.
    1.1. Call with cntrSetNum[0, 1]
                   and not NULL pointers setModeBmpPtr,
                                         portNumPtr,
                                         tcPtr,
                                         dpLevelPtr.
         Expected: GT_OK.
    1.2. Call with cntrSetNum [2] and other params as in 1.1.
         Expected: NOT GT_OK.
    1.3. Call with setModeBmpPtr [NULL] and other params as in 1.1.
         Expected: GT_BAD_PTR.
    1.4. Call with portNumPtr [NULL] and other params as in 1.1.
         Expected: GT_BAD_PTR.
    1.5. Call with tcPtr [NULL] and other params as in 1.1.
         Expected: GT_BAD_PTR.
    1.6. Call with dpLevelPtr [NULL] and other params as in 1.1.
         Expected: GT_BAD_PTR.
    2. Go over all non active devices and devices from non applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                          cntrSetNum = 0;
    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp = CPSS_EGRESS_CNT_PORT_E;
    GT_PHYSICAL_PORT_NUM            port       = PORT_STAT_VALID_PHY_PORT_CNS;
    GT_U32                          tc         = 0;
    CPSS_DP_LEVEL_ENT               dpLevel    = CPSS_DP_GREEN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with cntrSetNum[0, 1]
                      and not NULL pointers setModeBmpPtr,
                                            portNumPtr,
                                            tcPtr,
                                            dpLevelPtr.
            Expected: GT_OK.
        */
        /* iterate with cntrSetNum = 0 */
        cntrSetNum = 0;

        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                &setModeBmp, &port, &tc, &dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetNum);

        /* iterate with cntrSetNum = 1 */
        cntrSetNum = 1;

        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                &setModeBmp, &port, &tc, &dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetNum);

        /*
            1.2. Call with cntrSetNum [2] and other params as in 1.1.
            Expected: NOT GT_OK.
        */
        cntrSetNum = 2;

        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                &setModeBmp, &port, &tc, &dpLevel);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetNum);

        cntrSetNum = 0;

        /*
            1.3. Call with setModeBmpPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                NULL, &port, &tc, &dpLevel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, setModeBmpPtr = NULL", dev);

        /*
            1.4. Call with portNumPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                &setModeBmp, NULL, &tc, &dpLevel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portPtr = NULL", dev);

        /*
            1.5. Call with tcPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                &setModeBmp, &port, NULL, &dpLevel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcPtr = NULL", dev);

        /*
            1.6. Call with dpLevelPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                &setModeBmp, &port, &tc, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dpLevelPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
                &setModeBmp, &port, &tc, &dpLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEgressCntrModeGet(dev, cntrSetNum,
            &setModeBmp, &port, &tc, &dpLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEgressCntrsGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cntrSetNum,
    OUT CPSS_PX_PORT_EGRESS_CNTR_STC    *egrCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEgressCntrsGet)
{
/*
    1. Go over all applicable devices.
    1.1. Call function with cntrSetNum [1] and non-null egrCntrPtr.
         Expected: GT_OK.
    1.2. Call with with cntrSetNum [0] and egrCntrPtr [NULL].
         Expected: GT_BAD_PTR.
    1.3. Call with cntrSetNum [254] and non-null egrCntrPtr.
         Expected: NOT GT_OK.
    2. Go over all non active devices and devices from non applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st     = GT_OK;

    GT_U8                           dev;
    GT_U32                          setNum;
    CPSS_PX_PORT_EGRESS_CNTR_STC    egrCntr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with cntrSetNum [1] and non-null egrCntrPtr.
           Expected: GT_OK.
        */
        setNum = 1;

        st = cpssPxPortEgressCntrsGet(dev, setNum, &egrCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, setNum);

        /*
            1.2. Call with cntrSetNum [0] and egrCntrPtr [NULL].
            Expected: GT_BAD_PTR
        */
        setNum = 0;

        st = cpssPxPortEgressCntrsGet(dev, setNum, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, setNum);

        /*
            1.3. Call with cntrSetNum [254] and non-null egrCntrPtr.
            Expected: NOT GT_OK.
        */
        setNum = 254;

        st = cpssPxPortEgressCntrsGet(dev, setNum, &egrCntr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, setNum);
    }

    setNum = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEgressCntrsGet(dev, setNum, &egrCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEgressCntrsGet(dev, setNum, &egrCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersRxHistogramEnableSet(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersRxHistogramEnableSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
         Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check functions with out of bound value for
         port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check functions for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all non active devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS            st = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL              enable;
    GT_BOOL              enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* Call function for with enable = GT_FALSE and GT_TRUE.   */
            /* Expected:  GT_OK.                                       */
            enable = GT_FALSE;

            st = cpssPxPortMacCountersRxHistogramEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssPxPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "got another enable than was set: %d, %d", dev, port);

            enable = GT_TRUE;

            st = cpssPxPortMacCountersRxHistogramEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssPxPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "got another enable than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* Call function for each non-active port    */
            st = cpssPxPortMacCountersRxHistogramEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            st = cpssPxPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersRxHistogramEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        st = cpssPxPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM  */
        /* for CPU port number.                                             */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersRxHistogramEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        st = cpssPxPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_TRUE;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersRxHistogramEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssPxPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersRxHistogramEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssPxPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacCountersTxHistogramEnableSet(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacCountersTxHistogramEnableSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
         Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check functions with out of bound value for
         port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check functions for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all non active devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS            st = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL              enable;
    GT_BOOL              enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* Call function for with enable = GT_FALSE and GT_TRUE.   */
            /* Expected:  GT_OK.                                       */
            enable = GT_FALSE;

            st = cpssPxPortMacCountersTxHistogramEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssPxPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "got another enable than was set: %d, %d", dev, port);

            enable = GT_TRUE;

            st = cpssPxPortMacCountersTxHistogramEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssPxPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "got another enable than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* Call function for each non-active port    */
            st = cpssPxPortMacCountersTxHistogramEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            st = cpssPxPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacCountersTxHistogramEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        st = cpssPxPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM  */
        /* for CPU port number.                                             */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMacCountersTxHistogramEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        st = cpssPxPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_TRUE;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacCountersTxHistogramEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssPxPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacCountersTxHistogramEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssPxPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortStat suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortStat)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersOnPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersClearOnReadSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersClearOnReadGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacOversizedPacketsCounterModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacOversizedPacketsCounterModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCounterCaptureGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersCaptureOnPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersCaptureTriggerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersCaptureTriggerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortStatTxDebugCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEgressCntrModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEgressCntrModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEgressCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersRxHistogramEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacCountersTxHistogramEnableSet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortStat)

static GT_CHAR* counterNames[CPSS_PX_PORT_MAC_COUNTER____LAST____E] = {
     STR(CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E         )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_BAD_OCTETS_RECEIVED_E          )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_CRC_ERRORS_SENT_E              )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E    )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E    )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAMES_64_OCTETS_E             )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAMES_65_TO_127_OCTETS_E      )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAMES_128_TO_255_OCTETS_E     )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAMES_256_TO_511_OCTETS_E     )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAMES_512_TO_1023_OCTETS_E    )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E    )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E   )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E             )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E           )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E        )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E        )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E                      )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FC_RECEIVED_E                  )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_RECEIVED_FIFO_OVERRUN_E        )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_UNDERSIZE_E                    )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_FRAGMENTS_E                    )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_OVERSIZE_E                     )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_JABBER_E                       )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_RX_ERROR_FRAME_RECEIVED_E      )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_BAD_CRC_E                      )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_COLLISION_E                    )
    ,STR(CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E               )
};

extern GT_STATUS prvTgfPxSkipPortWithNoMac(IN GT_SW_DEV_NUM devNum , IN GT_PHYSICAL_PORT_NUM portNum);

#include <cpss/px/networkIf/cpssPxNetIf.h>
GT_STATUS showCpuSdmaCounters
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_STATUS rc;
    GT_U32  rxQueue;
    GT_U32  cpuOnQueueBmp = 0;
    CPSS_PX_NET_SDMA_RX_COUNTERS_STC cpuCounters[CPSS_TC_RANGE_CNS];

    for(rxQueue = 0 ; rxQueue < CPSS_TC_RANGE_CNS; rxQueue++)
    {
        rc = cpssPxNetIfSdmaRxCountersGet(devNum,rxQueue,&cpuCounters[rxQueue]);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(0 == cpuCounters[rxQueue].rxInPkts &&
           0 == cpuCounters[rxQueue].rxInOctets)
        {
            continue;
        }

        cpuOnQueueBmp |= 1<<rxQueue;
    }

    if(cpuOnQueueBmp == 0)
    {
        cpssOsPrintf("CPU got no packets \n");
        return GT_OK;
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("CPU got packets on next queues :\n");
    cpssOsPrintf("queue  |  rxInPkts    |  rxInOctets \n");
    cpssOsPrintf("----------------------------------- \n");

    for(rxQueue = 0 ; rxQueue < CPSS_TC_RANGE_CNS; rxQueue++)
    {
        if(0 == (cpuOnQueueBmp & (1<<rxQueue)))
        {
            continue;
        }

        cpssOsPrintf("  %3d  |  %4d        | %8d \n" ,
            rxQueue ,
            cpuCounters[rxQueue].rxInPkts ,
            cpuCounters[rxQueue].rxInOctets);
    }

    cpssOsPrintf("\n");

    return GT_OK;
}

/* debug function to print all counters on ALL ports ,
   using :  cpssPxPortMacCountersOnPortGet(...) */
void cpssPxPortStatShowAllPorts(IN GT_SW_DEV_NUM devNum)
{
    GT_STATUS rc;
    GT_U32  ii;
    CPSS_PX_PORT_MAC_COUNTERS_STC   counters[PRV_CPSS_PX_PORTS_NUM_CNS];
    GT_U32  preficNameOffset = cpssOsStrlen("CPSS_PX_PORT_MAC_COUNTER_");
    GT_U32  validPortsBmp = 0;
    CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName;
    GT_CHAR *currNamePtr;
    GT_U32  value;

    /***************************/
    /* get info from all ports */
    /***************************/
    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }

        rc = cpssPxPortMacCountersOnPortGet(devNum,ii,&counters[ii]);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed on port[%d] \n",ii);
            continue;
        }

        for(cntrName = 0 ; cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E; cntrName++)
        {
            if(counters[ii].mibCounter[cntrName].l[0])
            {
                /* this counter is not empty so we can print the counters of port */
                break;
            }
        }

        if(cntrName == CPSS_PX_PORT_MAC_COUNTER____LAST____E)
        {
            /* all counters are ZERO */
            cpssOsPrintf("port[%d] all counters are ZERO \n",ii);
            continue;
        }

        validPortsBmp |= 1<<ii;
    }

    if(validPortsBmp)
    {
        cpssOsPrintf("%32s ","ports:");
        for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
        {
            if(0 == (validPortsBmp & (1<<ii)))
            {
                continue;
            }

            cpssOsPrintf("%4d ",ii);
        }
        cpssOsPrintf("\n");

        cpssOsPrintf("%32s ","------");
        for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
        {
            if(0 == (validPortsBmp & (1<<ii)))
            {
                continue;
            }

            cpssOsPrintf("%5s","-----");
        }
        cpssOsPrintf("\n");

        /*print the info*/
        for(cntrName = 0 ; cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E; cntrName++)
        {
            currNamePtr = counterNames[cntrName];
            cpssOsPrintf("%32s ", currNamePtr + preficNameOffset);
            for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
            {
                if(0 == (validPortsBmp & (1<<ii)))
                {
                    continue;
                }

                value = counters[ii].mibCounter[cntrName].l[0];

                if(value)
                {
                    cpssOsPrintf("%4d ",value);
                }
                else
                {
                    cpssOsPrintf("---- ");
                }
            }

            cpssOsPrintf("\n");
        }
    }
    else
    {
        cpssOsPrintf("all counters on all network ports are ZERO \n");
    }

    (void)showCpuSdmaCounters(devNum);

    return;
}

/* debug function to print all counters on ALL ports ,
   using :  cpssPxPortMacCountersOnPortGet(...) */
GT_STATUS cpssPxPortStatShowCounter(
    IN GT_SW_DEV_NUM devNum ,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_CHAR*         counterNameStr
)
{
    GT_STATUS   rc;
    GT_U32  preficNameOffset = cpssOsStrlen("CPSS_PX_PORT_MAC_COUNTER_");
    CPSS_PX_PORT_MAC_COUNTER_ENT cntrName;
    GT_U64                      cntrValue;
    GT_CHAR *currNamePtr;

    if(counterNameStr == NULL)
    {
        rc =  GT_BAD_PTR;
        cpssOsPrintf("counter name is NULL here are recognized names:\n");
        goto printRecognizedNames_lbl;
    }

    for(cntrName = 0 ; cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E; cntrName++)
    {
        currNamePtr = counterNames[cntrName];
        if(0 == cpssOsStrCmp(counterNameStr,currNamePtr))
        {
            break;
        }

        currNamePtr += preficNameOffset;
        if(0 == cpssOsStrCmp(counterNameStr,currNamePtr))
        {
            break;
        }
    }

    if(cntrName == CPSS_PX_PORT_MAC_COUNTER____LAST____E)
    {
        rc =  GT_BAD_PARAM;

        cpssOsPrintf("counter name [%d] was not found recognized , here are recognized names:\n");

        printRecognizedNames_lbl:
        for(cntrName = 0 ; cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E; cntrName++)
        {
            currNamePtr = counterNames[cntrName];
            cpssOsPrintf("%s \t\t or ",currNamePtr);
            currNamePtr += preficNameOffset;
            cpssOsPrintf("%s \n",currNamePtr);
        }

        return rc;
    }

    rc = cpssPxPortMacCounterGet(devNum,portNum,cntrName,&cntrValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("cpssPxPortMacCounterGet: failed on port[%d]:\n",portNum);
        return rc;
    }

    currNamePtr = counterNames[cntrName];
    currNamePtr += preficNameOffset;
    cpssOsPrintf("%32s ", currNamePtr);
    cpssOsPrintf("%4d ",cntrValue.l[0]);
    cpssOsPrintf("\n");

    return GT_OK;
}



