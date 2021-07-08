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
* @file cpssPxNetIfUT.c
*
* @brief Unit tests for cpssPxNetIf, that provides
* Include PX network interface API functions
*
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define NETIF_VALID_PHY_PORT_CNS    0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffs[],
    OUT GT_U32                              buffLen[]
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaRxPacketGet)
{
/*
    ITERATE_DEVICE (All Px devices exclude Px1_Diamond)
    1.1. Call with queueIdx[7], numOfBuff[100], not NULL packetBuffs, buffLen.
    Expected: GT_OK.
    1.2. Call with out of range queueIdx[8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with numOfBuffPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with packetBuffs[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with buffLen[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                               queueIdx  = 0;
    GT_U32                              numOfBuff = 0;
    GT_U8                               packetBuffs[100];
    GT_U32                              buffLen[100];


    cpssOsBzero((GT_VOID*) packetBuffs, sizeof(packetBuffs));
    cpssOsBzero((GT_VOID*) buffLen, sizeof(buffLen));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with queueIdx[7], numOfBuff[100], not NULL packetBuffs, buffLen
            Expected: GT_OK.
        */
        queueIdx  = 7;
        numOfBuff = 100;

        st = cpssPxNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen);
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_STATE, st, dev, queueIdx, numOfBuff);
            continue;
        }
        st = (GT_NO_MORE == st) ? GT_OK : st;
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queueIdx, numOfBuff);

        /*
            1.2. Call with out of range queueIdx[8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        queueIdx = 8;

        st = cpssPxNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIdx);

        queueIdx = 0;

        /*
            1.3. Call with numOfBuffPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaRxPacketGet(dev, queueIdx, NULL, (GT_U8 **)packetBuffs, buffLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfBuffPtr = NULL", dev);

        /*
            1.4. Call with packetBuffs[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, NULL, buffLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetBuffsPtr = NULL", dev);

        /*
            1.5. Call with buffLen[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffLenPtr = NULL", dev);
    }

    queueIdx  = 7;
    numOfBuff = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaRxCountersGet
(
    IN    GT_U8      devNum,
    IN    GT_U8      queueIdx,
    OUT   CPSS_PX_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaRxCountersGet)
{
/*
    ITERATE_DEVICE (All Px devices exclude Px1_Diamond)
    1.1. Call with queueIdx[7]
                   and not NULL rxCountersPtr.
    Expected: GT_OK.
    1.2. Call with out of range queueIdx[8]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with rxCountersPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8                              queueIdx = 0;
    CPSS_PX_NET_SDMA_RX_COUNTERS_STC rxCounters;


    cpssOsBzero((GT_VOID*) &rxCounters, sizeof(rxCounters));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with queueIdx[7]
                           and not NULL rxCountersPtr.
            Expected: GT_OK.
        */
        queueIdx = 7;

        st = cpssPxNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, queueIdx);
            continue;
        }

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIdx);

        /*
            1.2. Call with out of range queueIdx[8]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        queueIdx = 8;

        st = cpssPxNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIdx);

        queueIdx = 0;

        /*
            1.3. Call with rxCountersPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaRxCountersGet(dev, queueIdx, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rxCountersPtr = NULL", dev);
    }

    queueIdx = 7;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaRxErrorCountGet
(
    IN GT_U8                                    devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC     *rxErrCountPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaRxErrorCountGet)
{
/*
    ITERATE_DEVICE (All Px devices exclude Px1_Diamond)
    1.1. Call with not NULL rxErrCountPtr;
    Expected: GT_OK.
    1.2. Call with rxErrCountPtr [NULL];
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC rxErrCount;


    cpssOsBzero((GT_VOID*) &rxErrCount, sizeof(rxErrCount));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL rxErrCountPtr;
            Expected: GT_OK.
        */
        st = cpssPxNetIfSdmaRxErrorCountGet(dev, &rxErrCount);
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
            continue;
        }

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with rxErrCountPtr [NULL];
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaRxErrorCountGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rxErrCountPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaRxErrorCountGet(dev, &rxErrCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaRxErrorCountGet(dev, &rxErrCount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaRxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaRxQueueEnable)
{
/*
    ITERATE_DEVICE (All Px devices exclude Px1_Diamond)
    1.1. Call with queue [0 / 7],
                   enable [GT_FALSE / GT_TRUE];
    Expected: GT_OK.
    1.2. Call cpssPxNetIfSdmaRxQueueEnableGet
    Expected: GT_OK and same enablePtr.
    1.3. Call with out of range queue [8],
                   and other params from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8       queue  = 0;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        /* iterate with queue = 0 */
        queue  = 0;
        enable = GT_FALSE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        /* iterate with queue = 3 */
        queue  = 3;
        enable = GT_FALSE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        /* iterate with queue = 7 */
        queue  = 7;
        enable = GT_TRUE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.3.  */
        queue = 8;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);
    }

    queue  = 0;
    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaRxQueueEnableGet)
{
/*
    ITERATE_DEVICE (All Px devices exclude Px1_Diamond)
    1.1. Call with not NULL queuePtr and enablePtr;
    Expected: GT_OK.
    1.2. Call with out of range queue.
    Expected: NOT GT_OK.
    1.3. Call with NULL  enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8       queue  = 0;
    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        queue = 0;
        enable = GT_TRUE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue = 3;
        enable = GT_FALSE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue = 7;
        enable = GT_TRUE;

        st = cpssPxNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        queue = 8;
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        queue = 0;

        /*  1.3.  */
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaTxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaTxQueueEnable)
{
/*
    ITERATE_DEVICE (All Px devices exclude Px1_Diamond)
    1.1. Call with queue [0 - 7], enable [GT_FALSE / GT_TRUE];
    Expected: GT_OK.
    1.2. Call cpssPxNetIfSdmaTxQueueEnableGet
    Expected: GT_OK and same queuePtr and enablePtr.
    1.3. Call with out of range queue [8], and other params from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8       queue  = 0;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* skip GM from the test to avoid stack overflow */
        if(prvUtfIsGmCompilation())
        {
            continue;
        }
        /*  1.1.  */
        enable = GT_TRUE;

        for(queue = 0; queue < 8; queue++)
        {
            /* skip Traffic Generator Queue to avoid traffic injection */
            if ((queue == 3) || (queue  == 6))
            {
                continue;
            }

            st = cpssPxNetIfSdmaTxQueueEnable(dev, queue, enable);
            if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
                continue;
            }

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

            /*  The Tx SDMA is not configured.
                The PP identify error and disables TX SDMA.  */
        }

        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
        {
            continue;
        }

        enable = GT_FALSE;

        for(queue = 0; queue < 8; queue++)
        {
            st = cpssPxNetIfSdmaTxQueueEnable(dev, queue, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

            cpssOsTimerWkAfter(10);

            /*  1.1.1  */
            st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);
        }

        /*
            1.2. Call with out of range queue [8],
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        queue = 8;

        st = cpssPxNetIfSdmaTxQueueEnable(dev, queue, enable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);
    }

    queue  = 0;
    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaTxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaTxQueueEnable(dev, queue, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaTxQueueEnableGet)
{
/*
    ITERATE_DEVICE (All Px devices exclude Px1_Diamond)
    1.1. Call with not NULL queuePtr and enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range queue [8]
    Expected: NOT GT_OK.
    1.3. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8       queue  = 0;
    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        queue  = 0;
        st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
            continue;
        }

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue  = 3;
        st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue  = 7;
        st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        queue  = 8;
        st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        queue  = 0;

        /*  1.3.  */
        st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    queue  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaRxResourceErrorModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   queue,
    IN  CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaRxResourceErrorModeSet)
{
/*
    ITERATE_DEVICES (Pxx)
    1.1. Call with queue[0 / 7],
                   mode [CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E / CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E].
    Expected: GT_OK.
    1.2. Call cpssPxNetIfSdmaRxResourceErrorModeGet.
    Expected: GT_OK and same modePtr.
    1.3. Call with queue[8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with mode [wrong enum values] and other params from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8 queue = 0;
    CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT mode;
    CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT modeGet;

    mode = CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E;
    modeGet = CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with queue[0 / 7],  PRV_CPSS_PX3_SDMA_WA_E
                           mode [CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E / CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E].
            Expected: GT_OK.
        */
        /* iterate with queue = 0 */
        queue = 0;
        mode = CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E;

        st = cpssPxNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);

        /*  1.2.  */
        st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, &modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        /*  Verify values  */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "got another mode than was set: %d", dev);

        /* iterate with queue = 7 */
        queue = 7;
        mode = CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E;

        st = cpssPxNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);

        /*  1.2.  */
        st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, &modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        /*  Verify values  */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "got another mode than was set: %d", dev);

        /*
            1.3. Call with queue[8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        queue = 8;

        st = cpssPxNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);

        queue = 0;

        /*
            1.4. Call with mode [wrong enum values] and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxNetIfSdmaRxResourceErrorModeSet
                            (dev, queue, mode),
                            mode);
    }

    queue = 0;
    mode = CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaRxResourceErrorModeGet)
{
/*
    ITERATE_DEVICES (Pxx)
    1.1. Call with queue[0 / 7].
    Expected: GT_OK.
    1.2. Call with queue[8].
    Expected: NOT GT_OK.
    1.3. Call with NULL modePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8 queue = 0;
    CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT mode = CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        /* iterate with queue = 0 */
        queue = 0;

        st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        /* iterate with queue = 7 */
        queue = 7;

        st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        /*  1.2.  */
        queue = 8;

        st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        queue = 0;

        /*  1.3.  */
        st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, queue);
    }

    queue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfRxBufFree
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfRxBufFree)
{
/*
    ITERATE_DEVICES (Pxx)
    1.1. Call with rxQueue [0], buffListLen [5].
    Expected: GT_OK.
    1.2. Call with wrong rxBuffList pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U8       rxQueue = 0;
    GT_U8      *rxBuffList[5];
    GT_U8       rxBuffer[10];
    GT_U32      buffListLen = 5;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* connect buffers to the list
       use same buffer */
    rxBuffList[0] = rxBuffList[1] = rxBuffList[2] =
    rxBuffList[3] = rxBuffList[4] = rxBuffer;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with rxQueue [0], buffListLen [5].
            Expected: GT_OK.
        */
        #if 0
        /* Only buffers that recived by cpssPxNetIfSdmaRxPacketGet can be free*/
        st = cpssPxNetIfRxBufFree(dev, rxQueue, rxBuffList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        #endif
        /*
            1.2. Call with wrong rxBuffList pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxNetIfRxBufFree(dev, rxQueue, NULL, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfRxBufFree(dev, rxQueue, rxBuffList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfRxBufFree(dev, rxQueue, rxBuffList, buffListLen);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfRxBufFreeWithSize
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfRxBufFreeWithSize)
{
/*
    ITERATE_DEVICES (Pxx)
    1.1. Call with rxQueue [0], buffListLen [5] and rxBuffSizeList[5].
    Expected: GT_OK.
    1.2. Call with wrong rxBuffList pointer [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with wrong rxBuffSizeList pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U8       rxQueue = 0;
    GT_U8       *rxBuffList[5];
    GT_U32      rxBuffSizeList[5];
    GT_U32      buffListLen = 5;
    GT_U8       rxBuffer[10] = {0,1,2,3,4,5,6,7,8,9};
    GT_U8       rxBuffer1[9] = {10,11,12,13,14,15,16,17,18};
    GT_U8       rxBuffer2[8] = {20,21,22,23,24,25,26,27};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* connect buffers to the list
       use same buffer */
    rxBuffList[0] = rxBuffList[3] = rxBuffer;
    rxBuffList[1] = rxBuffList[4] = rxBuffer1;
    rxBuffList[2] = rxBuffer2;

    rxBuffSizeList[0] = rxBuffSizeList[3] = sizeof(rxBuffer);
    rxBuffSizeList[1] = rxBuffSizeList[4] = sizeof(rxBuffer1);
    rxBuffSizeList[2] = sizeof(rxBuffer2);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with rxQueue [0], buffListLen [5] and rxBuffSizeList[5].
            Expected: GT_OK.
        */

        st = cpssPxNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
                                            rxBuffSizeList, buffListLen);
        if(PRV_CPSS_PX_PP_MAC(dev)->netIf.allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
        {
            if(PRV_CPSS_PP_MAC(dev)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
            }
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);
        }

        /*
            1.2. Call with wrong rxBuffList pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxNetIfRxBufFreeWithSize(dev, rxQueue, NULL,
                                            rxBuffSizeList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.2. Call with wrong rxBuffSizeList pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
                                            NULL, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
                                            rxBuffSizeList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
                                        rxBuffSizeList, buffListLen);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaSyncTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_PX_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaSyncTxPacketSend)
{
/*
    ITERATE_DEVICES (Pxx)
    ======= DSA_CMD_TO_CPU =======
    1.1. Call with pcktParamsPtr{
                      packetIsTagged [GT_TRUE],
                      cookie [],
                      sdmaInfo {recalcCrc [GT_TRUE],
                                txQueue [0],
                                invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                      dsaParam {commonParams {dsaTagType [CPSS_PX_NET_DSA_TYPE_REGULAR_E],
                                              vpt[0],
                                              cfiBit [0],
                                              vid [0],
                                              dropOnSource [GT_FALSE],
                                              packetIsLooped [GT_FALSE]}
                                dsaType [CPSS_PX_NET_DSA_CMD_TO_CPU_E],
                                dsaInfo.toCpu {isEgressPipe [GT_TRUE],
                                               isTagged [GT_TRUE],
                                               devNum [dev],
                                               srcIsTrunk [GT_TRUE],

                                               interface.srcTrunkId [2],

                                               cpuCode [CPSS_NET_CONTROL_E],
                                               wasTruncated [GT_TRUE],
                                               originByteCount [0],
                                               timestamp [0],

                                               packetIsTT[GT_FALSE](SIP5),
                                               flowIdTtOffset{flowId[0]}(SIP5),
                                               tag0TpidIndex[0](SIP5) }}},
                       packetBuffsArr[{0}],
                       buffLenArr[sizeof(packetBuffsArr)],
                       numOfBufs [1]
    Expected: GT_OK.
    1.2. Call with out of range pcktParamsPtr->cookie [NULL]
        and other valid params same as 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with out of range pcktParamsPtr->sdmaInfo.txQueue [8]
        and other valid params same as 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range pcktParamsPtr->dsaParam.commonParams.dsaTagType [wrong enum values]
        and other valid params same as 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range pcktParamsPtr->dsaParam.commonParams.vpt [8]
        and other valid params same as 1.1.
    Expected: NOT GT_OK.
    1.6. Call with out of range pcktParamsPtr->dsaParam.commonParams.cfiBit [2]
        and other valid params same as 1.1.
    Expected: NOT GT_OK.
    1.7. Call with out of range pcktParamsPtr->dsaParam.commonParams.vid [4096]
        and other valid params same as 1.1.
    Expected: GT_BAD_PARAM.
    1.8. Call with out of range pcktParamsPtr->dsaParam.dsaType [wrong enum values]
        and other valid params same as 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call with out of range pcktParamsPtr->dsaParam.toCpu.hwDevNum [128]
        and other valid params same as 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call with out of range pcktParamsPtr->dsaParam.toCpu.srcTrunkId [128]
        and other valid params same as 1.1.
    Expected: NOT GT_OK.
    1.11. Call with out of range pcktParamsPtr->dsaParam.toCpu.cpuCode [wrong enum values]
        and other valid params same as 1.1.
    Expected: GT_BAD_PARAM.
    1.12. Call with pcktParams->dsaParam.dsaInfo.toCpu.packetIsTT[GT_TRUE]
        and other valid params same as 1.1.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.13. Call with pcktParams->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[0xA5A5]
        and other valid params same as 1.1.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.14. Call with pcktParams->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[BIT_20]
        and other valid params same as 1.1.
    Expected: NOT GT_OK.
    1.15. Call with pcktParams->dsaParam.dsaInfo.toCpu.tag0TpidIndex[7]
        and other valid params same as 1.1.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.16. Call with pcktParams->dsaParam.dsaInfo.toCpu.tag0TpidIndex[BIT_3]
        and other valid params same as 1.1.
    Expected: NOT GT_OK.

    ======= DSA_CMD_FROM_CPU =======
    1.17. Call with pcktParamsPtr {
                      packetIsTagged [GT_FALSE],
                      cookie [],
                      sdmaInfo {recalcCrc [GT_FALSE],
                                txQueue [3],
                                invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                      dsaParam {commonParams {dsaTagType [CPSS_PX_NET_DSA_TYPE_EXTENDED_E],
                                              vpt[3],
                                              cfiBit [1],
                                              vid [100],
                                              dropOnSource [GT_TRUE],
                                              packetIsLooped [GT_TRUE]}
                                dsaType [CPSS_PX_NET_DSA_CMD_FROM_CPU_E],
                                dsaInfo.fromCpu {dstInterface.type [CPSS_INTERFACE_PORT_E],
                                                 tc [7],
                                                 dp [CPSS_DP_GREEN_E],
                                                 egrFilterEn [GT_TRUE],
                                                 cascadeControl [GT_TRUE],
                                                 egrFilterRegistered [GT_TRUE],
                                                 srcId [0],
                                                 srcHwDev [dev],
                                                 extDestInfo.devPort{
                                                     dstIsTagged [GT_TRUE],
                                                     mailBoxToNeighborCPU [GT_TRUE]},
                                                 isTrgPhyPortValid[GT_FALSE](SIP5),
                                                 dstEport[0](SIP5),
                                                 tag0TpidIndex[0]()SIP5}},
                      packetBuffsArr[{0}],
                      buffLenArr[sizeof(packetBuffsArr)],
                      numOfBufs [1].
    Expected: GT_OK.

    1.18. Call with pcktParams->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid[GT_TRUE]
        and other valid params same as 1.17.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.19. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid[GT_TRUE],
        out of range pcktParams->dsaParam.dsaInfo.fromCpu.dstEport[BIT_17]
        and other valid params same as 1.17.
    Expected: NOT GT_OK.
    1.20. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid[GT_TRUE],
        out of range pcktParams->dsaParam.dsaInfo.fromCpu.tag0TpidIndex[BIT_3]
        and other valid params same as 1.17.
    Expected: NOT GT_OK.
    1.21. Call with pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VID_E],
        pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.vid[100](same as commonParams.vid),
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface[GT_TRUE],
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type[CPSS_INTERFACE_TRUNK_E],
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId[2],
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged[CPSS_PX_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E]
        and other valid params same as 1.17.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.22. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VID_E],
        pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.vid[100](same as commonParams.vid),
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludeInterface[GT_TRUE],
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludedInterface.type[CPSS_INTERFACE_TRUNK_E],
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludedInterface.trunkId[2],
        out of range enum value pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multidest.srcIsTagged
        and other valid params same as 1.17.
    Expected: GT_BAD_PARAM.
    1.23. Call with out of range pcktParamsPtr->dsaParam.fromCpu.dstInterface.type [wrong enum values]
        and other valid params same as 1.17.
    Expected: GT_BAD_PARAM.
    1.24. Call with out of range pcktParamsPtr->dsaParam.fromCpu.tc [8]
        and other valid params same as 1.17.
    Expected: NOT GT_OK.
    1.25. Call with out of range pcktParamsPtr->dsaParam.fromCpu.dp [wrong enum values]
        and other valid params same as 1.17.
    Expected: GT_BAD_PARAM.
    1.26. Call with out of range pcktParamsPtr->dsaParam.fromCpu.srcHwDev [128]
        and other valid params same as 1.17.
    Expected: GT_BAD_PARAM.

    ======= DSA_CMD_TO_ANALYZER =======
    1.27. Call with pcktParamsPtr {packetIsTagged [GT_TRUE],
                                  cookie [],
                                  sdmaInfo {recalcCrc [GT_FALSE],
                                            txQueue [5],
                                            invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                                  dsaParam {commonParams {dsaTagType [CPSS_PX_NET_DSA_TYPE_REGULAR_E],
                                                          vpt[5],
                                                          cfiBit [0],
                                                          vid [1000],
                                                          dropOnSource [GT_FALSE],
                                                          packetIsLooped [GT_TRUE]}
                                            dsaType [CPSS_PX_NET_DSA_CMD_TO_ANALYZER_E],
                                            dsaInfo.toAnalyzer {rxSniffer [GT_TRUE],
                                                                isTagged [GT_TRUE],
                                                                devPort.hwDevNum [dev],
                                                                devPort.portNum [0]}}},
                      packetBuffsArr[{0}],
                      buffLenArr[sizeof(packetBuffsArr)],
                      numOfBufs [1].
    Expected: GT_OK.
    1.28. Call with out of range pcktParamsPtr->dsaParam.toAnalyzer.devPort.hwDevNum [128]
        and other valid params same as 1.27.
    Expected: GT_BAD_PARAM.
    1.29. Call with out of range pcktParamsPtr->dsaParam.toAnalyzer.devPort.portNum [64]
        and other valid params same as 1.27.
    Expected: GT_BAD_PARAM.
    1.30. Call with pcktParams->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[7]
        and other valid params same as 1.27.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.31. Call with pcktParams->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[BIT_3]
        and other valid params same as 1.27.
    Expected: NOT GT_OK.

    ======= DSA_CMD_FORWARD =======
    1.32. Call with pcktParamsPtr {packetIsTagged [GT_FALSE],
                                  cookie [],
                                  sdmaInfo {recalcCrc [GT_TRUE],
                                            txQueue [7],
                                            invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                                  dsaParam {commonParams {dsaTagType [CPSS_PX_NET_DSA_TYPE_EXTENDED_E],
                                                          vpt[7],
                                                          cfiBit [1],
                                                          vid [0xFFF],
                                                          dropOnSource [GT_TRUE],
                                                          packetIsLooped [GT_FALSE]}
                                            dsaType [CPSS_PX_NET_DSA_CMD_FORWARD_E],
                                            dsaInfo.forward {srcIsTagged [CPSS_PX_BRG_VLAN_PORT_TAG0_CMD_E],
                                                             srcHwDev [dev]
                                                             srcIsTrunk [GT_TRUE],
                                                             source.trunkId [2],
                                                             srcId [0],
                                                             egrFilterRegistered [GT_TRUE],
                                                             wasRouted [GT_TRUE],
                                                             qosProfileIndex [0],
                                                             dstInterface.type[CPSS_INTERFACE_VID_E],
                                                             dstInterface.vlanId[0xFFF]}}},
                      packetBuffsArr[{0}],
                      buffLenArr[sizeof(packetBuffsArr)],
                      numOfBufs [1].
    Expected: GT_OK.
    1.33. Call with pcktParamsPtr->dsaParam.dsaInfo.forward.dstInterface.srcIsTagged[CPSS_PX_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E]
        and other valid params same as 1.32.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.34. Call with out of range enum value
        pcktParamsPtr->dsaParam.dsaInfo.forward.dstInterface.srcIsTagged
        and other valid params same as 1.32.
    Expected: GT_BAD_PARAM.
    1.35. Call with out of range pcktParamsPtr->dsaParam.forward.srcHwDev [128]
        and other valid params same as 1.32.
    Expected: GT_BAD_PARAM.
    1.36. Call with out of range pcktParamsPtr->dsaParam.dsaInfo.forward.source.trunkId [128]
        and other valid params same as 1.32.
    Expected: NOT GT_OK.
    1.37. Call with out of range pcktParamsPtr->dsaParam.dsaInfo.forward.dstInterface.type [wrong enum values]
        and other valid params same as 1.32.
    Expected: GT_BAD_PARAM.
    1.38. Call with pcktParams->dsaParam.dsaInfo.forward.tag0TpidIndex[7]
        and other valid params same as 1.32.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.39. Call with pcktParams->dsaParam.dsaInfo.forward.tag0TpidIndex[BIT_3]
        and other valid params same as 1.32.
    Expected: NOT GT_OK.
    1.40. Call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_TRUE]
        and other valid params same as 1.32.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.41. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_FALSE],
        out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.trunkId[4096](not relevant)
        and other valid params same as 1.32.
    Expected: GT_OK.
    1.42. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_FALSE],
        out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096](not relevant)
        and other valid params same as 1.32.
    Expected: GT_OK.
    1.43. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_TRUE],
        out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.trunkId[4096](is relevant)
        and other valid params same as 1.32.
    Expected: NOT GT_OK.
    1.44. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_TRUE],
        out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096](is relevant)
        and other valid params same as 1.32.
    Expected: NOT GT_OK.
    1.45. Call with pcktParams->dsaParam.dsaInfo.forward.phySrcMcFilterEnable[GT_TRUE]
        and other valid params same as 1.32.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3 and NOT GT_OK for others.
    1.46. Call with pcktParamsPtr [NULL] and other valid params same as 1.1.
    Expected: GT_BAD_PTR.
    1.47. Call with packetBuffsArrPtr [NULL] and other valid params same as 1.1.
    Expected: GT_BAD_PTR.
    1.48. Call with buffLenArr [NULL] and other valid params same as 1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st                      = GT_OK;
    GT_U8                       dev                     = 0;
    CPSS_PX_NET_TX_PARAMS_STC pcktParams;
    GT_U8                       *buffList[1];
    GT_U32                      buffLenList[1];
    GT_U32                      numOfBufs               = 1;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily               = 0;
    GT_U32                      byteIndexForQueueId = 14;/* byte index in packetHeader[] to hold the queueId */
    GT_U8                       packetHeader[] =
    {0x07, 0x17, 0x27, 0x37, 0x47, 0x57, /* mac DA */
     0x08, 0x91, 0x92, 0x93, 0x94, 0x95, /* mac SA */
     0xaa, 0xbb, 0x00, 0xdd,             /* set the queueId in to byte : 0x00 (see byteIndexForQueueId)*/
     0xee, 0xff, 0x11, 0x22,
     0xaa, 0xaa, 0xaa, 0xaa,
     0xbb, 0xbb, 0xbb, 0xbb,
     0xcc, 0xcc, 0xcc, 0xcc,
     0xdd, 0xdd, 0xdd, 0xdd,
     };
    GT_U32                      txQueue;

    buffList[0] = cpssOsCacheDmaMalloc(64);
    buffLenList[0] = 64;
    cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
    cpssOsBzero((GT_VOID*) buffList[0], 64);
    cpssOsMemCpy(buffList[0], packetHeader, sizeof(packetHeader));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            ITERATE_DEVICES (Pxx)
        */
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        /*
            1.3. Call with out of range pcktParamsPtr->sdmaInfo.txQueue [8]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        pcktParams.txQueue = 8;

        st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->txQueue = %d",
                                         dev, pcktParams.txQueue);

        /* skip MII and not connected to CPU devices */
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
            continue;

        for(txQueue = 0 ;txQueue < CPSS_TC_RANGE_CNS; txQueue++)
        {
            buffList[0][byteIndexForQueueId] = (GT_U8)(0xFF - txQueue);

            pcktParams.txQueue = txQueue;
            st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->txQueue = %d",
                    dev, pcktParams.txQueue);
        }

        /*
            1.46. Call with pcktParamsPtr [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaSyncTxPacketSend(dev, NULL, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pcktParamsPtr = NULL", dev);

        /*
            1.47. Call with buffList [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, NULL,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffList = NULL", dev);

        /*
            1.48. Call with buffLenList [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              NULL, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffLenList = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                               buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                           buffLenList, numOfBufs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    cpssOsCacheDmaFree(buffList[0]);

#ifdef ASIC_SIMULATION
    /* allow all packets to egress the devices */
    cpssOsTimerWkAfter(500);
