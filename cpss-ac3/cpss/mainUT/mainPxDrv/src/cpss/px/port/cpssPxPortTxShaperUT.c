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
* @file cpssPxPortTxShaperUT.c
*
* @brief Unit tests for cpssPxPortTxShaper, that provides
* CPSS implementation for configuring the Physical Port Tx Shaper.
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

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortTxShaper.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/* Default valid value for port id */
#define PORT_TX_VALID_PHY_PORT_CNS  0

GT_STATUS prvCpssPxPortTxShaperTokenBucketEntryWrite
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    IN  GT_BOOL slowRateEn,
    IN  GT_U32  tbUpdateRatio,
    IN  GT_U32  tokens,
    IN  GT_U32  maxBucketSize,
    IN  GT_BOOL updCurrentBucketSize,
    IN  GT_U32  currentBucketSize
);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperEnableSet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE], avbModeEnable
           [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortTxShaperEnableGet
        enable = GT_FALSE, avbModeEnable = GT_FALSE
        Expected: GT_OK and the same value.
    1.1.3. Call cpssPxPortTxShaperEnableGet
        enable = GT_TRUE, avbModeEnable = GT_FALSE
        Expected: GT_OK and the same value.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_TX_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;
    GT_BOOL     avbModeEnable = GT_TRUE;
    GT_BOOL     avbModeEnableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE], avbModeEnable
                [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with enable = GT_FALSE, avbModeEnable = GT_FALSE */
            enable = GT_FALSE;
            avbModeEnable = GT_FALSE;

            st = cpssPxPortTxShaperEnableSet(dev, port, enable, avbModeEnable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, enable, avbModeEnable);

            /*
                1.1.2. Call cpssPxPortTxShaperEnableGet
                Expected: GT_OK and the same value.
            */
            st = cpssPxPortTxShaperEnableGet(dev, port, &enableGet, &avbModeEnableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(avbModeEnable, avbModeEnableGet,
                   "get another avbModeEnable than was set: %d, %d", dev, port);


            /* Call function with enable = GT_TRUE, avbModeEnable = GT_TRUE */
            enable = GT_TRUE;
            avbModeEnable = GT_TRUE;

            st = cpssPxPortTxShaperEnableSet(dev, port, enable, avbModeEnable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, enable, avbModeEnable);

            /*
                1.1.3. Call cpssPxPortTxShaperEnableGet
                Expected: GT_OK and the same value.
            */
            st = cpssPxPortTxShaperEnableGet(dev, port, &enableGet, &avbModeEnableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(avbModeEnable, avbModeEnableGet,
                   "get another avbModeEnable than was set: %d, %d", dev, port);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            /* avbModeEnable == GT_TRUE    */
            st = cpssPxPortTxShaperEnableSet(dev, port, enable, avbModeEnable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable,
                avbModeEnable);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        /* avbModeEnable == GT_TRUE    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperEnableSet(dev, port, enable, avbModeEnable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable,
            avbModeEnable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperEnableSet(dev, port, enable, avbModeEnable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port,
            enable, avbModeEnable);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperEnableSet(dev, port, enable, avbModeEnable);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable,
            avbModeEnable);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperEnableGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call function with non-NULL enablePtr and retAvbModeEnable.
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL] .
    Expected: GT_BAD_PTR.
    1.1.3. Call function with retAvbModeEnable [NULL] .
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     retEnable;
    GT_BOOL     retAvbModeEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call function with non-NULL enablePtr and retAvbModeEnable.
               Expected: GT_OK.
            */
            st = cpssPxPortTxShaperEnableGet(dev, port, &retEnable,
                &retAvbModeEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, retEnable,
                retAvbModeEnable);

            /*
               1.1.2. Call function with enablePtr [NULL] .
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxShaperEnableGet(dev, port, NULL, &retAvbModeEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);


            /*
               1.1.3. Call function with retAvbModeEnable [NULL] .
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortTxShaperEnableGet(dev, port, &retEnable, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssPxPortTxShaperEnableGet(dev, port, &retEnable,
                &retAvbModeEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperEnableGet(dev, port, &retEnable,
            &retAvbModeEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperEnableGet(dev, port, &retEnable,
            &retAvbModeEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperEnableGet(dev, port, &retEnable, &retAvbModeEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperProfileSet
(
    IN    GT_U8     devNum,
    IN    GT_U8     portNum,
    IN    GT_U16    burstSize,
    INOUT GT_U32    *maxRatePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperProfileSet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with burstSize [0/ 0xFFF] and non-null maxRatePtr.
        Expected: GT_OK and non-null maxRatePtr.
    1.1.2. Call get func with the same params.
        Expected: GT_OK and the same values.
    1.1.3. Call with out of range burstSize [0x2000] and non-null maxRatePtr.
        Expected: NOT GT_OK.
    1.1.4. Call with burstSize [0] and maxRatePtr [NULL].
        Expected: GT_BAD_PTR.
    1.1.5. Find the minimum rate.
        Loop from rate minimum*100 to minimum rate.
        Check that requested rate always <= return rate,
        except  if rate < minimum rate.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port      = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32      burstSize = 0;
    GT_U32      maxRate   = 0xFFFF;
    GT_U32      burstSizeGet = 1;
    GT_U32      maxRateGet   = 0xFFF1;
    GT_BOOL     enableGet    = GT_FALSE;
    GT_U32      minRate,setMaxRate;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call function with burstSize burstSize [0 / 0xFFF],
               non-null maxRatePtr
               Expected: GT_OK and non-null maxRatePtr.
            */

            /* Call function with burstSize = 0, maxRatePtr != NULL */
            burstSize = 0;
            maxRate = 65;

            st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, burstSize, maxRate);

            st = cpssPxPortTxShaperEnableSet(dev, port, GT_TRUE, GT_TRUE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /*
                1.1.2. Call get func with the same params.
                Expected: GT_OK and the same values.
            */

            st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                &maxRateGet, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(burstSize, burstSizeGet,
                                 "get another burstSize than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(maxRate, maxRateGet,
                                 "get another maxRate than was set: %d, %d", dev, port);


            /* Call function with burstSize == 0xFFF, maxRatePtr != NULL*/
            burstSize = 0xFFF;
            maxRate = 32;

            st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, burstSize, maxRate);

            /*
                1.1.2. Call get func with the same params.
                Expected: GT_OK and the same values.
            */

            st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                &maxRateGet, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(burstSize, burstSizeGet,
                                 "get another burstSize than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(maxRate, maxRateGet,
                                 "get another maxRate than was set: %d, %d", dev, port);



            /*
               1.1.3. Call with out of range burstSize [0x2000] non-null maxRatePtr.
               Expected: NOT GT_OK.
            */
            burstSize = 0x2000;

            st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, burstSize);

            /*
               1.1.4. Call with burstSize [0] maxRatePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            burstSize = 0;

            st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, NULL);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, NULL",
                                         dev, port, burstSize);
            /*
               1.1.5. Find the minimum rate.
                      Loop from rate minimum*100 to minimum rate.
                      Check that requested rate always <= return rate,
                      except  if rate < minimum rate.
            */
            /* find minimum rate */
            minRate = 0;
            st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &minRate);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, burstSize);

            burstSize = 0x200;
            maxRate = minRate*100;
            while(maxRate)
            {
                setMaxRate = maxRate;
                st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, burstSize);

                /* check if minimum */
                if (maxRate > setMaxRate)
                {
                    if (maxRate == minRate)
                    {
                        /*exit loop */
                        break;
                    }
                    else
                    {
                        /*error*/
                        UTF_VERIFY_EQUAL3_PARAM_MAC(maxRate, minRate, dev, port, maxRate);
                    }
                }
                else /* maxRate <= setMaxRate */
                {
                    /* check exact */
                    setMaxRate = maxRate;
                    st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(setMaxRate, maxRate, dev, port, maxRate);
                }
                maxRate--;
            }
        }

        burstSize = 4;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports.*/
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, burstSize == 4, maxRatePtr != NULL */

    st = cpssPxPortTxShaperProfileSet(dev, port, burstSize, &maxRate);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U16  *burstSizePtr,
    OUT GT_U32  *maxRatePtr,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperProfileGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with tcQueue [2].
    Expected: GT_OK.
    1.1.2. Call with wrong burstSizePtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with wrong maxRatePtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32      burstSizeGet = 1;
    GT_U32      userRateGet  = 0xFFF1;
    GT_BOOL     enableGet    = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with correct params.
               Expected: GT_OK.
            */
            st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with wrong burstSizePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxShaperProfileGet(dev, port, NULL,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /*
                1.1.3. Call with wrong userRateGet [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                            NULL, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /*
                1.1.4. Call with wrong enableGet [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                            &userRateGet, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports.*/
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */

            st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                        &userRateGet, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                        &userRateGet, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperProfileGet(dev, port, &burstSizeGet,
                                                    &userRateGet, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperModeSet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    IN  CPSS_PX_PORT_TX_SHAPER_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperModeSet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with mode [CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E /
                           CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E].
        Expected: GT_OK.
    1.1.2. Call cpssPxPortTxShaperModeGet with non-NULL pointers,
        other params same as in 1.1.1.
        Expected: GT_OK and the same values.
    1.1.3. Call with out of range mode [wrong enum values],
        other params same as in 1.1.1.
        Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port = PORT_TX_VALID_PHY_PORT_CNS;

    CPSS_PX_PORT_TX_SHAPER_MODE_ENT   mode    = CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E;
    CPSS_PX_PORT_TX_SHAPER_MODE_ENT   modeGet = CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with mode [CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E /
                                       CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E].
                Expected: GT_OK.
            */
            mode = CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E;

            st = cpssPxPortTxShaperModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.1.2. Call cpssPxPortTxShaperModeGet with non-NULL pointers,
                                                             other params same as in 1.1.1.
                Expected: GT_OK and the same values.
            */
            st = cpssPxPortTxShaperModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssPxPortTxShaperModeGet: %d",  dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d", dev);

            /*
                1.1.1. Call with mode [CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E /
                                       CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E].
                Expected: GT_OK.
            */
            mode = CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E;

            st = cpssPxPortTxShaperModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.1.2. Call cpssPxPortTxShaperModeGet with non-NULL pointers,
                other params same as in 1.1.1.
                Expected: GT_OK and the same values.
            */
            st = cpssPxPortTxShaperModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssPxPortTxShaperModeGet: %d",  dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d", dev);

            /*
                1.1.3. Call with out of range mode [wrong enum values],
                                 other params same as in 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortTxShaperModeSet
                                (dev, port, mode),
                                mode);
        }

        mode = CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortTxShaperModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    mode = CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperModeGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    OUT CPSS_PX_PORT_TX_SHAPER_MODE_ENT   *modePtr
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperModeGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with non NULL modePtr.
        Expected: GT_OK.
    1.1.2. Call with modePtr [NULL],
        other params same as in 1.1.1.
        Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port = PORT_TX_VALID_PHY_PORT_CNS;

    CPSS_PX_PORT_TX_SHAPER_MODE_ENT   mode = CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non NULL modePtr.
                Expected: GT_OK.
            */
            st = cpssPxPortTxShaperModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with modePtr [NULL],
                                 other params same as in 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxShaperModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortTxShaperModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssPxPortTxShaperBaselineSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseline
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperBaselineSet)
{
/*
    1. Go over all active devices.
    1.1. Call with baseline [0 / 1000 / 0xFFFFFF]
        Expected: GT_OK.
    1.2. Call cpssPxPortTxShaperBaselineGet with non-NULL pointers
        other params same as in 1.1.
        Expected: GT_OK and the same values.
    1.3. Call with out of range baseline [0xFFFFFF + 1],
        other params same as in 1.1.
        Expected: NOT GT_OK.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      baseline    = 0;
    GT_U32      baselineGet = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with baseline [0 / 1000 / 0xFFFFFF]
            Expected: GT_OK.
        */
        baseline = 0;

        st = cpssPxPortTxShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, baseline);

        /*
            1.2. Call cpssPxPortTxShaperBaselineGet with non-NULL pointers
                                                           other params same as in 1.1.
            Expected: GT_OK and the same values.
        */
        st = cpssPxPortTxShaperBaselineGet(dev, &baselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssPxPortTxShaperBaselineGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseline, baselineGet,
                   "got another baseline then was set: %d", dev);

        /*
            1.1. Call with baseline [0 / 1000 / 0xFFFFFF]
            Expected: GT_OK.
        */
        baseline = 1000;

        st = cpssPxPortTxShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, baseline);

        /*
            1.2. Call cpssPxPortTxShaperBaselineGet with non-NULL pointers
                                                           other params same as in 1.1.
            Expected: GT_OK and the same values.
        */
        st = cpssPxPortTxShaperBaselineGet(dev, &baselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssPxPortTxShaperBaselineGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseline, baselineGet,
                   "got another baseline then was set: %d", dev);

        /*
            1.1. Call with baseline [0 / 1000 / 0xFFFFFF]
            Expected: GT_OK.
        */
        baseline = 0xFFFFFF;

        st = cpssPxPortTxShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, baseline);

        /*
            1.2. Call cpssPxPortTxShaperBaselineGet with non-NULL pointers
                                                           other params same as in 1.1.
            Expected: GT_OK and the same values.
        */
        st = cpssPxPortTxShaperBaselineGet(dev, &baselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssPxPortTxShaperBaselineGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseline, baselineGet,
                   "got another baseline then was set: %d", dev);

        /*
            1.3. Call with out of range baseline [0xFFFFFF + 1],
                           other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        baseline = 0xFFFFFF + 1;

        st = cpssPxPortTxShaperBaselineSet(dev, baseline);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, baseline);
    }

    baseline = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperBaselineSet(dev, baseline);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperBaselineGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *baselinePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperBaselineGet)
{
/*
    1. Go over all active devices.
    1.1. Call with non NULL baselinePtr.
        Expected: GT_OK.
    1.2. Call with baselinePtr [NULL],
        other params same as in 1.1.
        Expected: GT_BAD_PTR.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      baseline = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non NULL baselinePtr.
            Expected: GT_OK.
        */
        st = cpssPxPortTxShaperBaselineGet(dev, &baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with baselinePtr [NULL],
                           other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxShaperBaselineGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, baselinePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperBaselineGet(dev, &baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperBaselineGet(dev, &baseline);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperConfigurationSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *configsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperConfigurationSet)
{
    /*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call with configsPtr{tokensRate [1 / 15], slowRateRatio [1 / 16],
        tokensRateGran [CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E /
                        CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E],
        portsPacketLength [0 / 0xFFFFFF],
        cpuPacketLength [0 / 0xFFFFFF]
        Expected: GT_OK.
    1.1.2. Call cpssPxPortTxShaperConfigurationGet with non-NULL pointers
        other params same as in 1.1.
        Expected: GT_OK and the same values.
    1.1.3. Call with out of range configsPtr->tokensRate [0 / 16],
        other params same as in 1.1.
        Expected: NOT GT_OK.
    1.1.4. Call with out of range configsPtr->slowRateRatio [0 / 17],
        other params same as in 1.1.
        Expected: NOT GT_OK.
    1.1.5. Call with out of range configsPtr->tokensRateGran [wrong enum values],
        other params same as in 1.1.1.
        Expected: GT_BAD_PARAM.
    1.1.6. Call with out of range configsPtr->portsPacketLength [0xFFFFFF + 1],
        other params same as in 1.1.
        Expected: NOT GT_OK.
    1.1.7. Call with out of range configsPtr->cpuPacketLength [0xFFFFFF + 1],
        other params same as in 1.1.
        Expected: NOT GT_OK.
    1.1.8. Call with configsPtr [NULL],
        other params same as in 1.1.
        Expected: GT_BAD_PTR.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port = PORT_TX_VALID_PHY_PORT_CNS;

    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC configs    = {0};
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC configsGet = {0};


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortTxShaperEnableSet(dev, port, GT_TRUE, GT_TRUE);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, GT_TRUE, GT_TRUE);

            st = prvCpssPxPortTxShaperTokenBucketEntryWrite(dev, port, GT_FALSE, 0, GT_TRUE, 1, 0xFFF, 0xFFF, GT_FALSE, 0);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(port % 2)
            {
                st = cpssPxPortTxShaperModeSet(dev,port,CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E);
            }
            else
            {
                st = cpssPxPortTxShaperModeSet(dev,port,CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E);
            }
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E);
        }

        /*
            1.1.1 Call with configsPtr{tokensRate [1 / 15],
                                      slowRateRatio [1 / 16],
                                      tokensRateGran[CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E /
                                                     CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E],
                                      portsPacketLength [0 / 0xFFFFFF],
                                      cpuPacketLength [0 / 0xFFFFFF]}
            Expected: GT_OK.
        */
        configs.tokensRate        = 1;
        configs.slowRateRatio     = 1;
        configs.tokensRateGran    = CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E;
        configs.portsPacketLength = 0;
        configs.cpuPacketLength   = 0;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call cpssPxPortTxShaperConfigurationGet with non-NULL pointers
                other params same as in 1.1.
            Expected: GT_OK and the same values.
        */
        st = cpssPxPortTxShaperConfigurationGet(dev, &configsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssPxPortTxShaperConfigurationGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.tokensRate, configsGet.tokensRate,
                           "got another configsPtr->tokensRate then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.slowRateRatio, configsGet.slowRateRatio,
                           "got another configsPtr->slowRateRatio then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.tokensRateGran, configsGet.tokensRateGran,
                           "got another configsPtr->tokensRateGran then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.portsPacketLength, configsGet.portsPacketLength,
                           "got another configsPtr->portsPacketLength then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.cpuPacketLength, configsGet.cpuPacketLength,
                           "got another configsPtr->cpuPacketLength then was set: %d", dev);

        /*
            1.1.1 Call with configsPtr{tokensRate [1 / 15],
                slowRateRatio [1 / 16],
                tokensRateGran[CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E /
                               CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E],
                portsPacketLength [0 / 0xFFFFFF],
                cpuPacketLength [0 / 0xFFFFFF]}
            Expected: GT_OK.
        */
        configs.tokensRate        = 15;
        configs.slowRateRatio     = 1;
        configs.tokensRateGran    = CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E;
        configs.portsPacketLength = 0xFFF;
        configs.cpuPacketLength   = 0xFFF;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call cpssPxPortTxShaperConfigurationGet with non-NULL pointers
                other params same as in 1.1.
            Expected: GT_OK and the same values.
        */
        st = cpssPxPortTxShaperConfigurationGet(dev, &configsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssPxPortTxShaperConfigurationGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.tokensRate, configsGet.tokensRate,
                           "got another configsPtr->tokensRate then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.slowRateRatio, configsGet.slowRateRatio,
                           "got another configsPtr->slowRateRatio then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.tokensRateGran, configsGet.tokensRateGran,
                           "got another configsPtr->tokensRateGran then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.portsPacketLength, configsGet.portsPacketLength,
                           "got another configsPtr->portsPacketLength then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.cpuPacketLength, configsGet.cpuPacketLength,
                           "got another configsPtr->cpuPacketLength then was set: %d", dev);

        configs.tokensRate        = 15;
        configs.slowRateRatio     = 16;
        configs.tokensRateGran    = CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;
        configs.portsPacketLength = 0xFFFFFF;
        configs.cpuPacketLength   = 0xFFFFFF;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call cpssPxPortTxShaperConfigurationGet with non-NULL pointers
                other params same as in 1.1.
            Expected: GT_OK and the same values.
        */
        st = cpssPxPortTxShaperConfigurationGet(dev, &configsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssPxPortTxShaperConfigurationGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.tokensRate, configsGet.tokensRate,
                           "got another configsPtr->tokensRate then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.slowRateRatio, configsGet.slowRateRatio,
                           "got another configsPtr->slowRateRatio then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.tokensRateGran, configsGet.tokensRateGran,
                           "got another configsPtr->tokensRateGran then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.portsPacketLength, configsGet.portsPacketLength,
                           "got another configsPtr->portsPacketLength then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(configs.cpuPacketLength, configsGet.cpuPacketLength,
                           "got another configsPtr->cpuPacketLength then was set: %d", dev);

        /*
            1.1.3. Call with out of range configsPtr->tokensRate [0 / 16],
                           other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        /* call with configsPtr->tokensRate = 0 */
        configs.tokensRate = 0;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->tokensRate = %f",
                                         dev, configs.tokensRate);

        /* call with configsPtr->tokensRate = 16 */
        configs.tokensRate = 16;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->tokensRate = %f",
                                         dev, configs.tokensRate);

        configs.tokensRate = 1;

        /*
            1.1.4. Call with out of range configsPtr->slowRateRatio [0 / 17],
                           other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        /* call with configsPtr->slowRateRatio = 0 */
        configs.slowRateRatio = 0;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->slowRateRatio = %f",
                                         dev, configs.slowRateRatio);

        /* call with configsPtr->slowRateRatio = 16 */
        configs.slowRateRatio = 17;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->slowRateRatio = %f",
                                         dev, configs.slowRateRatio);

        configs.slowRateRatio = 1;

        /*
            1.1.5. Call with out of range configsPtr->tokensRateGran [wrong enum values],
                other params same as in 1.1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortTxShaperConfigurationSet
                            (dev, &configs),
                            configs.tokensRateGran);

        /*
            1.1.6. Call with out of range configsPtr->portsPacketLength [0xFFFFFF + 1],
                other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        configs.portsPacketLength = 0xFFFFFF + 1;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->portsPacketLength = %f",
                                         dev, configs.portsPacketLength);

        configs.portsPacketLength = 0;

        /*
            1.1.7. Call with out of range configsPtr->cpuPacketLength [0xFFFFFF + 1],
                other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        configs.cpuPacketLength = 0xFFFFFF + 1;

        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->cpuPacketLength = %f",
                                         dev, configs.cpuPacketLength);

        configs.cpuPacketLength = 0;

        /*
            1.1.8. Call with configsPtr [NULL],
                           other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxShaperConfigurationSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configsPtr = NULL", dev);
    }

    configs.tokensRate        = 1;
    configs.slowRateRatio     = 1;
    configs.tokensRateGran    = CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E;
    configs.portsPacketLength = 0;
    configs.cpuPacketLength   = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperConfigurationSet(dev, &configs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperConfigurationGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *configsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperConfigurationGet)
{
/*
    1. Go over all active devices.
    1.1. Call with non NULL configsPtr.
        Expected: GT_OK.
    1.2. Call with configsPtr [NULL],
        other params same as in 1.1.
        Expected: GT_BAD_PTR.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC configs = {0};


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non NULL configsPtr.
            Expected: GT_OK.
        */
        st = cpssPxPortTxShaperConfigurationGet(dev, &configs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with configsPtr [NULL],
                           other params same as in 1.1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxShaperConfigurationGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configsPtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperConfigurationGet(dev, &configs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperConfigurationGet(dev, &configs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperQueueEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperQueueEnableSet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE] and tcQueue [0/ 1].
        Expected: GT_OK.
    1.1.2. Call cpssPxPortTxShaperQueueProfileGet with the same params.
        Expected: GT_OK and the same enable.
    1.1.3. Call with enable [GT_FALSE and GT_TRUE]
        and out of range tcQueue [CPSS_TC_RANGE_CNS = 8].
        Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U8       tcQueue   = 0;
    GT_BOOL     enable    = GT_TRUE;
    GT_BOOL     enableGet = GT_TRUE;
    GT_BOOL     avbModeEnable = GT_TRUE;
    GT_BOOL     avbModeEnableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE] and tcQueue [0/ 1].
                Expected: GT_OK.
            */
            /* Call function with enable = GT_FALSE, tcQueue = 0 */
            enable = GT_FALSE;
            avbModeEnable = GT_FALSE;
            tcQueue = 0;

            st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
                avbModeEnable);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, tcQueue, enable,
                avbModeEnable);

            /*
                1.1.2. Call cpssPxPortTxShaperQueueProfileGet with the same params.
                Expected: GT_OK and the same enable.
            */
            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue, &enableGet,
                &avbModeEnableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);
            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                         "get another enable than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(avbModeEnable, avbModeEnableGet,
                         "get another avbModeEnable than was set: %d, %d",
                         dev, port);


            /* Call function with enable = GT_TRUE, tcQueue = 1 */
            enable = GT_TRUE;
            avbModeEnable = GT_TRUE;
            tcQueue = 1;

            st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
                avbModeEnable);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, tcQueue, enable,
                avbModeEnable);

            /*
                1.1.2. Call cpssPxPortTxShaperQueueProfileGet with the same params.
                Expected: GT_OK and the same enable.
            */
            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue, &enableGet,
                &avbModeEnableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);
            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                         "get another enable than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(avbModeEnable, avbModeEnableGet,
                         "get another avbModeEnable than was set: %d, %d",
                         dev, port);
            /*
                1.1.3. Call with enable [GT_FALSE and GT_TRUE]
                       and out of range tcQueue [CPSS_TC_RANGE_CNS = 8].
                Expected: GT_BAD_PARAM.
            */
            /* Call function with enable = GT_FALSE, tcQueue = CPSS_TC_RANGE_CNS */
            enable = GT_FALSE;
            avbModeEnable = GT_FALSE;
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
                avbModeEnable);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue,
                enable, avbModeEnable);

            /* Call function with enable = GT_TRUE, tcQueue = CPSS_TC_RANGE_CNS */
            enable = GT_TRUE;
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
                avbModeEnable);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue,
                enable, avbModeEnable);
        }

        enable = GT_TRUE;
        avbModeEnable = GT_TRUE;
        tcQueue = 4;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable = GT_TRUE, tcQueue = 4 */

            st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
                avbModeEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE, tcQueue == 4 */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
            avbModeEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
            avbModeEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE, tcQueue == 4 */

    st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, enable,
        avbModeEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperQueueEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   &enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperQueueEnableGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with tcQueue [0/ 1].
        Expected: GT_OK.
    1.1.2. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8].
        Expected: GT_BAD_PARAM.
    1.1.3. Call with wrong enablePtr [NULL].
        Expected: GT_BAD_PTR.
    1.1.4. Call with wrong avbModeEnableGet [NULL].
        Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port      = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U8       tcQueue   = 0;
    GT_BOOL     enableGet = GT_TRUE;
    GT_BOOL     avbModeEnableGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with tcQueue [0/ 1].
                Expected: GT_OK.
            */
            /* Call function with tcQueue = 0 */
            tcQueue = 0;

            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue,
                &enableGet, &avbModeEnableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /* Call function with tcQueue = 1 */
            tcQueue = 1;

            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue,
                &enableGet, &avbModeEnableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);
            /*
                1.1.2. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8].
                Expected: GT_BAD_PARAM.
            */
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue,
                &enableGet, &avbModeEnableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue);

            tcQueue = 1;

            /*
                1.1.3. Call with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue, NULL,
                &avbModeEnableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tcQueue);

            /*
                1.1.4. Call with wrong avbModeEnableGet [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue,
                &enableGet, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tcQueue);
        }

        tcQueue = 4;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue,
                &enableGet, &avbModeEnableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue, &enableGet,
            &avbModeEnableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue, &enableGet,
            &avbModeEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE, tcQueue == 4 */

    st = cpssPxPortTxShaperQueueEnableGet(dev, port, tcQueue, &enableGet,
        &avbModeEnableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxShaperQueueProfileSet
(
    IN    GT_U8     devNum,
    IN    GT_U8     portNum,
    IN    GT_U8     tcQueue,
    IN    GT_U16    burstSize,
    INOUT GT_U32    *userRatePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperQueueProfileSet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with tcQueue [2], burstSize [0/ 0xFFF] and non-null userRatePtr
        Expected: GT_OK and non-null userRatePtr.
    1.1.2. Call get func with the same params.
        Expected: GT_OK and the same values.
    1.1.3. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8],
        burstSize [17], and non-null userRatePtr.
        Expected: GT_BAD_PARAM.
    1.1.4. Call with tcQueue [4], out of range burstSize [0x2000] and non-null
        userRatePtr.
        Expected: NOT GT_OK.
    1.1.5. Call with tcQueue [4], burstSize [0] and userRatePtr [NULL].
        Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port      = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U8       tcQueue   = 0;
    GT_U32      burstSize = 0;
    GT_U32      userRate   = 0xFFFF;

    GT_U32      burstSizeGet = 1;
    GT_U32      userRateGet  = 0xFFF1;
    GT_BOOL     enableGet    = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with tcQueue [2], burstSize [0 / 0xFFF] non-null userRatePtr.
               Expected: GT_OK and non-null userRatePtr.
            */

            /* Call function with tcQueue = 2, burstSize = 0, userRatePtr != NULL */
            tcQueue = 2;
            burstSize = 0;
            userRate = 65;

            st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, tcQueue, burstSize, userRate);

            st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, GT_TRUE, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);
            /*
                1.1.2. Call get func with the same params.
                Expected: GT_OK and the same values.
            */

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(burstSize, burstSizeGet,
                                 "get another burstSize than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(userRate, userRateGet,
                                 "get another userRate than was set: %d, %d", dev, port);


            /* Call function with tcQueue = 5, burstSize == 0xFFF, userRatePtr != NULL */
            tcQueue = 5;
            burstSize = 0xFFF;
            userRate = 32;

            st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, tcQueue, burstSize, userRate);

            st = cpssPxPortTxShaperQueueEnableSet(dev, port, tcQueue, GT_TRUE, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);
            /*
                1.1.2. Call get func with the same params.
                Expected: GT_OK and the same userRatePtr.
            */

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(burstSize, burstSizeGet,
                                 "get another burstSize than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(userRate, userRateGet,
                                 "get another userRate than was set: %d, %d", dev, port);


            /*
               1.1.3. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8],
               burstSize [17], non-null userRatePtr.
               Expected: GT_BAD_PARAM.
            */
            tcQueue = CPSS_TC_RANGE_CNS;
            burstSize = 17;
            userRate = 50;

            st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue, burstSize, userRate);

            /*
               1.1.4. Call with tcQueue [4], out of range burstSize [0x2000]
                                non-null userRatePtr
               Expected: NOT GT_OK.
            */
            tcQueue = 4;
            burstSize = 0x2000;
            userRate = 33;

            st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
            UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, tcQueue, burstSize, userRate);

            /*
               1.1.5. Call with tcQueue [4], burstSize [0] userRatePtr [NULL]
               Expected: GT_BAD_PTR.
            */
            tcQueue = 4;
            burstSize = 0;

            st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, NULL);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, %d, NULL",
                                         dev, port, tcQueue, burstSize);
        }

        burstSize = 4;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports.*/
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */

            st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, tcQueue == 4, burstSize == 4, userRatePtr != NULL */

    st = cpssPxPortTxShaperQueueProfileSet(dev, port, tcQueue, burstSize, &userRate);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxQShaperProfileGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     tcQueue,
    OUT GT_U16   *burstSizePtr,
    OUT GT_U32   *maxRatePtr,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxShaperQueueProfileGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with tcQueue [2].
        Expected: GT_OK.
    1.1.2. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8] and non-null
        pointers.
        Expected: GT_BAD_PARAM.
    1.1.3. Call with wrong burstSizePtr [NULL].
        Expected: GT_BAD_PTR.
    1.1.4. Call with wrong maxRatePtr [NULL].
        Expected: GT_BAD_PTR.
    1.1.5. Call with wrong enablePtr [NULL].
        Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/


    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port      = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U8       tcQueue   = 0;
    GT_U32      burstSizeGet = 1;
    GT_U32      userRateGet  = 0xFFF1;
    GT_BOOL     enableGet    = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with tcQueue [2].
               Expected: GT_OK.
            */

            tcQueue = 2;

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /*
               1.1.2. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8].
               Expected: GT_BAD_PARAM.
            */
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue);

            /*
                1.1.3. Call with wrong burstSizePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            tcQueue = 4;

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, NULL,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tcQueue);

            /*
                1.1.4. Call with wrong userRateGet [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                            NULL, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tcQueue);

            /*
                1.1.5. Call with wrong enableGet [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                            &userRateGet, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tcQueue);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports.*/
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */

            st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                            &userRateGet, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                        &userRateGet, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                        &userRateGet, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxShaperQueueProfileGet(dev, port, tcQueue, &burstSizeGet,
                                                    &userRateGet, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortTx suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortTxShaper)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperBaselineSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperBaselineGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperConfigurationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperConfigurationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperQueueEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperQueueEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperQueueProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxShaperQueueProfileSet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortTxShaper)



