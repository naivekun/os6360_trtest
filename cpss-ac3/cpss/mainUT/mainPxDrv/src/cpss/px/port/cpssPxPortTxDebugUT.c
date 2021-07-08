/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssPxPortTxUT.c
*
* DESCRIPTION:
*       Unit tests for cpssPxPortTxDebug, that provides
*       CPSS implementation for configuring the Physical Port Tx Traffic Class.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/port/cpssPxPortTxDebug.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>


/* Default valid value for port id */
#define PORT_TX_VALID_PHY_PORT_CNS  0


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugQueueingEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U8           portNum,
    IN  GT_U32          tcQueue,
    IN  GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugQueueingEnableSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available ports.
    1.1.1. Call function with enable[GT_FALSE,GT_TRUE] and tcQueue[0,4].
           Expected: GT_OK.
    1.1.2. Call function with enable[GT_FALSE,GT_TRUE] and out of range
           tcQueue[CPSS_TC_RANGE_CNS].
           Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non-available ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices call function with out of bound value of
         port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices call function for CPU port.
         Expected: GT_OK.
    2. Go over all non-active and non-applicable devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st          = GT_OK;
    GT_U8                   dev         = 0;
    GT_PHYSICAL_PORT_NUM    port        = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32                  tcQueue     = 0;
    GT_BOOL                 enable      = GT_TRUE;
    GT_BOOL                 enableGet;

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
                1.1.1. Call function for with enable = GT_FALSE and GT_TRUE
                       tcQueue [0, 4]
                Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE, tcQueue = 0*/
            enable = GT_FALSE;
            tcQueue = 0;

            st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, tcQueue, enable);

            st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another Queueing status then was set: %d", dev);

            /* Call function with enable = GT_TRUE, tcQueue = 4 */
            enable = GT_TRUE;
            tcQueue = 4;

            st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, tcQueue, enable);

            st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another Queueing status then was set: %d", dev);

            /*
                1.1.2. Call with enable [GT_FALSE and GT_TRUE]
                       out of range tcQueue [CPSS_TC_RANGE_CNS = 8]
                Expected: GT_BAD_PARAM.
            */

            /* Call function with enable = GT_FALSE, tcQueue = CPSS_TC_RANGE_CNS */
            enable = GT_FALSE;
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue, enable);

            /* Call function with enable = GT_TRUE, tcQueue = CPSS_TC_RANGE_CNS*/
            enable = GT_TRUE;
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue, enable);
        }

        enable = GT_TRUE;
        tcQueue = 2;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
                 enable == GT_TRUE, tcQueue == 2
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
                 enable == GT_TRUE, tcQueue == 2
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /*
        2. For not active devices check that function returns non GT_OK.
    */

    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id
           port == 0, enable == GT_TRUE, tcQueue == 2
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugQueueingEnableSet(dev, port, tcQueue, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugQueueingEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U8           portNum,
    IN  GT_U32          tcQueue,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugQueueingEnableGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all applicable ports.
    1.1.1. Call function with tcQueue[0].
           Expected: GT_OK.
    1.1.2. Call function with tcQueue[1].
           Expected: GT_OK.
    1.1.3. Call function with tcQueue[CPSS_TC_RANGE_CNS].
           Expected: GT_BAD_PARAM.
    1.1.4. Call function with wrong enablePtr[NULL].
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available ports.
         Expected: GT_BAD_PARAM.
    1.3. Call function with out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. Call function for CPU port.
         Expected: GT_OK.
    2. Go over all non-active and not applicable devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st          = GT_OK;
    GT_U8                   dev         = 0;
    GT_PHYSICAL_PORT_NUM    port        = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32                  tcQueue     = 0;
    GT_BOOL                 enableGet   = GT_TRUE;

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
                1.1.1. Call with tcQueue [0].
                Expected: GT_OK.
            */
            tcQueue = 0;

            st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /*
                1.1.2. Call with tcQueue [0].
                Expected: GT_OK.
            */
            tcQueue = 1;

            st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /*
                1.1.3. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8].
                Expected: GT_BAD_PARAM.
            */
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue);

            /*
                1.1.4. Call with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            tcQueue = 1;

            st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tcQueue);
        }

        tcQueue = 4;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK for
                 CPU port number.
                 enable == GT_TRUE, tcQueue == 4
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /*
        2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.
    */
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id
           port == 0, enable == GT_TRUE, tcQueue == 4
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugQueueingEnableGet(dev, port, tcQueue, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U8           portNum,
    IN  GT_U32          tcQueue,
    IN  GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugQueueTxEnableSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available ports.
    1.1.1. Call function with enable[GT_FALSE] and tcQueue[0].
           Expected: GT_OK.
    1.1.2. Call function with enable[GT_TRUE] and tcQueue[2].
           Expected: GT_OK.
    1.1.3. Call with enable [GT_FALSE] and out of range tcQueue[CPSS_TC_RANGE_CNS].
           Expected: GT_BAD_PARAM.
    1.1.4. Call with enable [GT_TRUE] and out of range tcQueue[CPSS_TC_RANGE_CNS].
           Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices call function with out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices call function for CPU port.
         Expected: GT_OK.
    2. Go over all non-active and not applicable devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st          = GT_OK;
    GT_U8                   dev         = 0;
    GT_PHYSICAL_PORT_NUM    port        = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32                  tcQueue     = 0;
    GT_BOOL                 enable      = GT_TRUE;
    GT_BOOL                 enableGet;

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
                1.1.1. Call function for with enable[GT_FALSE] and tcQueue[0].
                Expected: GT_OK.
            */
            enable = GT_FALSE;
            tcQueue = 0;

            st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, tcQueue, enable);

            st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another Tx Queue status then was set: %d", dev);

            /*
                1.1.2. Call function for with enable[GT_TRUE] and tcQueue[2].
                Expected: GT_OK.
            */
            enable = GT_TRUE;
            tcQueue = 2;

            st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, tcQueue, enable);

            st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another Tx Queue status then was set: %d", dev);

            /*
                1.1.3. Call with enable [GT_FALSE] and out of range
                       tcQueue[CPSS_TC_RANGE_CNS].
                Expected: GT_BAD_PARAM.
            */
            enable = GT_FALSE;
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue, enable);

            /*
                1.1.4. Call with enable [GT_TRUE] and out of range
                       tcQueue[CPSS_TC_RANGE_CNS].
                Expected: GT_BAD_PARAM.
            */
            enable = GT_TRUE;
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue, enable);
        }

        tcQueue = 4;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
                 enable == GT_TRUE, tcQueue == 4
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK for CPU port.
                 enable == GT_TRUE, tcQueue == 4
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /*
        2. For not active devices check that function returns non GT_OK.
    */
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
           port == 0, enable == GT_TRUE, tcQueue == 4
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugQueueTxEnableSet(dev, port, tcQueue,  enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U8           portNum,
    IN  GT_U32          tcQueue,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugQueueTxEnableGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available ports.
    1.1.1. Call function with tcQueue[0].
           Expected: GT_OK.
    1.1.2. Call function with tcQueue[2].
           Expected: GT_OK.
    1.1.3. Call function with out of range tcQueue[CPSS_TC_RANGE_CNS].
           Expected: GT_BAD_PARAM.
    1.1.4. Call function with wrong enablePtr [NULL].
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices call function with out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices call function for CPU port.
         Expected: GT_OK.
    2. Go over all non-active and not applicable devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st          = GT_OK;
    GT_U8                   dev         = 0;
    GT_PHYSICAL_PORT_NUM    port        = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32                  tcQueue     = 0;
    GT_BOOL                 enableGet   = GT_TRUE;

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
                1.1.1. Call with tcQueue [0].
                Expected: GT_OK.
            */
            tcQueue = 0;

            st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /*
                1.1.2. Call with tcQueue [1].
                Expected: GT_OK.
            */
            tcQueue = 1;

            st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tcQueue);

            /*
                1.1.3. Call with out of range tcQueue [CPSS_TC_RANGE_CNS = 8].
                Expected: GT_BAD_PARAM.
            */
            tcQueue = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tcQueue);

            /*
                1.1.4. Call with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            tcQueue = 1;

            st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tcQueue);
        }

        tcQueue = 4;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK for
                 CPU port number.
                 enable == GT_TRUE, tcQueue == 4
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /*
        2. For not active devices check that function returns non GT_OK.
    */
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
           port == 0, enable == GT_TRUE, tcQueue == 4
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugQueueTxEnableGet(dev, port, tcQueue, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugResourceHistogramThresholdSet
(
    IN  GT_U8  dev,
    IN  GT_U32 cntrNum,
    IN  GT_U32 threshold
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugResourceHistogramThresholdSet)
{
/*
    1. Go over all active devices.
    1.1. Call function with cntrNum[0] and threshold[0].
         Expected: GT_OK.
    1.1. Call function with cntrNum[1] and threshold[1].
         Expected: GT_OK.
    1.1. Call function with cntrNum[2] and threshold[0x2FFF].
         Expected: GT_OK.
    1.1. Call function with cntrNum[3] and threshold[0xFFFF].
         Expected: GT_OK.
    1.5. Call function with wrong cntrNum[4].
         Expected: GT_BAD_PARAM.
    1.6. Call function with wrong threshold[0x10000].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st            = GT_OK;
    GT_U8      dev           = 0;
    GT_U32     cntrNum       = 0;
    GT_U32     threshold     = 0;
    GT_U32     thresholdGet  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with cntrNum[0] and threshold[0].
            Expected: GT_OK.
        */
        cntrNum = 0;
        threshold = 0;

        st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &thresholdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPortTxDebugResourceHistogramThresholdGet: %d ", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                       "got another threshold then was set: %d", dev);

        /*
            1.2. Call function with cntrNum[1] and threshold[1].
            Expected: GT_OK.
        */
        cntrNum = 1;
        threshold = 1;

        st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &thresholdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPortTxDebugResourceHistogramThresholdGet: %d ", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                       "got another threshold then was set: %d", dev);

        /*
            1.3. Call function with cntrNum[2] and threshold[0x2FFF].
            Expected: GT_OK.
        */
        cntrNum = 2;
        threshold = 0x2FFF;

        st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &thresholdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPortTxDebugResourceHistogramThresholdGet: %d ", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                       "got another threshold then was set: %d", dev);

        /*
            1.4. Call function with cntrNum[3] and threshold[0xFFFF].
            Expected: GT_OK.
        */
        cntrNum = 3;
        threshold = 0xFFFF;

        st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &thresholdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPortTxDebugResourceHistogramThresholdGet: %d ", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                       "got another threshold then was set: %d", dev);

        /*
            1.5. Call api with wrong cntrNum [4].
            Expected: GT_BAD_PARAM.
        */
        cntrNum = 4;

        st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.6. Call api with wrong threshold[0x10000].
            Expected: GT_OUT_OF_RANGE.
        */
        cntrNum = 0;
        threshold = 0x010000;

        st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* restore correct values */
    cntrNum = 0;
    threshold = 0;

    /*
        2. For not-active devices and devices from non-applicable family check
           that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugResourceHistogramThresholdSet(dev, cntrNum, threshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugResourceHistogramThresholdGet
(
    IN  GT_U8   dev,
    IN  GT_U32  cntrNum,
    OUT GT_U32  *thresholdPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugResourceHistogramThresholdGet)
{
/*
    1. Go over all active devices.
    1.1. Call function with cntrNum[0]
         Expected: GT_OK.
    1.2. Call function with cntrNum[1]
         Expected: GT_OK.
    1.3. Call function with cntrNum[2]
         Expected: GT_OK.
    1.4. Call function with cntrNum[3]
         Expected: GT_OK.
    1.5. Call function with wrong cntrNum[4]
         Expected: GT_BAD_PARAM.
    1.6. Call function with wrong thresholdPtr[NULL]
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;
    GT_U8   dev;
    GT_U32  cntrNum;
    GT_U32  threshold;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with cntrNum[0].
            Expected: GT_OK.
        */
        cntrNum = 0;

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with cntrNum[1].
            Expected: GT_OK.
        */
        cntrNum = 1;

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call function with cntrNum[2].
            Expected: GT_OK.
        */
        cntrNum = 2;

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call function with cntrNum[3].
            Expected: GT_OK.
        */
        cntrNum = 3;

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5. Call function with wrong cntrNum[4].
            Expected: GT_BAD_PARAM.
        */
        cntrNum = 4;

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.6. Call api with wrong thresholdPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        cntrNum = 0;

        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "thresholdPtr is NULL", dev);
    }

    /* restore correct values */
    cntrNum = 0;

    /*
        2. Go over all not-active devices and devices from non-applicable family.
           Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugResourceHistogramThresholdGet(dev, cntrNum, &threshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugResourceHistogramCounterGet
(
    IN  GT_U8   dev,
    IN  GT_U32  cntrNum,
    OUT GT_U32  *cntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugResourceHistogramCounterGet)
{
/*
    1. Go over all active devices.
    1.1. Call function with cntrNum[0].
         Expected: GT_OK.
    1.2. Call function with cntrNum[1].
         Expected: GT_OK.
    1.3. Call function with cntrNum[2].
         Expected: GT_OK.
    1.4. Call function with cntrNum[3].
         Expected: GT_OK.
    1.5. Call function with wrong cntrNum[4].
         Expected: GT_BAD_PARAM.
    1.6. Call function with wrong cntrPtr[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st;
    GT_U8      dev;
    GT_U32     cntrNum;
    GT_U32     cntr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with cntrNum[0].
            Expected: GT_OK.
        */
        cntrNum = 0;

        st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, &cntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with cntrNum[1].
            Expected: GT_OK.
        */
        cntrNum = 1;

        st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, &cntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call function with cntrNum[2].
            Expected: GT_OK.
        */
        cntrNum = 2;

        st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, &cntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call function with cntrNum[3].
            Expected: GT_OK.
        */
        cntrNum = 3;

        st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, &cntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5. Call api with wrong cntrNum[4].
            Expected: GT_BAD_PARAM.
        */
        cntrNum = 4;

        st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, &cntr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.6. Call api with wrong cntrPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        cntrNum = 0;

        st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "cntrPtr is NULL", dev);
    }

    /* restore correct values */
    cntrNum = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, &cntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugResourceHistogramCounterGet(dev, cntrNum, &cntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugGlobalDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          limit
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugGlobalDescLimitSet)
{
/*
    1. Go over all active devices.
    1.1. Call function with limit[0].
         Expected: GT_OK.
    1.2. Call function with limit[0x1000].
         Expected: GT_OK.
    1.3. Call function with limit[0xFFFF].
         Expected: GT_OK.
    1.4. Call api with wrong limit[65536].
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st;
    GT_U8      dev;
    GT_U32     limit;
    GT_U32     limitGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with limit[0].
            Expected: GT_OK
        */
        limit = 0;

        st = cpssPxPortTxDebugGlobalDescLimitSet(dev, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugGlobalDescLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(limit, limitGet, "get another limit value -", dev);

        /*
            1.2. Call function with limit[0x1000].
            Expected: GT_OK
        */
        limit = 0x1000;

        st = cpssPxPortTxDebugGlobalDescLimitSet(dev, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugGlobalDescLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(limit, limitGet, "get another limit value -", dev);

        /*
            1.3. Call function with limit[0xFFFF].
            Expected: GT_OK
        */
        limit = 0xFFFF;

        st = cpssPxPortTxDebugGlobalDescLimitSet(dev, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugGlobalDescLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(limit, limitGet, "get another limit value -", dev);

        /*
            1.4. Call api with wrong limit[0x10000].
            Expected: GT_OUT_OF_RANGE.
        */
        limit = 0x10000;

        st = cpssPxPortTxDebugGlobalDescLimitSet(dev, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* restore correct values */
    limit = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugGlobalDescLimitSet(dev, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugGlobalDescLimitSet(dev, limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugGlobalDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *limitPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugGlobalDescLimitGet)
{
/*
    1. Go over all active devices.
    1.1. Call function with valid limit variable.
         Expected: GT_OK.
    1.2. Call function with wrong limit[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from not applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS st;
    GT_U8   dev;
    GT_U32  limit;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid limit variable.
            Expected: GT_OK.
        */
        st = cpssPxPortTxDebugGlobalDescLimitGet(dev, &limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with wrong limit[NULL].
        */
        st = cpssPxPortTxDebugGlobalDescLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugGlobalDescLimitGet(dev, &limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugGlobalDescLimitGet(dev, &limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxDebugGlobalQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugGlobalQueueTxEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssPxPortTxDebugGlobalQueueTxEnableGet.
    Expected: GT_OK and the same values than was set.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable[GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssPxPortTxDebugGlobalQueueTxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugGlobalQueueTxEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet, "get another enable value -", dev);

        /*
            1.2. Call function with enable[GT_TRUE].
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssPxPortTxDebugGlobalQueueTxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxPortTxDebugGlobalQueueTxEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet, "get another enable value -", dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugGlobalQueueTxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugGlobalQueueTxEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortTxDebugGlobalQueueTxEnableGet)
/*
    GT_STATUS cpssPxPortTxDebugGlobalQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
*/
{
/*
    1. Go over all active devices.
    1.1. Call with not NULL enablePtr.
         Expected: GT_OK.
    1.1. Call with NULL enablePtr.
         Expected: GT_BAD_PTR.
    2. Go over all non-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st;
    GT_U8      dev;
    GT_BOOL    enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssPxPortTxDebugGlobalQueueTxEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with NULL enablePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxDebugGlobalQueueTxEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxDebugGlobalQueueTxEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*
        3. Call function with out of bound value for device id.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxDebugGlobalQueueTxEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
    Configuration of cpssPxPortTxDebug suit
*/
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortTxDebug)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugQueueingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugQueueingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugQueueTxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugQueueTxEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugResourceHistogramThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugResourceHistogramThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugResourceHistogramCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugGlobalDescLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugGlobalDescLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugGlobalQueueTxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxDebugGlobalQueueTxEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortTxDebug)