#endif /*ASIC_SIMULATION*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxNetIfSdmaTxFreeDescripotrsNumberGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_U32             *numberOfFreeTxDescriptorsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxNetIfSdmaTxFreeDescripotrsNumberGet)
{
/*
    ITERATE_DEVICES (Pxx)
    1.1. Call with txQueue [0].
    Expected: GT_OK.
    1.2. Call with wrong txQueue [8].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong numberOfFreeTxDescriptorsPtr pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U8       txQueue = 0;
    GT_U32      numberOfFreeTxDescriptors;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with txQueue [0].
            Expected: GT_OK.
        */

        st = cpssPxNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue,
                                                        &numberOfFreeTxDescriptors);
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
            continue;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong txQueue [8].
            Expected: GT_BAD_PARAM.
        */

        st = cpssPxNetIfSdmaTxFreeDescripotrsNumberGet(dev, 8,
                                                        &numberOfFreeTxDescriptors);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call with wrong numberOfFreeTxDescriptorsPtr pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue,
                                                        &numberOfFreeTxDescriptors);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue,
                                                    &numberOfFreeTxDescriptors);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxNetIf suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxNetIf)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaRxPacketGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaRxCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaRxErrorCountGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaRxQueueEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaRxQueueEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaTxQueueEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaTxQueueEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaRxResourceErrorModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaRxResourceErrorModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfRxBufFree)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfRxBufFreeWithSize)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaTxFreeDescripotrsNumberGet)

UTF_SUIT_END_TESTS_MAC(cpssPxNetIf)

/* the tests under cpssPxNetIf_packetSend are tests that may send packets , and
   need a 'correct' tables configurations.
   so those should run as 'first tests' after cpssInitSystem
     */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxNetIf_packetSend)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxNetIfSdmaSyncTxPacketSend)
UTF_SUIT_END_TESTS_MAC(cpssPxNetIf_packetSend)



