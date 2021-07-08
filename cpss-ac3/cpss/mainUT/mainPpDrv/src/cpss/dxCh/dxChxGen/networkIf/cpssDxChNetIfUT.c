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
* @file cpssDxChNetIfUT.c
*
* @brief Unit tests for cpssDxChNetIf, that provides
* Include DxCh network interface API functions
*
*
* @version   62
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define NETIF_VALID_PHY_PORT_CNS    0

/* Internal functions forward declaration */

/*  Set dsaInfoPtr->dsaInfo.toCpu structure.         */
static void prvSetDsaInfoToCpu      (OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr);

/*  Set dsaInfoPtr->dsaInfo.fromCpu structure.       */
static void prvSetDsaInfoFromCpu    (OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr);

/*  Set dsaInfoPtr->dsaInfo.toAnalizer structure.    */
static void prvSetDsaInfoToAnalizer (OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr);

/* Set dsaInfoPtr->dsaInfo.forward structure.        */
static void prvSetDsaInfoForward     (OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr);

/* This routine returns core clock per device.       */
static GT_STATUS prvUtfCoreClockGet  (IN GT_U8 dev, OUT GT_U32 *coreClock);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPrePendTwoBytesHeaderSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPrePendTwoBytesHeaderSet)
{
/*
    ITERATE_DEVICE (DXCHX)
    1.1. Call with enable[GT_FALSE / GT_TRUE]
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfPrePendTwoBytesHeaderGet with the same params.
    Expected: GT_OK and the same values.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE / GT_TRUE]
            Expected: GT_OK.
        */
        /* iterate with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChNetIfPrePendTwoBytesHeaderSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChNetIfPrePendTwoBytesHeaderGet with the same params.
            Expected: GT_OK and the same values.
        */
        st = cpssDxChNetIfPrePendTwoBytesHeaderGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                         "got another enable than was set %d", dev);

        /* iterate with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChNetIfPrePendTwoBytesHeaderSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChNetIfPrePendTwoBytesHeaderGet with the same params.
            Expected: GT_OK and the same values.
        */
        st = cpssDxChNetIfPrePendTwoBytesHeaderGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                         "got another enable than was set %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPrePendTwoBytesHeaderSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPrePendTwoBytesHeaderSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPrePendTwoBytesHeaderGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPrePendTwoBytesHeaderGet)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChNetIfPrePendTwoBytesHeaderGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL]
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfPrePendTwoBytesHeaderGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPrePendTwoBytesHeaderGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPrePendTwoBytesHeaderGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffs[],
    OUT GT_U32                              buffLen[],
    OUT CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaRxPacketGet)
{
/*
    ITERATE_DEVICE (All DxCh devices exclude DxCh1_Diamond)
    1.1. Call with queueIdx[7], numOfBuff[100], not NULL packetBuffs, buffLen, rxParamsPtr.
    Expected: GT_OK.
    1.2. Call with out of range queueIdx[8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with numOfBuffPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with packetBuffs[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with buffLen[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with rxParamsPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                               queueIdx  = 0;
    GT_U32                              numOfBuff = 0;
    GT_U8                               packetBuffs[100];
    GT_U32                              buffLen[100];
    CPSS_DXCH_NET_RX_PARAMS_STC         rxParams;
    GT_U32                              numOfNetIfs;

    cpssOsBzero((GT_VOID*) packetBuffs, sizeof(packetBuffs));
    cpssOsBzero((GT_VOID*) buffLen, sizeof(buffLen));
    cpssOsBzero((GT_VOID*) &rxParams, sizeof(rxParams));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.1. Call with queueIdx[7], numOfBuff[100], not NULL packetBuffs, buffLen, rxParamsPtr.
            Expected: GT_OK.
        */
        queueIdx  = 7;
        numOfBuff = 100;

        st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen, &rxParams);
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
        queueIdx = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen, &rxParams);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIdx);

        queueIdx = 0;

        /*
            1.3. Call with numOfBuffPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, NULL, (GT_U8 **)packetBuffs, buffLen, &rxParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfBuffPtr = NULL", dev);

        /*
            1.4. Call with packetBuffs[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, NULL, buffLen, &rxParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetBuffsPtr = NULL", dev);

        /*
            1.5. Call with buffLen[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, NULL, &rxParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffLenPtr = NULL", dev);

        /*
            1.6. Call with rxParamsPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rxParamsPtr = NULL", dev);
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
        st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen, &rxParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaRxPacketGet(dev, queueIdx, &numOfBuff, (GT_U8 **)packetBuffs, buffLen, &rxParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaRxCountersGet
(
    IN    GT_U8      devNum,
    IN    GT_U8      queueIdx,
    OUT   CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaRxCountersGet)
{
/*
    ITERATE_DEVICE (All DxCh devices exclude DxCh1_Diamond)
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
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC rxCounters;
    GT_U32                             numOfNetIfs;


    cpssOsBzero((GT_VOID*) &rxCounters, sizeof(rxCounters));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.1. Call with queueIdx[7]
                           and not NULL rxCountersPtr.
            Expected: GT_OK.
        */
        queueIdx = 7;

        st = cpssDxChNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
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
        queueIdx = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIdx);

        queueIdx = 0;

        /*
            1.3. Call with rxCountersPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaRxCountersGet(dev, queueIdx, NULL);
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
        st = cpssDxChNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaRxCountersGet(dev, queueIdx, &rxCounters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaRxErrorCountGet
(
    IN GT_U8                                    devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC     *rxErrCountPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaRxErrorCountGet)
{
/*
    ITERATE_DEVICE (All DxCh devices exclude DxCh1_Diamond)
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
        st = cpssDxChNetIfSdmaRxErrorCountGet(dev, &rxErrCount);
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
        st = cpssDxChNetIfSdmaRxErrorCountGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rxErrCountPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaRxErrorCountGet(dev, &rxErrCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaRxErrorCountGet(dev, &rxErrCount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaRxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaRxQueueEnable)
{
/*
    ITERATE_DEVICE (All DxCh devices exclude DxCh1_Diamond)
    1.1. Call with queue [0 / 7],
                   enable [GT_FALSE / GT_TRUE];
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfSdmaRxQueueEnableGet
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
    GT_U32      numOfNetIfs;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*  1.1.  */
        /* iterate with queue = 0 */
        queue  = 0;
        enable = GT_FALSE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        /* iterate with queue = 3 */
        queue  = 3;
        enable = GT_FALSE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        /* iterate with queue = 7 */
        queue  = 7;
        enable = GT_TRUE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /*  1.2  */
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.3.  */
        queue = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
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
        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaRxQueueEnableGet)
{
/*
    ITERATE_DEVICE (All DxCh devices exclude DxCh1_Diamond)
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
    GT_U32      numOfNetIfs;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*  1.1.  */
        queue = 0;
        enable = GT_TRUE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue = 3;
        enable = GT_FALSE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue = 7;
        enable = GT_TRUE;

        st = cpssDxChNetIfSdmaRxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        queue = (GT_U8)(8 * numOfNetIfs);
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        queue = 0;

        /*  1.3.  */
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaRxQueueEnableGet(dev, queue, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxQueueEnable)
{
/*
    ITERATE_DEVICE (All DxCh devices exclude DxCh1_Diamond)
    1.1. Call with queue [0 - 7], enable [GT_FALSE / GT_TRUE];
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfSdmaTxQueueEnableGet
    Expected: GT_OK and same queuePtr and enablePtr.
    1.3. Call with out of range queue [8], and other params from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8       queue  = 0;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      numOfNetIfs;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
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
            if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) || IS_XCAT3_DEV_MAC(dev))
            {
                if ((queue == 3) || (queue  == 6))
                {
                    continue;
                }
            }

            st = cpssDxChNetIfSdmaTxQueueEnable(dev, queue, enable);
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
            st = cpssDxChNetIfSdmaTxQueueEnable(dev, queue, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, enable);

            cpssOsTimerWkAfter(10);

            /*  1.1.1  */
            st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, &enableGet);
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
        queue = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaTxQueueEnable(dev, queue, enable);
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
        st = cpssDxChNetIfSdmaTxQueueEnable(dev, queue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaTxQueueEnable(dev, queue, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxQueueEnableGet)
{
/*
    ITERATE_DEVICE (All DxCh devices exclude DxCh1_Diamond)
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
    GT_U32      numOfNetIfs;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*  1.1.  */
        queue  = 0;
        st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
            continue;
        }

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue  = 3;
        st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        queue  = 7;
        st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        queue  = (GT_U8)(8 * numOfNetIfs);
        st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        queue  = 0;

        /*  1.3.  */
        st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, NULL);
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
        st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaTxQueueEnableGet(dev, queue, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfDsaTagBuild
(
    IN  GT_U8                          devNum,
    IN  CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr,
    OUT GT_U8                          *dsaBytesPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfDsaTagBuild)
{
/*
    ITERATE_DEVICE (DXCH)
    {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E}
    1.1. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                      CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                           vpt [0 / 7],
                                           cfiBit [0 / 1],
                                           vid [0 / 100],
                                           dropOnSource [GT_FAIL / GT_TRUE],
                                           packetIsLooped [GT_FAIL / GT_TRUE] }
                              dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E,
                              dsaInfo{toCpu {set to cpu}}
    Expected: GT_OK.
    1.2. Call with dsaInfoPtr->commonParams->dsaType[wrong enum values]and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range dsaInfoPtr->commonParams.vpt [8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range dsaInfoPtr->commonParams.cfiBit [2] and other params from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range dsaInfoPtr->commonParams.vid [4096] and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with dsaInfoPtr->dsaInfo.toCpu.hwDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS] and other params from 1.1.
    Expected: NOT GT_OK.
    1.8. Call with isEgressPipe [GT_FALSE],
                   srcIsTrunk[GT_FALSE],
                   dsaInfoPtr->dsaInfo.toCpu.interface.port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.8. Call with isEgressPipe [GT_FALSE],
                   srcIsTrunk[GT_TRUE],
                   dsaInfoPtr->dsaInfo.toCpu.interface.port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (not relevant)
                   and other params from 1.1.
    Expected: GT_OK.
    1.9. Call with dsaInfoPtr->dsaInfo.toCpu.cpuCode[wrong enum values] and other params from 1.1.
    Expected: NOT GT_OK.


    {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E}
    1.10. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                      CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                           vpt [0 / 7],
                                           cfiBit [0 / 1],
                                           vid [0 / 100],
                                           dropOnSource [GT_FAIL / GT_TRUE],
                                           packetIsLooped [GT_FAIL / GT_TRUE] }
                              dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E,
                              dsaInfo{fromCpu {set from cpu}}
    Expected: GT_OK.
    1.11. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_TRUNK_E] (not supported)
                    and other params from 1.10.
    Extended: NOT GT_OK.
    1.12. Call with wrong enum values dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type
                    and other params from 1.10.
    Extended: GT_BAD_PARAM.
    1.13. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_PORT_E]
                    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                    and other params from 1.10.
    Extended: NOT GT_OK.
    1.14. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_PORT_E]
                    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                    and other params from 1.10.
    Extended: NOT GT_OK.
    1.15. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (not relevant)
                    and other params from 1.10.
    Extended: GT_OK.
    1.16. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (not relevant)
                    and other params from 1.10.
    Extended: GT_OK.
    1.17. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vidx [4096] (out of range)
                    and other params from 1.10.
    Extended: NOT GT_OK.
    1.18. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VID_E]
                    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vidx [4096] (not relevant)
                    and other params from 1.10.
    Extended: GT_OK.
    1.21. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (not relevant)
                    and other params from 1.10.
    Extended: GT_OK.
    1.22. Call with dsaInfoPtr->dsaInfo.fromCpu.dp [wrong enum values]
                    and other params from 1.10.
    Extended: GT_BAD_PARAM.
    1.23. Call with dsaInfoPtr->dsaInfo.fromCpu.srcHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS]
                    and other params from 1.10.
    Extended: NOT GT_OK.


    {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E}
    1.24. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                      CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                           vpt [0 / 7],
                                           cfiBit [0 / 1],
                                           vid [0 / 100],
                                           dropOnSource [GT_FAIL / GT_TRUE],
                                           packetIsLooped [GT_FAIL / GT_TRUE] }
                              dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E,
                              dsaInfo{toAnalyzer {set to analyzer}}
    Expected: GT_OK.
    1.25 Call with dsaInfoPtr->dsaInfo.toAnalyzer.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                   and other params from 1.24
    Expected: NOT GT_OK.
    1.26 Call with dsaInfoPtr->dsaInfo.toAnalyzer.rx_sniff [GT_TRUE] (not extended rule)
                   dsaInfoPtr->dsaInfo.toAnalyzer.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                   and other params from 1.24
    Expected: NOT GT_OK.
    1.27 Call with dsaInfoPtr->dsaInfo.toAnalyzer.rx_sniff [GT_FALSE] (rxtended rule)
                   dsaInfoPtr->dsaInfo.toAnalyzer.devPort.portNum [31] (out of range)
                   and other params from 1.24
    Expected: NOT GT_OK.

    {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E}
    1.28. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                      CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                           vpt [0 / 7],
                                           cfiBit [0 / 1],
                                           vid [0 / 100],
                                           dropOnSource [GT_FAIL / GT_TRUE],
                                           packetIsLooped [GT_FAIL / GT_TRUE] }
                              dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E,
                              dsaInfo{forward {set forward}}
    Expected: GT_OK.
    1.29 Call with dsaInfoPtr->dsaInfo.forward.srcHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                   and other params from 1.28.
    Expected: NOT GT_OK.
    1.30 Call with dsaInfoPtr->dsaInfo.forward.srcIsTrunk [GT_FALSE]
                   dsaInfoPtr->dsaInfo.forward.source.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                   and other params from 1.28.
    Expected: NOT GT_OK.
    1.30 Call with dsaInfoPtr->dsaInfo.forward.srcIsTrunk [GT_TRUE]
                   dsaInfoPtr->dsaInfo.forward.source.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (not relevant)
                   and other params from 1.28.
    Expected: GT_OK.
    1.31 Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type [CPSS_INTERFACE_TRUNK_E] (not supported)
                   and other params from 1.28.
    Expected: NOT GT_OK.
    1.32 Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type [wrong enum values] (out of range)
                   and other params from 1.28.
    Expected: GT_BAD_PARAM.

    1.33. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E]
                    dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                    and other params from 1.28.
    Extended: NOT GT_OK.
    1.34. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E]
                    dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                    and other params from 1.28.
    Extended: NOT GT_OK.
    1.35. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (not relevant)
                    and other params from 1.28.
    Extended: GT_OK.
    1.36. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (not relevant)
                    and other params from 1.28.
    Extended: GT_OK.
    1.37. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.forward.dstInterface.vidx [4096] (out of range)
                    and other params from 1.28.
    Extended: NOT GT_OK.
    1.38. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VID_E]
                    dsaInfoPtr->dsaInfo.forward.dstInterface.vidx [4096] (not relevant)
                    and other params from 1.28.
    Extended: GT_OK.
    1.41. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                    dsaInfoPtr->dsaInfo.forward.dstInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (not relevant)
                    and other params from 1.28.
    Extended: GT_OK.

    1.42 Call with dsaInfoPtr[NULL]
                   and other params from 1.28.
    Expected: GT_BAD_PTR.
    1.43 Call with dsaBytesPtr[NULL]
                   and other params from 1.28.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U32      index = 0;
    GT_U8       dev;

    CPSS_DXCH_NET_DSA_PARAMS_STC    dsaInfo;
    GT_U8                           dsaBytes[16];

    cpssOsBzero((GT_VOID*) &dsaInfo, sizeof(dsaInfo));
    cpssOsBzero((GT_VOID*) dsaBytes, sizeof(dsaBytes));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E}
            1.1. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                              CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                   vpt [0 / 7],
                                                   cfiBit [0 / 1],
                                                   vid [0 / 100],
                                                   dropOnSource [GT_FAIL / GT_TRUE],
                                                   packetIsLooped [GT_FAIL / GT_TRUE] }
                                      dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E,
                                      dsaInfo{toCpu {set to cpu}}
            Expected: GT_OK.
        */
        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;

        dsaInfo.commonParams.vpt    = 0;
        dsaInfo.commonParams.cfiBit = 0;
        dsaInfo.commonParams.vid    = 0;

        dsaInfo.commonParams.dropOnSource   = GT_FALSE;
        dsaInfo.commonParams.packetIsLooped = GT_FALSE;

        dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;

        prvSetDsaInfoToCpu(&dsaInfo);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;

        dsaInfo.commonParams.vpt    = 7;
        dsaInfo.commonParams.cfiBit = 1;
        dsaInfo.commonParams.vid    = 100;

        dsaInfo.commonParams.dropOnSource   = GT_TRUE;
        dsaInfo.commonParams.packetIsLooped = GT_TRUE;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with dsaInfoPtr->commonParams.dsaType[wrong enum values]
                 and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfDsaTagBuild
                            (dev, &dsaInfo, dsaBytes),
                            dsaInfo.commonParams.dsaTagType);

        /*
            1.3. Call with out of range dsaInfoPtr->commonParams.vpt [8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        dsaInfo.commonParams.vpt = 8;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->commonParams.vpt = %d",
                                         dev, dsaInfo.commonParams.vpt);

        dsaInfo.commonParams.vpt = 7;

        /*
            1.4. Call with out of range dsaInfoPtr->commonParams.cfiBit [2] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        dsaInfo.commonParams.cfiBit = 2;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->commonParams.cfiBit = %d",
                                         dev, dsaInfo.commonParams.cfiBit);

        dsaInfo.commonParams.cfiBit = 1;

        /*
            1.5. Call with out of range dsaInfoPtr->commonParams.vid [4096] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        dsaInfo.commonParams.vid = 4096;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->commonParams.vid = %d",
                                         dev, dsaInfo.commonParams.vid);

        dsaInfo.commonParams.vid = 1;

        /*
            1.6. Call with dsaInfoPtr->dsaInfo.toCpu.hwDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        dsaInfo.dsaInfo.toCpu.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.toCpu.hwDevNum = %d",
                                         dev, dsaInfo.dsaInfo.toCpu.hwDevNum);

        dsaInfo.dsaInfo.toCpu.hwDevNum = 0;

        /*
            1.8. Call with isEgressPipe [GT_FALSE],
                           srcIsTrunk[GT_FALSE],
                           dsaInfoPtr->dsaInfo.toCpu.interface.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        dsaInfo.dsaInfo.toCpu.isEgressPipe      = GT_FALSE;
        dsaInfo.dsaInfo.toCpu.srcIsTrunk        = GT_FALSE;
        dsaInfo.dsaInfo.toCpu.interface.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->.dsaInfo.toCpu.interface.portNum = %d",
                                         dev, dsaInfo.dsaInfo.toCpu.interface.portNum);

        dsaInfo.dsaInfo.toCpu.interface.portNum = NETIF_VALID_PHY_PORT_CNS;

        /*
            1.8. Call with isEgressPipe [GT_FALSE],
                           srcIsTrunk[GT_TRUE],
                           dsaInfo.dsaInfo.toCpu.interface.srcTrunkId [maximal value]
                           and other params from 1.1.
            Expected: GT_OK.
        */
        dsaInfo.dsaInfo.toCpu.isEgressPipe      = GT_FALSE;
        dsaInfo.dsaInfo.toCpu.srcIsTrunk        = GT_TRUE;
        dsaInfo.dsaInfo.toCpu.interface.srcTrunkId = BIT_7 - 1;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfo.dsaInfo.toCpu.interface.srcTrunkId = %d",
                                     dev, dsaInfo.dsaInfo.toCpu.interface.srcTrunkId);

        dsaInfo.dsaInfo.toCpu.srcIsTrunk        = GT_FALSE;
        dsaInfo.dsaInfo.toCpu.interface.portNum = NETIF_VALID_PHY_PORT_CNS;

        /*
            1.9. Call with dsaInfoPtr->dsaInfo.toCpu.cpuCode[wrong enum values]
                 and other params from 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfDsaTagBuild
                            (dev, &dsaInfo, dsaBytes),
                            dsaInfo.dsaInfo.toCpu.cpuCode);

        /*
            {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E}
            1.10. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                              CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                   vpt [0 / 7],
                                                   cfiBit [0 / 1],
                                                   vid [0 / 100],
                                                   dropOnSource [GT_FAIL / GT_TRUE],
                                                   packetIsLooped [GT_FAIL / GT_TRUE] }
                                      dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E,
                                      dsaInfo{fromCpu {set from cpu}}
            Expected: GT_OK.
        */
        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;

        dsaInfo.commonParams.vpt    = 0;
        dsaInfo.commonParams.cfiBit = 0;
        dsaInfo.commonParams.vid    = 0;

        dsaInfo.commonParams.dropOnSource   = GT_FALSE;
        dsaInfo.commonParams.packetIsLooped = GT_FALSE;

        dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

        prvSetDsaInfoFromCpu(&dsaInfo);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;

        dsaInfo.commonParams.vpt    = 7;
        dsaInfo.commonParams.cfiBit = 1;
        dsaInfo.commonParams.vid    = 100;

        dsaInfo.commonParams.dropOnSource   = GT_TRUE;
        dsaInfo.commonParams.packetIsLooped = GT_TRUE;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.11. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_TRUNK_E] (not supported)
                            and other params from 1.10.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_TRUNK_E;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d",
                                        dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type);

        dsaInfo.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_VIDX_E;

        /*
            1.12. Call with wrong enum values dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type
                            and other params from 1.10.
            Extended: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfDsaTagBuild
                            (dev, &dsaInfo, dsaBytes),
                            dsaInfo.dsaInfo.fromCpu.dstInterface.type);

        /*
            1.13. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_PORT_E]
                            dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                            and other params from 1.10.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type           = CPSS_INTERFACE_PORT_E;
        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = %d",
                                        dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type, dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum);

        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = 0;

        /*
            1.14. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_PORT_E]
                            dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                            and other params from 1.10.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type            = CPSS_INTERFACE_PORT_E;
        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.portNum = %d",
                                        dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type, dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum);

        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum = NETIF_VALID_PHY_PORT_CNS;

        /*
            1.15. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (not relevant)
                            and other params from 1.10.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type           = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = %d",
                                        dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type, dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum);

        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = 0;

        /*
            1.16. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (not relevant)
                            and other params from 1.10.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type            = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.portNum = %d",
                                        dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type, dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum);

        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum = NETIF_VALID_PHY_PORT_CNS;

        /*
            1.17. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vidx [4096] (out of range)
                            and other params from 1.10.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.fromCpu.dstInterface.vidx = 4096;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vidx = %d",
                                        dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type, dsaInfo.dsaInfo.fromCpu.dstInterface.vidx);

        dsaInfo.dsaInfo.fromCpu.dstInterface.vidx = 100;

        /*
            1.18. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VID_E]
                            dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vidx [4096] (not relevant)
                            and other params from 1.10.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_VID_E;
        dsaInfo.dsaInfo.fromCpu.dstInterface.vidx = 4096;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vidx = %d",
                                    dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type, dsaInfo.dsaInfo.fromCpu.dstInterface.vidx);

        dsaInfo.dsaInfo.fromCpu.dstInterface.vidx = 100;

        /*
            1.21. Call with dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (not relevant)
                            and other params from 1.10.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.dstInterface.type   = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.fromCpu.dstInterface.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = %d, dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vlanId = %d",
                                     dev, dsaInfo.dsaInfo.fromCpu.dstInterface.type, dsaInfo.dsaInfo.fromCpu.dstInterface.vlanId);

        dsaInfo.dsaInfo.fromCpu.dstInterface.vlanId = 100;

        /*
            1.22. Call with dsaInfoPtr->dsaInfo.fromCpu.dp [wrong enum values]
                            and other params from 1.10.
            Extended: GT_BAD_PARAM. for DxCh3 - GT_OK(not relevant).
        */

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
           for(index = 0; index < utfInvalidEnumArrSize; index++)
           {
                dsaInfo.dsaInfo.fromCpu.dp = utfInvalidEnumArr[index];

                st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.dp = %d",
                                             dev, dsaInfo.dsaInfo.fromCpu.dp);
            }
        }
        else
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfDsaTagBuild
                                (dev, &dsaInfo, dsaBytes),
                                dsaInfo.dsaInfo.fromCpu.dp);
        }


        dsaInfo.dsaInfo.fromCpu.dp = CPSS_DP_GREEN_E;

        /*
            1.23. Call with dsaInfoPtr->dsaInfo.fromCpu.srcHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS]
                            and other params from 1.10.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.fromCpu.srcHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.fromCpu.srcHwDev = %d",
                                         dev, dsaInfo.dsaInfo.fromCpu.srcHwDev);

        dsaInfo.dsaInfo.fromCpu.srcHwDev = 0;

        /*
            {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E}
            1.24. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                              CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                   vpt [0 / 7],
                                                   cfiBit [0 / 1],
                                                   vid [0 / 100],
                                                   dropOnSource [GT_FAIL / GT_TRUE],
                                                   packetIsLooped [GT_FAIL / GT_TRUE] }
                                      dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E,
                                      dsaInfo{toAnalyzer {set to analyzer}}
            Expected: GT_OK.
        */
        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;

        dsaInfo.commonParams.vpt    = 0;
        dsaInfo.commonParams.cfiBit = 0;
        dsaInfo.commonParams.vid    = 0;

        dsaInfo.commonParams.dropOnSource   = GT_FALSE;
        dsaInfo.commonParams.packetIsLooped = GT_FALSE;

        dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E;

        prvSetDsaInfoToAnalizer(&dsaInfo);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;

        dsaInfo.commonParams.vpt    = 7;
        dsaInfo.commonParams.cfiBit = 1;
        dsaInfo.commonParams.vid    = 100;

        dsaInfo.commonParams.dropOnSource   = GT_TRUE;
        dsaInfo.commonParams.packetIsLooped = GT_TRUE;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.25 Call with dsaInfoPtr->dsaInfo.toAnalyzer.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                           and other params from 1.24
            Expected: NOT GT_OK.
        */
        dsaInfo.dsaInfo.toAnalyzer.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfo.dsaInfo.toAnalyzer.devPort.hwDevNum = %d",
                                    dev, dsaInfo.dsaInfo.toAnalyzer.devPort.hwDevNum);

        dsaInfo.dsaInfo.toAnalyzer.devPort.hwDevNum = 0;

        /*
            1.26 Call with dsaInfoPtr->dsaInfo.toAnalyzer.rxSniffer [GT_TRUE] (not extended rule)
                           dsaInfoPtr->dsaInfo.toAnalyzer.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                           and other params from 1.24
            Expected: NOT GT_OK.
        */
        dsaInfo.dsaInfo.toAnalyzer.rxSniffer       = GT_TRUE;
        dsaInfo.dsaInfo.toAnalyzer.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.toAnalyzer.rxSniffer = %d, dsaInfo.dsaInfo.toAnalyzer.devPort.portNum = %d",
                                    dev, dsaInfo.dsaInfo.toAnalyzer.rxSniffer, dsaInfo.dsaInfo.toAnalyzer.devPort.portNum);

        dsaInfo.dsaInfo.toAnalyzer.devPort.portNum = 0;

        /*
            1.27 Call with dsaInfo.commonParams.dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E] (rxtended rule)
                           dsaInfoPtr->dsaInfo.toAnalyzer.devPort.portNum [31] (out of range)
                           and other params from 1.24
            Expected: NOT GT_OK.
        */
        dsaInfo.commonParams.dsaTagType            = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;
        dsaInfo.dsaInfo.toAnalyzer.devPort.portNum = 32;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->commonParams.dsaTagType = %d, dsaInfo.dsaInfo.toAnalyzer.devPort.portNum = %d",
                                    dev, dsaInfo.commonParams.dsaTagType, dsaInfo.dsaInfo.toAnalyzer.devPort.portNum);

        dsaInfo.dsaInfo.toAnalyzer.devPort.portNum = 0;

        /*
            {dsaInfoPtr->dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E}
            1.28. Call with dsaInfoPtr{commonParams(dsaTagType[CPSS_DXCH_NET_DSA_TYPE_REGULAR_E /
                                                              CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                   vpt [0 / 7],
                                                   cfiBit [0 / 1],
                                                   vid [0 / 100],
                                                   dropOnSource [GT_FAIL / GT_TRUE],
                                                   packetIsLooped [GT_FAIL / GT_TRUE] }
                                      dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E,
                                      dsaInfo{forward {set forward}}
            Expected: GT_OK.
        */
        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;

        dsaInfo.commonParams.vpt    = 0;
        dsaInfo.commonParams.cfiBit = 0;
        dsaInfo.commonParams.vid    = 0;

        dsaInfo.commonParams.dropOnSource   = GT_FALSE;
        dsaInfo.commonParams.packetIsLooped = GT_FALSE;

        dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;

        prvSetDsaInfoForward(&dsaInfo);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* iterate with dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E */
        dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;

        dsaInfo.commonParams.vpt    = 7;
        dsaInfo.commonParams.cfiBit = 1;
        dsaInfo.commonParams.vid    = 100;

        dsaInfo.commonParams.dropOnSource   = GT_TRUE;
        dsaInfo.commonParams.packetIsLooped = GT_TRUE;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.29 Call with dsaInfoPtr->dsaInfo.forward.srcHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                           and other params from 1.28.
            Expected: NOT GT_OK.
        */
        dsaInfo.dsaInfo.forward.srcHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.srcHwDev = %d",
                                         dev, dsaInfo.dsaInfo.forward.srcHwDev);

        dsaInfo.dsaInfo.forward.srcHwDev = 0;

        /*
            1.30 Call with dsaInfoPtr->dsaInfo.forward.srcIsTrunk [GT_FALSE]
                           dsaInfoPtr->dsaInfo.forward.source.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                           and other params from 1.28.
            Expected: NOT GT_OK.
        */
        dsaInfo.dsaInfo.forward.srcIsTrunk     = GT_FALSE;
        dsaInfo.dsaInfo.forward.source.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.srcIsTrunk = %d, dsaInfoPtr->dsaInfo.forward.source.portNum = %d",
                                        dev, dsaInfo.dsaInfo.forward.srcIsTrunk, dsaInfo.dsaInfo.forward.source.portNum);

        dsaInfo.dsaInfo.forward.source.portNum = 0;

        /*
            1.30 Call with dsaInfoPtr->dsaInfo.forward.srcIsTrunk [GT_TRUE]
                           dsaInfo.dsaInfo.forward.srcIsTrunk [maximal]
                           and other params from 1.28.
            Expected: GT_OK.
        */
        dsaInfo.dsaInfo.forward.srcIsTrunk     = GT_TRUE;
        dsaInfo.dsaInfo.forward.source.trunkId = BIT_7 - 1;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.srcIsTrunk = %d, dsaInfo.dsaInfo.forward.source.trunkId = %d",
                                    dev, dsaInfo.dsaInfo.forward.srcIsTrunk, dsaInfo.dsaInfo.forward.source.trunkId);

        dsaInfo.dsaInfo.forward.srcIsTrunk     = GT_FALSE;
        dsaInfo.dsaInfo.forward.source.portNum = 0;

        /*
            1.31 Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type [CPSS_INTERFACE_TRUNK_E] (not supported)
                           and other params from 1.28.
            Expected: NOT GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type  = CPSS_INTERFACE_TRUNK_E;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d",
                                        dev, dsaInfo.dsaInfo.forward.dstInterface.type);

        dsaInfo.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_VIDX_E;

        /*
            1.32 Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type [wrong enum values]
                           and other params from 1.28.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfDsaTagBuild
                            (dev, &dsaInfo, dsaBytes),
                            dsaInfo.dsaInfo.forward.dstInterface.type);

        /*
            1.33. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E]
                            dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (out of range)
                            and other params from 1.28.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type           = CPSS_INTERFACE_PORT_E;
        dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d, dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum = %d",
                                        dev, dsaInfo.dsaInfo.forward.dstInterface.type, dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum);

        dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum = 0;

        /*
            1.34. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E]
                            dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (out of range)
                            and other params from 1.28.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type            = CPSS_INTERFACE_PORT_E;
        dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d, dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.portNum = %d",
                                        dev, dsaInfo.dsaInfo.forward.dstInterface.type, dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum);

        dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum = 0;

        /*
            1.35. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS] (not relevant)
                            and other params from 1.28.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type           = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d, dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum = %d",
                                     dev, dsaInfo.dsaInfo.forward.dstInterface.type, dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum);

        dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum = 0;

        /*
            1.36. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (not relevant)
                            and other params from 1.28.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type            = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d, dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.portNum = %d",
                                     dev, dsaInfo.dsaInfo.forward.dstInterface.type, dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum);

        dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum = 0;

        /*
            1.37. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.forward.dstInterface.vidx [4096] (out of range)
                            and other params from 1.28.
            Extended: NOT GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.forward.dstInterface.vidx = 4096;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d, dsaInfoPtr->dsaInfo.forward.dstInterface.vidx = %d",
                                         dev, dsaInfo.dsaInfo.forward.dstInterface.type, dsaInfo.dsaInfo.forward.dstInterface.vidx);

        dsaInfo.dsaInfo.forward.dstInterface.vidx = 0;

        /*
            1.38. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VID_E]
                            dsaInfoPtr->dsaInfo.forward.dstInterface.vidx [4096] (not relevant)
                            and other params from 1.28.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_VID_E;
        dsaInfo.dsaInfo.forward.dstInterface.vidx = 4096;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d, dsaInfoPtr->dsaInfo.forward.dstInterface.vidx = %d",
                                     dev, dsaInfo.dsaInfo.forward.dstInterface.type, dsaInfo.dsaInfo.forward.dstInterface.vidx);

        dsaInfo.dsaInfo.forward.dstInterface.vidx = 0;

        /*
            1.41. Call with dsaInfoPtr->dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_VIDX_E]
                            dsaInfoPtr->dsaInfo.forward.dstInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (not relevant)
                            and other params from 1.28.
            Extended: GT_OK.
        */
        dsaInfo.dsaInfo.forward.dstInterface.type   = CPSS_INTERFACE_VIDX_E;
        dsaInfo.dsaInfo.forward.dstInterface.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, dsaInfoPtr->dsaInfo.forward.dstInterface.type = %d, dsaInfoPtr->dsaInfo.forward.dstInterface.vlanId = %d",
                                         dev, dsaInfo.dsaInfo.forward.dstInterface.type, dsaInfo.dsaInfo.forward.dstInterface.vlanId);

        dsaInfo.dsaInfo.forward.dstInterface.vlanId = 100;

        /*
            1.42 Call with dsaInfoPtr[NULL]
                           and other params from 1.28.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfDsaTagBuild(dev, NULL, dsaBytes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dsaInfoPtr = NULL", dev);

        /*
            1.43 Call with dsaBytesPtr[NULL]
                           and other params from 1.28.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dsaBytesPtr = NULL", dev);
    }

    dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;

    dsaInfo.commonParams.vpt    = 0;
    dsaInfo.commonParams.cfiBit = 0;
    dsaInfo.commonParams.vid    = 0;

    dsaInfo.commonParams.dropOnSource   = GT_FALSE;
    dsaInfo.commonParams.packetIsLooped = GT_FALSE;

    dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;

    prvSetDsaInfoToCpu(&dsaInfo);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfDsaTagBuild(dev, &dsaInfo, dsaBytes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfDuplicateEnableSet
(
    IN  GT_U8                          devNum,
    IN  GT_BOOL                        enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfDuplicateEnableSet)
{
/*
    ITERATE DEVICE (DxCh2 and above)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfDuplicateEnableGet.
    Expected: GT_OK and same enablePtr.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        enable = GT_FALSE;

        st = cpssDxChNetIfDuplicateEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*  1.2.  */
        st = cpssDxChNetIfDuplicateEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  Verify values  */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChNetIfDuplicateEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*  1.2.  */
        st = cpssDxChNetIfDuplicateEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  Verify values  */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable than was set: %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfDuplicateEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfDuplicateEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeToPhysicalPortGet)
{
/*
    ITERATE DEVICE (BC2 and above)
    1.1. Call with portNumPtr.
    Expected: GT_OK.
    1.2. Call with NULL portNumPtr.
    Expected: GT_BAD_PTR.
    1.3 Call with non exist cpuCode
    Expected: GT_BAD_PARAM
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32                   notAppFamilyBmp = 0;
    GT_PHYSICAL_PORT_NUM     portNum = 0;

    /* prepare device iterator */
    /* ITERATE_DEVICES (Bobcat2, Caelum, Aldrin, Xcat3x, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChNetIfCpuCodeToPhysicalPortGet(dev,CPSS_NET_CONTROL_BPDU_E ,&portNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChNetIfCpuCodeToPhysicalPortGet(dev,CPSS_NET_CONTROL_BPDU_E, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeToPhysicalPortGet(dev,CPSS_NET_INTERVENTION_ARP_E,&portNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*3. Call function with out of bound value for cpuCode*/

    st = cpssDxChNetIfCpuCodeToPhysicalPortGet(dev,CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E +1 ,&portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    /* 4. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeToPhysicalPortGet(dev,CPSS_NET_INTERVENTION_ARP_E ,&portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeToPhysicalPortSet
(
    IN  GT_U8                          devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT       cpuCode,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeToPhysicalPortSet)
{
/*
    ITERATE_DEVICE (BC2 and above)
    1.1.1. Call with few cpu codes.
    Expected: GT_OK.
    1.1.2. Call cpssDxChNetIfCpuCodeToPhysicalPortGet.
    Expected: GT_OK and same enablePtr.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port, port_get;
    GT_U32      notAppFamilyBmp = 0;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with some cpu codes
                Expected: GT_OK.
            */

            st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev,CPSS_NET_LOCK_PORT_MIRROR_E,port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, CPSS_NET_LOCK_PORT_MIRROR_E, port);

            /*  1.2.  */
            st = cpssDxChNetIfCpuCodeToPhysicalPortGet(dev, CPSS_NET_LOCK_PORT_MIRROR_E, &port_get);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  Verify values  */
            UTF_VERIFY_EQUAL1_STRING_MAC(port, port_get,
                                         "got another port than was set: %d", dev);

            /* iterate with CPSS_NET_EGRESS_SAMPLED_E */


            st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev,CPSS_NET_IPV6_NEIGHBOR_SOLICITATION_E,port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, CPSS_NET_IPV6_NEIGHBOR_SOLICITATION_E ,port);

            /*  1.2.  */
            st = cpssDxChNetIfCpuCodeToPhysicalPortGet(dev,CPSS_NET_IPV6_NEIGHBOR_SOLICITATION_E,&port_get);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  Verify values  */
            UTF_VERIFY_EQUAL1_STRING_MAC(port, port_get,
                                         "got another port than was set: %d", dev);
        }

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev, CPSS_NET_UDP_BC_MIRROR_TRAP2_E, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev, CPSS_NET_UDP_BC_MIRROR_TRAP2_E ,port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev,CPSS_NET_UDP_BC_MIRROR_TRAP2_E ,port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = NETIF_VALID_PHY_PORT_CNS;
    /* 2. Call function with out of bound cpu code.*/

    st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev, CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E +1 ,port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    /* 3. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev,CPSS_NET_UDP_BC_MIRROR_TRAP2_E ,port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }


    /* 4. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeToPhysicalPortSet(dev, CPSS_NET_UDP_BC_MIRROR_TRAP2_E ,port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaPhysicalPortToQueueGet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U8                       *firstQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaPhysicalPortToQueueGet)
{
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port,portGet;
    GT_U8   firstQueue,firstQueueGet;
    GT_U32  numOfNetIfs;
    CPSS_PORTS_BMP_STC  cpuSdmaPortsBmp;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /****************************************************/
        /* check how many netIf SDMA the initialization did */
        /****************************************************/
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev,&numOfNetIfs);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&cpuSdmaPortsBmp);

        /* make sure at least one CPU SDMA */
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, numOfNetIfs);

        /* check that the 8 queues per CPU port are different from each other
           and that port <--> firstQueue are matched  */
        for(firstQueue = 0 ; firstQueue < (numOfNetIfs * 8) ; firstQueue += 8)
        {
            st = cpssDxChNetIfSdmaQueueToPhysicalPortGet(dev, firstQueue + 3 , &portGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, firstQueue);

            port = portGet;

            /* make sure this port is NOT already used */
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, CPSS_PORTS_BMP_IS_PORT_SET_MAC(&cpuSdmaPortsBmp,port));

            st = cpssDxChNetIfSdmaPhysicalPortToQueueGet(dev,port,&firstQueueGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* make sure the port <--> firstQueue are matched */
            UTF_VERIFY_EQUAL2_PARAM_MAC(firstQueue, firstQueueGet, dev, port);

            /* state the this port is used */
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,port,GT_TRUE);
        }

#ifdef  ASIC_SIMULATION
        if(numOfNetIfs == 1 &&
           GT_FALSE == prvUtfIsGmCompilation() &&
           PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
           !PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /*****************************/
            /* PATCH for ASIC_SIMULATION */
            /*****************************/
            /* the appDemo of BC3/Aldrin2 initialized 4 CPU SDMA ports but the
               function prvCpssDxChNetIfMultiNetIfNumberGet will return : 1 (not 4)
               because WM not supports traffic on those MG1..3 */

            /* state the this port is used */
            port = 80;
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,port,GT_TRUE);
            port = 81;
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,port,GT_TRUE);
            port = 82;
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,port,GT_TRUE);

            /* state the this port is used for 512 ports mode */
            port = 336;
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,port,GT_TRUE);
            port = 337;
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,port,GT_TRUE);
            port = 338;
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,port,GT_TRUE);
        }
#endif /*ASIC_SIMULATION*/

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {

            /* check NULL pointer */
            st = cpssDxChNetIfSdmaPhysicalPortToQueueGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);


            if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&cpuSdmaPortsBmp,port))
            {
                /* we already checked this port */
                continue;
            }

            /* this port is not 'CPU SDMA port' ... need to fail */
            st = cpssDxChNetIfSdmaPhysicalPortToQueueGet(dev,port,&firstQueueGet);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    port = CPSS_CPU_PORT_NUM_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = cpssDxChNetIfSdmaPhysicalPortToQueueGet(dev,port ,&firstQueueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }


    /* 4. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaPhysicalPortToQueueGet(dev,port ,&firstQueueGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaQueueToPhysicalPortGet
(
    IN GT_U8                        devNum,
    IN GT_U8                        queueIdx,
    OUT GT_PHYSICAL_PORT_NUM        *portNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaQueueToPhysicalPortGet)
{
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  lastPortGet = 0,portGet;
    GT_U8   queue;
    GT_U32  numOfNetIfs;
    CPSS_PORTS_BMP_STC  cpuSdmaPortsBmp;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /****************************************************/
        /* check how many netIf SDMA the initialization did */
        /****************************************************/
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev,&numOfNetIfs);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&cpuSdmaPortsBmp);

        /* check that the 8 queues per CPU port are different from each other
           and that port <--> firstQueue are matched  */
        for(queue = 0 ; /* no limit here*/ ; queue ++)
        {
            /* check NULL pointer */
            st = cpssDxChNetIfSdmaQueueToPhysicalPortGet(dev, queue , NULL);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);


            st = cpssDxChNetIfSdmaQueueToPhysicalPortGet(dev, queue , &portGet);
            if(queue < (numOfNetIfs * 8))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

                if(0 == (queue % 8))
                {
                    lastPortGet = portGet;

                    /* make sure this port is NOT already used */
                    UTF_VERIFY_EQUAL0_PARAM_MAC(0, CPSS_PORTS_BMP_IS_PORT_SET_MAC(&cpuSdmaPortsBmp,portGet));

                    /* state the this port is used */
                    CPSS_PORTS_BMP_PORT_ENABLE_MAC(&cpuSdmaPortsBmp,portGet,GT_TRUE);
                }
                else
                {
                    /* check that the port hold 8 consecutive queues */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(lastPortGet, portGet, dev, queue);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, queue);
            }

            if(queue == 255)/*GT_U8 limit*/
            {
                /* no more possible queues to query */
                break;
            }
        }/*queue*/
    } /*dev*/


    queue = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = cpssDxChNetIfSdmaQueueToPhysicalPortGet(dev, queue , &portGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }


    /* 4. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaQueueToPhysicalPortGet(dev, queue , &portGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}



/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChNetIfDuplicateEnableGet)
{
/*
    ITERATE DEVICE (DxCh2 and above)
    1.1. Call with enable.
    Expected: GT_OK.
    1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChNetIfDuplicateEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChNetIfDuplicateEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfDuplicateEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfDuplicateEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortDuplicateToCpuSet
(
    IN  GT_U8                          devNum,
    IN  GT_U8                          portNum,
    IN  GT_BOOL                        enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortDuplicateToCpuSet)
{
/*
    ITERATE_DEVICE_PHY_CPU_PORT (DxCh2 and above)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChNetIfPortDuplicateToCpuGet.
    Expected: GT_OK and same enablePtr.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;

    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChNetIfPortDuplicateToCpuSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*  1.2.  */
            st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  Verify values  */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                         "got another enable than was set: %d", dev);

            /* iterate with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChNetIfPortDuplicateToCpuSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*  1.2.  */
            st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  Verify values  */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                         "got another enable than was set: %d", dev);
        }

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNetIfPortDuplicateToCpuSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfPortDuplicateToCpuSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNetIfPortDuplicateToCpuSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;

    port = NETIF_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortDuplicateToCpuSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortDuplicateToCpuSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortDuplicateToCpuGet)
{
/*
    ITERATE_DEVICE_PHY_CPU_PORT (DxCh2 and above)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*  1.1.1.  */
            st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;

    port = NETIF_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortDuplicateToCpuGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpSynSet
(
    IN  GT_U8                          devNum,
    IN  GT_BOOL                        enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpSynSet)
{
/*
    ITERATE_DEVICE (DxCh2 and above)
    1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfAppSpecificCpuCodeTcpSynGet with not NULL enablePtr
    Expected: GT_OK and the same enablePtr as was et.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /* iterate with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChNetIfAppSpecificCpuCodeTcpSynGet with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeTcpSynGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet, "got another enable than was set %d", dev);

        /* iterate with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChNetIfAppSpecificCpuCodeTcpSynGet with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeTcpSynGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet, "got another enable than was set %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeTcpSynSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpSynGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpSynGet)
{
/*
    ITERATE_DEVICE (DxCh2 and above)
    1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL]
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeTcpSynGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeTcpSynGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet
(
    IN GT_U8            devNum,
    IN GT_U32           index,
    IN GT_U8            protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT   cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with index[0 / 7],
                   protocol[0 / 255],
                   cpuCode[CPSS_NET_TCP_SYN_TO_CPU_E / CPSS_NET_MIRROR_IPV6_UC_ICMP_REDIRECT_E].
    Expected: GT_Ok.
    1.2. Call cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet with index [0 / 7]
              and not NULL validPtr, protocolPtr, cpuCodePtr.
    Expected: GT_Ok and the same protocolPtr, cpuCodePtr as was set.
    1.3. Call with out of range  index[8] and other parameters from 1.1.
    Expected: NOT GT_Ok.
    1.4. Call with wrong enum values cpuCode and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.5 Call cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate with index[0 / 7].
    Expected: GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                      index;
    GT_U8                       protocol;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;
    GT_U8                       protocolGet;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCodeGet;
    GT_BOOL                     valid;


    index       = 0;
    protocol    = 0;
    protocolGet = 0;
    valid       = GT_FALSE;
    cpuCode     = CPSS_NET_ALL_CPU_OPCODES_E;
    cpuCodeGet  = CPSS_NET_ALL_CPU_OPCODES_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[0 / 7],
                           protocol[0 / 255],
                           cpuCode[CPSS_NET_TCP_SYN_TO_CPU_E / CPSS_NET_MIRROR_IPV6_UC_ICMP_REDIRECT_E].
            Expected: GT_Ok.
        */
        /* iterate with index = 0 */
        index    = 0;
        protocol = 0;
        cpuCode  = CPSS_NET_TCP_SYN_TO_CPU_E;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet(dev, index, protocol, cpuCode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet with index [0 / 7]
                      and not NULL validPtr, protocolPtr, cpuCodePtr.
            Expected: GT_Ok and the same protocolPtr, cpuCodePtr as was set.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocolGet, &cpuCodeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet: %d, %d", dev, index);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(protocol, protocolGet, "got another protocol than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /* iterate with index = 7 */
        index    = 7;
        protocol = 255;
        cpuCode  = CPSS_NET_MIRROR_IPV6_UC_ICMP_REDIRECT_E;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet(dev, index, protocol, cpuCode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet with index [0 / 7]
                      and not NULL validPtr, protocolPtr, cpuCodePtr.
            Expected: GT_Ok and the same protocolPtr, cpuCodePtr as was set.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocolGet, &cpuCodeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet: %d, %d", dev, index);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(protocol, protocolGet, "got another protocol than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /*
            1.3. Call with index[8] and other parameters from 1.1.
            Expected: NOT GT_Ok.
        */
        index = 8;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet(dev, index, protocol, cpuCode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 7;

        /*
            1.4. Call with cpuCode[wrong enum values] and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet
                            (dev, index, protocol, cpuCode),
                            cpuCode);

        /*
            1.5 Call cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate with index[0 / 7].
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate: %d, %d", dev, index);

        /* iterate with index = 7 */
        index = 7;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate: %d, %d", dev, index);
    }

    index    = 0;
    protocol = 0;
    cpuCode  = CPSS_NET_UN_REGISTERD_MC_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet(dev, index, protocol, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet(dev, index, protocol, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate
(
    IN GT_U8            devNum,
    IN GT_U32           index
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with index[0 / 7].
    Expected: GT_OK.
    1.2. Call with out of range index[8].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      index;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[0 / 7].
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* iterate with index = 7 */
        index = 7;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with index[8].
            Expected: NOT GT_OK.
        */
        index = 8;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(dev, index);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(dev, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(dev, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet
(
    IN GT_U8            devNum,
    IN GT_U32           index,
    OUT GT_BOOL         *validPtr,
    OUT GT_U8           *protocolPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT   *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with index [0 / 7] and not NULL validPtr, protocolPtr, cpuCodePtr.
    Expected: GT_OK.
    1.2. Call with out of range index [8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with validPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with protocolPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with cpuCodePtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      index    = 0;
    GT_BOOL     valid    = GT_FALSE;
    GT_U8       protocol = 0;

    CPSS_NET_RX_CPU_CODE_ENT   cpuCode;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / 7] and not NULL validPtr, protocolPtr, cpuCodePtr.
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* iterate with index = 7 */
        index = 7;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with index [8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        index = 8;

        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocol, &cpuCode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 7;

        /*
            1.3. Call with validPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, NULL, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, validPtr = NULL", dev);

        /*
            1.4. Call with protocolPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, NULL, &cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, protocolPtr = NULL", dev);

        /*
            1.5. Call with cpuCodePtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocol, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpuCodePtr = NULL", dev);

    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(dev, index, &valid, &protocol, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           rangeIndex,
    IN GT_U16                           minDstPort,
    IN GT_U16                           maxDstPort,
    IN CPSS_NET_TCP_UDP_PACKET_TYPE_ENT packetType,
    IN CPSS_NET_PROT_ENT                protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT         cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with rangeIndex[0 / 15],
                   minDstPort[0],
                   maxDstPort[63],
                   packetType[CPSS_NET_TCP_UDP_PACKET_UC_E / CPSS_NET_TCP_UDP_PACKET_MC_E],
                   protocol[CPSS_NET_PROT_UDP_E / CPSS_NET_PROT_TCP_E,],
                   cpuCode[CPSS_NET_UN_REGISTERD_MC_E / CPSS_NET_INTERVENTION_DA_E]
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet with rangeIndex[0 / 15]
              and not NULL validPtr, rangeIndexPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr.
    Expected: GT_OK and the same rangeIndexPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr as was set.
    1.3. Call with out of range rangeIndex[16] and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with wrong enum values packetType and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong enum values protocol and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values cpuCode  and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate with rangeIndex[0 / 15]
    Expected: GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                           rangeIndex    = 0;
    GT_U16                           minDstPort    = 0;
    GT_U16                           maxDstPort    = 0;
    CPSS_NET_TCP_UDP_PACKET_TYPE_ENT packetType    = CPSS_NET_TCP_UDP_PACKET_UC_E;
    CPSS_NET_PROT_ENT                protocol      = CPSS_NET_PROT_UDP_E;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode       = CPSS_NET_UN_REGISTERD_MC_E;
    GT_U16                           minDstPortGet = 0;
    GT_U16                           maxDstPortGet = 0;
    CPSS_NET_TCP_UDP_PACKET_TYPE_ENT packetTypeGet = CPSS_NET_TCP_UDP_PACKET_UC_E;
    CPSS_NET_PROT_ENT                protocolGet   = CPSS_NET_PROT_UDP_E;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCodeGet    = CPSS_NET_UN_REGISTERD_MC_E;
    GT_BOOL                          valid         = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with rangeIndex[0 / 15],
                           minDstPort[0],
                           maxDstPort[63],
                           packetType[CPSS_NET_TCP_UDP_PACKET_UC_E / CPSS_NET_TCP_UDP_PACKET_MC_E],
                           protocol[CPSS_NET_PROT_UDP_E / CPSS_NET_PROT_TCP_E,],
                           cpuCode[CPSS_NET_TCP_SYN_TO_CPU_E / CPSS_NET_MIRROR_IPV6_UC_ICMP_REDIRECT_E]
            Expected: GT_OK.
        */
        /* iterate with rangeIndex = 0 */
        rangeIndex = 0;
        minDstPort = 0;
        maxDstPort = 0;
        packetType = CPSS_NET_TCP_UDP_PACKET_UC_E;
        protocol   = CPSS_NET_PROT_UDP_E;
        cpuCode    = CPSS_NET_TCP_SYN_TO_CPU_E;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet(dev, rangeIndex, minDstPort, maxDstPort, packetType, protocol, cpuCode);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, dev, rangeIndex, minDstPort, maxDstPort, packetType, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet with rangeIndex[0 / 15]
                      and not NULL validPtr, rangeIndexPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr.
            Expected: GT_OK and the same rangeIndexPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr as was set.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPortGet, &maxDstPortGet, &packetTypeGet, &protocolGet, &cpuCodeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet: %d, %d", dev, rangeIndex);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(minDstPort, minDstPortGet, "got another minDstPort than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(maxDstPort, maxDstPortGet, "got another maxDstPort than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetType, packetTypeGet, "got another packetType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(protocol, protocolGet, "got another protocol than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /* iterate with rangeIndex = 15 */
        rangeIndex = 15;
        minDstPort = 0;
        maxDstPort = 0;
        packetType = CPSS_NET_TCP_UDP_PACKET_MC_E;
        protocol   = CPSS_NET_PROT_TCP_E;
        cpuCode    = CPSS_NET_MIRROR_IPV6_UC_ICMP_REDIRECT_E;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet(dev, rangeIndex, minDstPort, maxDstPort, packetType, protocol, cpuCode);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, dev, rangeIndex, minDstPort, maxDstPort, packetType, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet with rangeIndex[0 / 15]
                      and not NULL validPtr, rangeIndexPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr.
            Expected: GT_OK and the same rangeIndexPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr as was set.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPortGet, &maxDstPortGet, &packetTypeGet, &protocolGet, &cpuCodeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet: %d, %d", dev, rangeIndex);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(minDstPort, minDstPortGet, "got another minDstPort than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(maxDstPort, maxDstPortGet, "got another maxDstPort than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetType, packetTypeGet, "got another packetType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(protocol, protocolGet, "got another protocol than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /*
            1.3. Call with out of range rangeIndex[16] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        rangeIndex = 16;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet(dev, rangeIndex, minDstPort, maxDstPort, packetType, protocol, cpuCode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rangeIndex);

        rangeIndex = 15;

        /*
            1.4. Call with wrong enum values packetType and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet
                            (dev, rangeIndex, minDstPort, maxDstPort,
                             packetType, protocol, cpuCode),
                            packetType);

        /*
            1.5. Call with wrong enum values protocol and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet
                            (dev, rangeIndex, minDstPort, maxDstPort,
                             packetType, protocol, cpuCode),
                            protocol);

        /*
            1.6. Call with wrong enum values cpuCode  and other params from 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet
                            (dev, rangeIndex, minDstPort, maxDstPort,
                             packetType, protocol, cpuCode),
                            cpuCode);

        /*
            1.7. Call cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate with rangeIndex[0 / 15]
            Expected: GT_OK.
        */
        /* iterte with rangeIndex = 0 */
        rangeIndex = 0;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(dev, rangeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate: %d, %d", dev, rangeIndex);

        /* iterte with rangeIndex = 15 */
        rangeIndex = 15;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(dev, rangeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate: %d, %d", dev, rangeIndex);
    }

    rangeIndex = 0;
    minDstPort = 0;
    maxDstPort = 0;
    packetType = CPSS_NET_TCP_UDP_PACKET_UC_E;
    protocol   = CPSS_NET_PROT_UDP_E;
    cpuCode    = CPSS_NET_UN_REGISTERD_MC_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet(dev, rangeIndex, minDstPort, maxDstPort, packetType, protocol, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet(dev, rangeIndex, minDstPort, maxDstPort, packetType, protocol, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate
(
    IN GT_U8                            devNum,
    IN GT_U32                           rangeIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with rangeIndex [0 / 15].
    Expected: GT_OK.
    1.2. Call with out of range rangeIndex [16].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      rangeIndex = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with rangeIndex [0 / 15].
            Expected: GT_OK.
        */
        /* iterate with rangeIndex = 0 */
        rangeIndex = 0;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(dev, rangeIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, rangeIndex);

        /* iterate with rangeIndex = 15 */
        rangeIndex = 15;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(dev, rangeIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, rangeIndex);

        /*
            1.2. Call with out of range rangeIndex [16].
            Expected: NOT GT_OK.
        */
        rangeIndex = 16;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(dev, rangeIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rangeIndex);
    }

    rangeIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(dev, rangeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(dev, rangeIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet
(
    IN GT_U8                             devNum,
    IN GT_U32                            rangeIndex,
    OUT GT_BOOL                          *validPtr,
    OUT GT_U16                           *minDstPortPtr,
    OUT GT_U16                           *maxDstPortPtr,
    OUT CPSS_NET_TCP_UDP_PACKET_TYPE_ENT *packetTypePtr,
    OUT CPSS_NET_PROT_ENT                *protocolPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT         *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with rangeIndex[0 / 15] and not NULL validPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr.
    Expected: GT_OK.
    1.2. Call with out of range rangeIndex[16] and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with validPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with minDstPortPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with maxDstPortPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with packetTypePtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call with protocolPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.8. Call with cpuCodePtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                           rangeIndex    = 0;
    GT_U16                           minDstPort    = 0;
    GT_U16                           maxDstPort    = 0;
    CPSS_NET_TCP_UDP_PACKET_TYPE_ENT packetType    = CPSS_NET_TCP_UDP_PACKET_UC_E;
    CPSS_NET_PROT_ENT                protocol      = CPSS_NET_PROT_UDP_E;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode       = CPSS_NET_UN_REGISTERD_MC_E;
    GT_BOOL                          valid         = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with rangeIndex[0 / 15] and not NULL validPtr, minDstPortPtr, maxDstPortPtr, packetTypePtr, protocolPtr, cpuCodePtr.
            Expected: GT_OK.
        */
        /* iterate with rangeIndex = 0 */
        rangeIndex = 0;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, &packetType, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, rangeIndex);

        /* iterate with rangeIndex = 15 */
        rangeIndex = 15;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, &packetType, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, rangeIndex);

        /*
            1.2. Call with out of range rangeIndex[16] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        rangeIndex = 16;

        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, &packetType, &protocol, &cpuCode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rangeIndex);

        /*
            1.3. Call with validPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, NULL, &minDstPort, &maxDstPort, &packetType, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, validPtr = NULL", dev);

        /*
            1.4. Call with minDstPortPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, NULL, &maxDstPort, &packetType, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, minDstPortPtr = NULL", dev);

        /*
            1.5. Call with maxDstPortPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, NULL, &packetType, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maxDstPortPtr = NULL", dev);

        /*
            1.6. Call with packetTypePtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, NULL, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetTypePtr = NULL", dev);

        /*
            1.7. Call with protocolPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, &packetType, NULL, &cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, protocolPtr = NULL", dev);

        /*
            1.8. Call with cpuCodePtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, &packetType, &protocol, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpuCodePtr = NULL", dev);
    }

    rangeIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, &packetType, &protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(dev, rangeIndex, &valid, &minDstPort, &maxDstPort, &packetType, &protocol, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeIpLinkLocalProtSet
(
    IN GT_U8                      devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT ipVer,
    IN GT_U8                      protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT   cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeIpLinkLocalProtSet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with ipVer[CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_PROTOCOL_IPV6_E], protocol[0], cpuCode[CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E / CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E]
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfCpuCodeIpLinkLocalProtGet with not NULL cpuCodePtr and other params from 1.1.
    Expected: GT_OK and the same cpuCode as was set.
    1.3. Call with wrong enum values ipVer and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values cpuCode and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with cpuCode[CPSS_NET_CONTROL_DEST_MAC_TRAP_E] and other params from 1.1. (not supported)
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_IP_PROTOCOL_STACK_ENT    ipVer      = CPSS_IP_PROTOCOL_IPV4_E;
    GT_U8                         protocol   = 0;
    CPSS_NET_RX_CPU_CODE_ENT      cpuCode    = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;
    CPSS_NET_RX_CPU_CODE_ENT      cpuCodeGet = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with ipVer[CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_PROTOCOL_IPV6_E], protocol[0], cpuCode[CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E / CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E]
            Expected: GT_OK.
        */
        /* iterate with ipVer = CPSS_IP_PROTOCOL_IPV4_E */
        ipVer    = CPSS_IP_PROTOCOL_IPV4_E;
        protocol = 0;
        cpuCode  = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;

        st = cpssDxChNetIfCpuCodeIpLinkLocalProtSet(dev, ipVer, protocol, cpuCode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ipVer, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfCpuCodeIpLinkLocalProtGet with not NULL cpuCodePtr and other params from 1.1.
            Expected: GT_OK and the same cpuCode as was set.
        */
        st = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(dev, ipVer, protocol, &cpuCodeGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeIpLinkLocalProtGet: %d, %d, %d", dev, ipVer, protocol);

        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /* iterate with ipVer = CPSS_IP_PROTOCOL_IPV6_E */
        ipVer    = CPSS_IP_PROTOCOL_IPV6_E;
        protocol = 0;
        cpuCode  = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E;

        st = cpssDxChNetIfCpuCodeIpLinkLocalProtSet(dev, ipVer, protocol, cpuCode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ipVer, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfCpuCodeIpLinkLocalProtGet with not NULL cpuCodePtr and other params from 1.1.
            Expected: GT_OK and the same cpuCode as was set.
        */
        st = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(dev, ipVer, protocol, &cpuCodeGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeIpLinkLocalProtGet: %d, %d, %d", dev, ipVer, protocol);

        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values ipVer and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeIpLinkLocalProtSet
                            (dev, ipVer, protocol, cpuCode),
                            ipVer);

        /*
            1.4. Call with wrong enum values cpuCode and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeIpLinkLocalProtSet
                            (dev, ipVer, protocol, cpuCode),
                            cpuCode);

        /*
            1.5. Call with cpuCode[CPSS_NET_CONTROL_DEST_MAC_TRAP_E] and other params from 1.1. (not supported)
            Expected: NOT GT_OK.
        */
        cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E;

        st = cpssDxChNetIfCpuCodeIpLinkLocalProtSet(dev, ipVer, protocol, cpuCode);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cpuCode = %d", dev, cpuCode);
    }

    ipVer    = CPSS_IP_PROTOCOL_IPV4_E;
    protocol = 0;
    cpuCode  = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeIpLinkLocalProtSet(dev, ipVer, protocol, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeIpLinkLocalProtSet(dev, ipVer, protocol, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeIpLinkLocalProtGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   ipVer,
    IN GT_U8                        protocol,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeIpLinkLocalProtGet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with protocol[0], ipVer[CPSS_IP_PROTOCOL_IPV4_E] and not NULL cpuCodePtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values ipVer and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with cpuCodePtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                      protocol = 0;
    CPSS_IP_PROTOCOL_STACK_ENT ipVer    = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_NET_RX_CPU_CODE_ENT   cpuCode  = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with protocol[0] and not NULL cpuCodePtr.
            Expected: GT_OK.
        */
        protocol = 0;

        st = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(dev, ipVer, protocol, &cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ipVer, protocol);

        /*
            1.2. Call with wrong enum values ipVer and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeIpLinkLocalProtGet
                            (dev, ipVer, protocol, &cpuCode),
                            ipVer);

        /*
            1.3. Call with cpuCodePtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(dev, ipVer, protocol, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpuCodePtr = NULL", dev);
    }

    protocol = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(dev, ipVer, protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(dev, ipVer, protocol, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet
(
    IN GT_U8                    devNum,
    IN GT_U8                    protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with protocol[0], cpuCode[CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E / CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E]
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet with not NULL cpuCodePtr and other params from 1.1.
    Expected: GT_OK and the same cpuCode as was set.
    1.3. Call with wrong enum values cpuCode and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with cpuCode[CPSS_NET_CONTROL_DEST_MAC_TRAP_E] and other params from 1.1. (not supported)
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                       protocol   = 0;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode    = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCodeGet = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with protocol[0],
                           cpuCode[CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E /
                                   CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E]
            Expected: GT_OK.
        */
        /* iterate with cpuCode = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E */
        protocol = 0;
        cpuCode  = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E;

        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet(dev, protocol, cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet with not NULL cpuCodePtr
                                                                        and other params from 1.1.
            Expected: GT_OK and the same cpuCode as was set.
        */
        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(dev, protocol, &cpuCodeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet: %d, %d", dev, protocol);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /* iterate with cpuCode = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E */
        protocol = 0;
        cpuCode  = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E;

        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet(dev, protocol, cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cpuCode);

        /*
            1.2. Call cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet with not NULL cpuCodePtr
                                                                        and other params from 1.1.
            Expected: GT_OK and the same cpuCode as was set.
        */
        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(dev, protocol, &cpuCodeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet: %d, %d", dev, protocol);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet, "got another cpuCode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values cpuCode and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet
                            (dev, protocol, cpuCode),
                            cpuCode);

        /*
            1.4. Call with cpuCode[CPSS_NET_CONTROL_DEST_MAC_TRAP_E]
                           and other params from 1.1. (not supported)
            Expected: NOT GT_OK.
        */
        cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E;

        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet(dev, protocol, cpuCode);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cpuCode = %d", dev, cpuCode);
    }

    protocol = 0;
    cpuCode  = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet(dev, protocol, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet(dev, protocol, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet
(
    IN GT_U8                      devNum,
    IN GT_U8                      protocol,
    OUT CPSS_NET_RX_CPU_CODE_ENT  *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet)
{
/*
    ITERATE_DEVICE (DXCH2 and above)
    1.1. Call with protocol[0], and not NULL cpuCodePtr.
    Expected: GT_OK.
    1.2. Call with cpuCodePtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                       protocol = 0;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode  = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with protocol[0],
                           and not NULL cpuCodePtr.
            Expected: GT_OK.
        */
        protocol = 0;

        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(dev, protocol, &cpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, protocol);

        /*
            1.2. Call with cpuCodePtr[NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(dev, protocol, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpuCodePtr = NULL", dev);
    }

    protocol = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(dev, protocol, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(dev, protocol, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeTableSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeTableSet)
{
/*
    ITERATE_DEVICE (DXCH)
    1.1. Call with cpuCode[CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E / CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E],
                   entryInfo{
                            tc[0 / 7],
                            dp[CPSS_DP_GREEN_E / CPSS_DP_RED_E],
                            truncate[GT_FALSE / GT_TRUE],
                            cpuRateLimitMode[CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E / CPSS_NET_CPU_CODE_RATE_LIMIT_AGGREGATE_E],
                            cpuCodeRateLimiterIndex[0 / 31],
                            cpuCodeStatRateLimitIndex[0 / 31],
                            designatedDevNumIndex[0 / 7] }.
    Expected: GT_OK
    1.2. Call cpssDxChNetIfCpuCodeTableGet with not NULL entryInfoPtr and other params from 1.1.
    Expected: GT_OK and the same entryInfo as aws set.
    1.3. Call with wrong enum values cpuCode and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with wrong enum values entryInfoPtr->dp and other params from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with wrong enum values entryInfoPtr->cpuRateLimitMode and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with out of range entryInfoPtr->cpuCodeRateLimiterIndex[32] and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with out of range entryInfoPtr->cpuCodeStatRateLimitIndex[32] and other params from 1.1.
    Expected: NOT GT_OK.
    1.8. Call with out of range entryInfoPtr->designatedDevNumIndex[8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.9. Call with out of range entryInfoPtr->tc [8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.10. Call with out of range entryInfoPtr->dp [CPSS_DP_YELLOW_E] and other params from 1.1.
    Expected: NOT GT_OK.
    1.11. Call with out of range entryInfoPtr->entryInfoPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL                                 isEqual   = GT_FALSE;
    CPSS_NET_RX_CPU_CODE_ENT                cpuCode   = CPSS_NET_CONTROL_E;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC  entryInfo;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC  entryInfoGet;
    GT_U32  max_cpuCodeRateLimiterIndex,max_cpuCodeStatRateLimitIndex;


    cpssOsBzero((GT_VOID*) &entryInfo, sizeof(entryInfo));
    cpssOsBzero((GT_VOID*) &entryInfoGet, sizeof(entryInfoGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            max_cpuCodeRateLimiterIndex = 255;
            max_cpuCodeStatRateLimitIndex = 255;
        }
        else
        {
            max_cpuCodeRateLimiterIndex = 31;
            max_cpuCodeStatRateLimitIndex = 31;
        }

        /*
            1.1. Call with cpuCode[CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E / CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E],
                           entryInfo{
                                    tc[0 / 7],
                                    dp[CPSS_DP_GREEN_E / CPSS_DP_RED_E],
                                    truncate[GT_FALSE / GT_TRUE],
                                    cpuRateLimitMode[CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E / CPSS_NET_CPU_CODE_RATE_LIMIT_AGGREGATE_E],
                                    cpuCodeRateLimiterIndex[0 / 31],
                                    cpuCodeStatRateLimitIndex[0 / 31],
                                    designatedDevNumIndex[0 / 7] }.
            Expected: GT_OK
        */
        /* iterate with cpuCode = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E */
        cpuCode = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;
        entryInfo.tc       = 0;
        entryInfo.dp       = CPSS_DP_GREEN_E;
        entryInfo.truncate = GT_FALSE;
        entryInfo.cpuRateLimitMode          = CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E;
        entryInfo.cpuCodeRateLimiterIndex   = 0;
        entryInfo.cpuCodeStatRateLimitIndex = 0;
        entryInfo.designatedDevNumIndex     = 0;

        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cpuCode);

        /*
            1.2. Call cpssDxChNetIfCpuCodeTableGet with not NULL entryInfoPtr and other params from 1.1.
            Expected: GT_OK and the same entryInfo as aws set.
        */
        st = cpssDxChNetIfCpuCodeTableGet(dev, cpuCode, &entryInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeTableGet: %d, %d", dev, cpuCode);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &entryInfo, (GT_VOID*) &entryInfoGet, sizeof(entryInfo) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another entryInfo than was set", dev);

        /* iterate with cpuCode = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E */
        cpuCode = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E;
        entryInfo.tc       = 7;
        entryInfo.dp       = CPSS_DP_RED_E;
        entryInfo.truncate = GT_TRUE;
        entryInfo.cpuRateLimitMode          = CPSS_NET_CPU_CODE_RATE_LIMIT_AGGREGATE_E;
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
            entryInfo.cpuCodeRateLimiterIndex   = max_cpuCodeRateLimiterIndex;
        entryInfo.cpuCodeStatRateLimitIndex = max_cpuCodeStatRateLimitIndex;
        entryInfo.designatedDevNumIndex     = 7;

        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cpuCode);

        /*
            1.2. Call cpssDxChNetIfCpuCodeTableGet with not NULL entryInfoPtr and other params from 1.1.
            Expected: GT_OK and the same entryInfo as aws set.
        */
        st = cpssDxChNetIfCpuCodeTableGet(dev, cpuCode, &entryInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeTableGet: %d, %d", dev, cpuCode);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &entryInfo, (GT_VOID*) &entryInfoGet, sizeof(entryInfo) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another entryInfo than was set", dev);

        /*
            1.3. Call with wrong enum values cpuCode and other params from 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeTableSet
                            (dev, cpuCode, &entryInfo),
                            cpuCode);

        /*
            1.4. Call with wrong enum values entryInfoPtr->dp and other params from 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeTableSet
                            (dev, cpuCode, &entryInfo),
                            entryInfo.dp);

        /*
            1.5. Call with wrong enum values entryInfoPtr->cpuRateLimitMode
                 and other params from 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeTableSet
                            (dev, cpuCode, &entryInfo),
                            entryInfo.cpuRateLimitMode);

        /*
            1.6. Call with out of range entryInfoPtr->cpuCodeRateLimiterIndex[32] and other params from 1.1.
            Expected: NOT GT_OK.
        */

        entryInfo.cpuCodeRateLimiterIndex = max_cpuCodeRateLimiterIndex + 1;


        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, entryInfo.cpuCodeRateLimiterIndex = %d",
                                     dev, entryInfo.cpuCodeRateLimiterIndex);
        else
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, entryInfo.cpuCodeRateLimiterIndex = %d",
                                     dev, entryInfo.cpuCodeRateLimiterIndex);

        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
            entryInfo.cpuCodeRateLimiterIndex = max_cpuCodeRateLimiterIndex;
        else
            entryInfo.cpuCodeRateLimiterIndex = 0;

        /*
            1.7. Call with out of range entryInfoPtr->cpuCodeStatRateLimitIndex[32] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        entryInfo.cpuCodeStatRateLimitIndex = max_cpuCodeStatRateLimitIndex+1;

        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, entryInfo.cpuCodeStatRateLimitIndex = %d",
                                     dev, entryInfo.cpuCodeStatRateLimitIndex);

        entryInfo.cpuCodeStatRateLimitIndex = max_cpuCodeStatRateLimitIndex;

        /*
            1.8. Call with out of range entryInfoPtr->designatedDevNumIndex[8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        entryInfo.designatedDevNumIndex = 8;

        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, entryInfo.designatedDevNumIndex = %d",
                                     dev, entryInfo.designatedDevNumIndex);

        entryInfo.designatedDevNumIndex = 7;

        /*
            1.9. Call with out of range entryInfoPtr->tc [8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        entryInfo.tc = 8;

        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, entryInfo.tc = %d", dev, entryInfo.tc);

        entryInfo.tc = 7;

        /*
            1.10. Call with out of range entryInfoPtr->dp [CPSS_DP_YELLOW_E] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        entryInfo.dp = CPSS_DP_YELLOW_E;

        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(
                GT_OK, st, "%d, entryInfo.dp = %d", dev, entryInfo.dp);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "%d, entryInfo.dp = %d", dev, entryInfo.dp);
        }

        if (st == GT_OK)
        {
            st = cpssDxChNetIfCpuCodeTableGet(dev, cpuCode, &entryInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeTableGet: %d, %d", dev, cpuCode);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &entryInfo, (GT_VOID*) &entryInfoGet, sizeof(entryInfo) )) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another entryInfo than was set", dev);
        }

        entryInfo.dp = CPSS_DP_GREEN_E;

        /*
            1.11. Call with out of range entryInfoPtr->entryInfoPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, entryInfoPtr = NULL", dev);
    }

    cpuCode = CPSS_NET_INTERVENTION_SA_E;
    entryInfo.tc       = 0;
    entryInfo.dp       = CPSS_DP_GREEN_E;
    entryInfo.truncate = GT_FALSE;
    entryInfo.cpuRateLimitMode          = CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E;
    entryInfo.cpuCodeRateLimiterIndex   = 0;
    entryInfo.cpuCodeStatRateLimitIndex = 0;
    entryInfo.designatedDevNumIndex     = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeTableSet(dev, cpuCode, &entryInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeTableGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeTableGet)
{
/*
    ITERATE_DEVICE (DXCH)
    1.1. Call with cpuCode[CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E] and not NULL entryInfoPtr.
    Expected: GT_OK.
    1.2. Call with cpuCode[wrong enum values] and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with entryInfoPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_NET_RX_CPU_CODE_ENT                cpuCode   = CPSS_NET_INTERVENTION_SA_E;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC  entryInfo;


    cpssOsBzero((GT_VOID*) &entryInfo, sizeof(entryInfo));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with cpuCode[CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E]
                           and not NULL entryInfoPtr.
            Expected: GT_OK.
        */
        cpuCode = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E;

        st = cpssDxChNetIfCpuCodeTableGet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cpuCode);

        /*
            1.2. Call with cpuCode[wrong enum values] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfCpuCodeTableGet
                            (dev, cpuCode, &entryInfo),
                            cpuCode);

        /*
            1.3. Call with entryInfoPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeTableGet(dev, cpuCode, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, entryInfoPtr=  NULL", dev);
    }

    cpuCode = CPSS_NET_INTERVENTION_SA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeTableGet(dev, cpuCode, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeTableGet(dev, cpuCode, &entryInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_U32       statisticalRateLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet)
{
/*
    ITERATE_DEVICE (DXCHX)
    1.1. Call with index[0 / 31], statisticalRateLimit[0 / 0xFFFFFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet with not NULL statisticalRateLimitPtr
                                                                     and other params from 1.1.
    Expected: GT_OK and the same statisticalRateLimit as was set.
    1.3. Call with index[32] and other param from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32       index = 0;
    GT_U32       statisticalRateLimit    = 0;
    GT_U32       statisticalRateLimitArrGet[1] = {0};


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[0 / 31], statisticalRateLimit[0 / 0xFFFFFFFF].
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;
        statisticalRateLimit = 0;

        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(dev, index, statisticalRateLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, statisticalRateLimit);

        /*
            1.2. Call cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet with not NULL statisticalRateLimitPtr
                                                                             and other params from 1.1.
            Expected: GT_OK and the same statisticalRateLimit as was set.
        */
        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, statisticalRateLimitArrGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(statisticalRateLimit, statisticalRateLimitArrGet[0],
                                     "got another statisticalRateLimit than was set: %d", dev);

        /* iterate with index = 31 */
        index = 31;
        statisticalRateLimit = 0xFFFFFFFF;

        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(dev, index, statisticalRateLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, statisticalRateLimit);

        /*
            1.2. Call cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet with not NULL statisticalRateLimitPtr
                                                                             and other params from 1.1.
            Expected: GT_OK and the same statisticalRateLimit as was set.
        */
        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, statisticalRateLimitArrGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(statisticalRateLimit, statisticalRateLimitArrGet[0],
                                     "got another statisticalRateLimit than was set: %d", dev);

        /*
            1.3. Call with index[32] and other param from 1.1.
            Expected: NOT GT_OK.
        */
        index = 32;

        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(dev, index, statisticalRateLimit);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
    }

    index = 0;
    statisticalRateLimit = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(dev, index, statisticalRateLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(dev, index, statisticalRateLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      index,
    OUT GT_U32      *statisticalRateLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet)
{
/*
    ITERATE_DEVICE (DXCHX)
    1.1. Call with index[0 / 31] and not NULL statisticalRateLimitPtr.
    Expected: GT_OK.
    1.2. Call with index[32] and other param from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with statisticalRateLimitPtr[NULL] and other param from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      index = 0;
    GT_U32      statisticalRateLimitArr[1] = {0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[0 / 31] and not NULL statisticalRateLimitPtr.
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, statisticalRateLimitArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* iterate with index = 31 */
        index = 31;

        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, statisticalRateLimitArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with index[32] and other param from 1.1.
            Expected: NOT GT_OK.
        */
        index = 32;

        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, statisticalRateLimitArr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 31;

        /*
            1.3. Call with statisticalRateLimitPtr[NULL] and other param from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, statisticalRateLimit = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, statisticalRateLimitArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(dev, index, statisticalRateLimitArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeDesignatedDeviceTableSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_U8        designatedHwDevNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeDesignatedDeviceTableSet)
{
/*
    ITERATE_DEVICE (DXCH)
    1.1. Call with index[1 / 7], designatedHwDevNum[0 / 31].
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfCpuCodeDesignatedDeviceTableGet with not NULL designatedHwDevNumPtr
                                                                     and other params from 1.1.
    Expected: GT_OK and the same designatedHwDevNum as was set.
    1.3. Call with out of range index[8] and other param from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range index[0] and other param from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range designatedHwDevNum[32] and other param from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32       index = 0;
    GT_HW_DEV_NUM   designatedHwDevNum    = 0;
    GT_HW_DEV_NUM   designatedHwDevNumGet = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[1 / 7], designatedHwDevNum[0 / 31].
            Expected: GT_OK.
        */
        /* iterate with index = 1 */
        index = 1;
        designatedHwDevNum = 0;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(dev, index, designatedHwDevNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, designatedHwDevNum);

        /*
            1.2. Call cpssDxChNetIfCpuCodeDesignatedDeviceTableGet with not NULL designatedHwDevNumPtr
                                                                             and other params from 1.1.
            Expected: GT_OK and the same designatedHwDevNum as was set.
        */
        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNumGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeDesignatedDeviceTableGet: %d, %d",
                                     dev, index);
        UTF_VERIFY_EQUAL1_STRING_MAC(designatedHwDevNum, designatedHwDevNumGet,
                                     "got another designatedHwDevNum than was set: %d", dev);

        /* iterate with index = 7 */
        index = 7;
        designatedHwDevNum = 31;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(dev, index, designatedHwDevNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, designatedHwDevNum);

        /*
            1.2. Call cpssDxChNetIfCpuCodeDesignatedDeviceTableGet with not NULL designatedHwDevNumPtr
                                                                             and other params from 1.1.
            Expected: GT_OK and the same designatedHwDevNum as was set.
        */
        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNumGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeDesignatedDeviceTableGet: %d, %d",
                                     dev, index);
        UTF_VERIFY_EQUAL1_STRING_MAC(designatedHwDevNum, designatedHwDevNumGet,
                                     "got another designatedHwDevNum than was set: %d", dev);

        /*
            1.3. Call with out of range index[8] and other param from 1.1.
            Expected: NOT GT_OK.
        */
        index = 8;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(dev, index, designatedHwDevNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 7;

        /*
            1.4. Call with out of range index[0] and other param from 1.1.
            Expected: NOT GT_OK.
        */
        index = 0;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(dev, index, designatedHwDevNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 7;

        /*
            1.5. Call with out of range designatedHwDevNum[32] and other param from 1.1.
            Expected: NOT GT_OK.
        */
        designatedHwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(dev, index, designatedHwDevNum);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, designatedHwDevNum = %d", dev, designatedHwDevNum);
    }

    index = 0;
    designatedHwDevNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(dev, index, designatedHwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(dev, index, designatedHwDevNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeDesignatedDeviceTableGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      index,
    OUT GT_U8       *designatedHwDevNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeDesignatedDeviceTableGet)
{
/*
    ITERATE_DEVICE (DXCH)
    1.1. Call with index[1 / 7] and not NULL designatedHwDevNumPtr.
    Expected: GT_OK.
    1.2. Call with out of range index[8] and other param from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with out of range index[0] and other param from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with designatedHwDevNumPtr [NULL] and other param from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      index = 0;
    GT_HW_DEV_NUM   designatedHwDevNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[1 / 7] and not NULL designatedHwDevNumPtr.
            Expected: GT_OK.
        */
        /* iterate with index = 1 */
        index = 1;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* iterate with index = 7 */
        index = 7;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with out of range index[8] and other param from 1.1.
            Expected: NOT GT_OK.
        */
        index = 8;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 1;

        /*
            1.3. Call with out of range index[0] and other param from 1.1.
            Expected: NOT GT_OK.
        */
        index = 0;

        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 1;

        /*
            1.4. Call with designatedHwDevNumPtr [NULL] and other param from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, designatedHwDevNumPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(dev, index, &designatedHwDevNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterTableSet
(
    IN GT_U8  devNum,
    IN GT_U32 rateLimiterIndex,
    IN GT_U32 windowSize,
    IN GT_U32 pktLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeRateLimiterTableSet)
{
/*
    ITERATE_DEVICE (DxCh2 and above)
    1.1. Call with rateLimiterIndex[1 / rateLimiterIndexMax],
                   windowSize[0 / 0xFFF],
                   pktLimit[0 / 0xFFFF]
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfCpuCodeRateLimiterTableGet with not NULL windowSizePtr, pktLimitPtr and other params from 1.1.
    Expected: GT_OK and the same windowSize, pktLimit as was set.
    1.3. Call with out of range rateLimiterIndex[0] and other params from 1.1.
    Expected: NOT GT_OK
    1.4. Call with out of range rateLimiterIndex[rateLimiterIndexMax + 1] and other params from 1.1.
    Expected: NOT GT_OK
    1.5. Call with out of range windowSize[0x1000] and other params from 1.1.
    Expected: NOT GT_OK
    1.6. Call with out of range pktLimit[0x10000] and other params from 1.1.
    Expected: NOT GT_OK
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      rateLimiterIndex = 0;
    GT_U32      windowSize       = 0;
    GT_U32      pktLimit         = 0;
    GT_U32      windowSizeGet    = 0;
    GT_U32      pktLimitGet      = 0;
    GT_U32      rateLimiterIndexMax;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

         if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
         {
             rateLimiterIndexMax = 255;
         }
         else
         {
             rateLimiterIndexMax = 31;
         }
        /*
            1.1. Call with rateLimiterIndex[1 / rateLimiterIndexMax],
                           windowSize[0 / 0xFFF],
                           pktLimit[0 / 0xFFFF]
            Expected: GT_OK.
        */
        /* iterate with rateLimiterIndex = 1 */
        rateLimiterIndex = 1;
        windowSize       = 0;
        pktLimit         = 0;

        st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex, windowSize, pktLimit);

        /*
            1.2. Call cpssDxChNetIfCpuCodeRateLimiterTableGet with not NULL windowSizePtr, pktLimitPtr and other params from 1.1.
            Expected: GT_OK and the same windowSize, pktLimit as was set.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSizeGet, &pktLimitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeRateLimiterTableGet: %d, %d", dev, rateLimiterIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet, "got another windowSize than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pktLimit, pktLimitGet, "got another pktLimit than was set: %d", dev);

        /* iterate with rateLimiterIndex = rateLimiterIndexMax */
        rateLimiterIndex = rateLimiterIndexMax;
        windowSize       = 0xFFF;
        pktLimit         = 0xFFFF;

        st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex, windowSize, pktLimit);

        /*
            1.2. Call cpssDxChNetIfCpuCodeRateLimiterTableGet with not NULL windowSizePtr, pktLimitPtr and other params from 1.1.
            Expected: GT_OK and the same windowSize, pktLimit as was set.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSizeGet, &pktLimitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeRateLimiterTableGet: %d, %d", dev, rateLimiterIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet, "got another windowSize than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pktLimit, pktLimitGet, "got another pktLimit than was set: %d", dev);

        /*
            1.3. Call with out of range rateLimiterIndex[0] and other params from 1.1.
            Expected: NOT GT_OK
        */
        rateLimiterIndex = 0;

        st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex);

        rateLimiterIndex = rateLimiterIndexMax;

        /*
            1.4. Call with out of range rateLimiterIndex[rateLimiterIndexMax] and other params from 1.1.
            Expected: NOT GT_OK
        */
        rateLimiterIndex = rateLimiterIndexMax + 1;

        st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex);

        rateLimiterIndex = rateLimiterIndexMax;

        /*
            1.5. Call with out of range windowSize[0x1000] and other params from 1.1.
            Expected: NOT GT_OK
        */
        windowSize = 0x1000;

        st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, windowSize = %d", dev, windowSize);

        windowSize = 0xFFF;

        /*
            1.5. Call with out of range pktLimit[0x10000] and other params from 1.1.
            Expected: NOT GT_OK
        */
        pktLimit = 0x10000;

        st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, pktLimit = %d", dev, pktLimit);
    }

    rateLimiterIndex = 1;
    windowSize       = 0;
    pktLimit         = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeRateLimiterTableSet(dev, rateLimiterIndex, windowSize, pktLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterTableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  rateLimiterIndex,
    OUT GT_U32  *windowSizePtr,
    OUT GT_U32  *pktLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeRateLimiterTableGet)
{
/*
    ITERATE_DEVICE (DxCh2 and above)
    1.1. Call with rateLimiterIndex[1 / rateLimiterIndexMax] and not NULL windowSizePtr, pktLimitPtr
    Expected: GT_OK.
    1.2. Call with rateLimiterIndex[0] and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with rateLimiterIndex[rateLimiterIndexMax + 1] and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with windowSizePtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with pktLimitPtr[NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      rateLimiterIndex = 0;
    GT_U32      windowSize       = 0;
    GT_U32      pktLimit         = 0;
    GT_U32      rateLimiterIndexMax;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
         if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
         {
             rateLimiterIndexMax = 255;
         }
         else
         {
             rateLimiterIndexMax = 31;
         }
        /*
            1.1. Call with rateLimiterIndex[1 / rateLimiterIndexMax] and not NULL windowSizePtr, pktLimitPtr
            Expected: GT_OK.
        */
        /* iterate with rateLimiterIndex = 1 */
        rateLimiterIndex = 1;

        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSize, &pktLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex);

        /* iterate with rateLimiterIndex = rateLimiterIndexMax */
        rateLimiterIndex = rateLimiterIndexMax;

        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSize, &pktLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex);

        /*
            1.2. Call with rateLimiterIndex[0] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        rateLimiterIndex = 0;

        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSize, &pktLimit);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex);

        rateLimiterIndex = rateLimiterIndexMax;

        /*
            1.3. Call with rateLimiterIndex[rateLimiterIndexMax + 1] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        rateLimiterIndex = rateLimiterIndexMax + 1;

        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSize, &pktLimit);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rateLimiterIndex);

        rateLimiterIndex = rateLimiterIndexMax;

        /*
            1.4. Call with windowSizePtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, NULL, &pktLimit);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, windowSizePtr = NULL", dev);

        /*
            1.5. Call with pktLimitPtr[NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSize, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pktLimitPtr = NULL", dev);
    }

    rateLimiterIndex = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSize, &pktLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeRateLimiterTableGet(dev, rateLimiterIndex, &windowSize, &pktLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet
(
    IN GT_U8  devNum,
    IN GT_U32 windowResolution
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet)
{
/*
    ITERATE_DEVICE (DxCh2 and above)
    1.1. Call with windowResolution[step1*granularity / step2*granularity], where granularity = (1000 * systemClockCycles) / coreClock(dev).
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet with not NULL windowResolutionPtr.
    Expected: GT_OK and the same windowResolution as was set.
    1.3. Call with windowResolution[0]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32 windowResolution    = 0;
    GT_U32 windowResolutionGet = 0;
    GT_U32 coreClock           = 0;
    GT_U32 granularity         = 0;
    GT_U32 systemClockCycles   = 0;
    GT_U32 step1               = 0;
    GT_U32 step2               = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device coreClock */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            systemClockCycles = 256;
            step1 = 10;
            step2 = 20;
        }
        else
        {
            systemClockCycles = 32;
            step1 = 1;
            step2 = 2;
        }

        st = prvUtfCoreClockGet(dev, &coreClock);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCoreClockGet: %d", dev);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, coreClock, "prvUtfCoreClockGet: coreClock = 0, dev = %d", dev);
        if(0 == coreClock)
        {
            continue;
        }

        /*
            1.1. Call with windowResolution[step1 * granularity / step2 * granularity]
            Expected: GT_OK.
        */
        /* iterate with windowResolution = step1 * granularity */
        granularity      = (1000 * systemClockCycles) / coreClock;
        windowResolution = step1 * granularity;

        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(dev, windowResolution);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, windowResolution);

        /*
            1.2. Call cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet with not NULL windowResolutionPtr.
            Expected: GT_OK and the same windowResolution as was set.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(dev, &windowResolutionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(windowResolution, windowResolutionGet, "got another windowResolution than was set: %d", dev);

        /* iterate with windowResolution = step2 * granularity */
        windowResolution = step2 * granularity;

        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(dev, windowResolution);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, windowResolution);

        /*
            1.2. Call cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet with not NULL windowResolutionPtr.
            Expected: GT_OK and the same windowResolution as was set.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(dev, &windowResolutionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(windowResolution, windowResolutionGet, "got another windowResolution than was set: %d", dev);

        /*
            1.3. Call with windowResolution[0]
            Expected: NOT GT_OK.
        */
        windowResolution = 0;

        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(dev, windowResolution);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, windowResolution);
    }

    windowResolution = 1 * granularity;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(dev, windowResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(dev, windowResolution);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet
(
    IN GT_U8  devNum,
    IN GT_U32 *windowResolutionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet)
{
/*
    ITERATE_DEVICE (DxCh2 and above)
    1.1. Call with not NULL windowResolutionPtr.
    Expected: GT_OK.
    1.2. Call with windowResolutionPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      windowResolution = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL windowResolutionPtr.
            Expected: GT_OK.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(dev, &windowResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with windowResolutionPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, windowResolutionPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(dev, &windowResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(dev, &windowResolution);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfFromCpuDpSet
(
    IN GT_U8               devNum,
    IN  CPSS_DP_LEVEL_ENT  dpLevel
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfFromCpuDpSet)
{
/*
    ITERATE_DEVICE (DxCh3 and above)
    1.1. Call with dpLevel [CPSS_DP_GREEN_E / CPSS_DP_RED_E].
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfFromCpuDpGet with not NULL dpLevelPtr.
    Expected: GT_OK.
    1.3. Call with dpLevel [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.4. Call with dpLevel [CPSS_DP_YELLOW_E].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DP_LEVEL_ENT  dpLevel    = CPSS_DP_GREEN_E;
    CPSS_DP_LEVEL_ENT  dpLevelGet = CPSS_DP_GREEN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with dpLevel [CPSS_DP_GREEN_E / CPSS_DP_RED_E].
            Expected: GT_OK.
        */
        /* iterate with dpLevel = CPSS_DP_GREEN_E */
        dpLevel = CPSS_DP_GREEN_E;

        st = cpssDxChNetIfFromCpuDpSet(dev, dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dpLevel);

        /*
            1.2. Call cpssDxChNetIfFromCpuDpGet with not NULL dpLevelPtr.
            Expected: GT_OK.
        */
        st = cpssDxChNetIfFromCpuDpGet(dev, &dpLevelGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNetIfFromCpuDpGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(dpLevel, dpLevelGet, "get another dpLevel than was set: %d", dev);

        /* iterate with dpLevel = CPSS_DP_RED_E */
        dpLevel = CPSS_DP_RED_E;

        st = cpssDxChNetIfFromCpuDpSet(dev, dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dpLevel);

        /*
            1.2. Call cpssDxChNetIfFromCpuDpGet with not NULL dpLevelPtr.
            Expected: GT_OK.
        */
        st = cpssDxChNetIfFromCpuDpGet(dev, &dpLevelGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNetIfFromCpuDpGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(dpLevel, dpLevelGet, "get another dpLevel than was set: %d", dev);

        /*
            1.3. Call with dpLevel [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfFromCpuDpSet
                            (dev, dpLevel),
                            dpLevel);

        /*
            1.4. Call with dpLevel [CPSS_DP_YELLOW_E].
            Expected: NOT GT_OK.
        */
        dpLevel = CPSS_DP_YELLOW_E;

        st = cpssDxChNetIfFromCpuDpSet(dev, dpLevel);
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, dpLevel);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dpLevel);
        }

        if (st == GT_OK)
        {
            st = cpssDxChNetIfFromCpuDpGet(dev, &dpLevelGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNetIfFromCpuDpGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(dpLevel, dpLevelGet, "get another dpLevel than was set: %d", dev);
        }
    }

    dpLevel = CPSS_DP_GREEN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfFromCpuDpSet(dev, dpLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfFromCpuDpSet(dev, dpLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfFromCpuDpGet
(
    IN  GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT  *dpLevelPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfFromCpuDpGet)
{
/*
    ITERATE_DEVICE (DxCh3 and above)
    1.1. Call with not NULL dpLevelPtr.
    Expected: GT_OK.
    1.2. Call with dpLevel [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DP_LEVEL_ENT  dpLevel = CPSS_DP_GREEN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL dpLevelPtr.
            Expected: GT_OK.
        */
        st = cpssDxChNetIfFromCpuDpGet(dev, &dpLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with dpLevel [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfFromCpuDpGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dpLevelPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfFromCpuDpGet(dev, &dpLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfFromCpuDpGet(dev, &dpLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaRxResourceErrorModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   queue,
    IN  CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaRxResourceErrorModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with queue[0 / 7],
                   mode [CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E / CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E].
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfSdmaRxResourceErrorModeGet.
    Expected: GT_OK and same modePtr.
    1.3. Call with queue[8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with mode [wrong enum values] and other params from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8 queue = 0;
    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT mode;
    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT modeGet;
    GT_U32                                numOfNetIfs;

    mode = CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E;
    modeGet = CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.1. Call with queue[0 / 7],  PRV_CPSS_DXCH3_SDMA_WA_E
                           mode [CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E / CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E].
            Expected: GT_OK.
        */
        /* iterate with queue = 0 */
        queue = 0;
        mode = CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E;

        st = cpssDxChNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        /* Due to Cheetah 3 Errata - " FEr#2009: DSDMA resource errors may cause
           PCI Express packets reception malfunction"
           -- see PRV_CPSS_DXCH3_SDMA_WA_E */
        if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH3_SDMA_WA_E) == GT_TRUE)
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);

            /*  1.2.  */
            st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

            /*  Verify values  */
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                         "got another mode than was set: %d", dev);
        }

        /* iterate with queue = 7 */
        queue = 7;
        mode = CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E;

        st = cpssDxChNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        /* Due to Cheetah 3 Errata - " FEr#2009: DSDMA resource errors may cause
           PCI Express packets reception malfunction"
           -- see PRV_CPSS_DXCH3_SDMA_WA_E */
        if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH3_SDMA_WA_E) == GT_TRUE)
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);

            /*  1.2.  */
            st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

            /*  Verify values  */
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                         "got another mode than was set: %d", dev);
        }

        /*
            1.3. Call with queue[8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        queue = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, queue, mode);

        queue = 0;

        /*
            1.4. Call with mode [wrong enum values] and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        /* Due to Cheetah 3 Errata - " FEr#2009: DSDMA resource errors may cause
           PCI Express packets reception malfunction"
           -- see PRV_CPSS_DXCH3_SDMA_WA_E */
        if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH3_SDMA_WA_E) == GT_TRUE)
        {
            for(enumsIndex = 0; enumsIndex < utfInvalidEnumArrSize; enumsIndex++)
            {
                mode = utfInvalidEnumArr[enumsIndex];
                st = cpssDxChNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, queue, mode);
            }
        }
        else
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaRxResourceErrorModeSet
                                (dev, queue, mode),
                                mode);
        }
    }

    queue = 0;
    mode = CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaRxResourceErrorModeSet(dev, queue, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaRxResourceErrorModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
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
    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT mode = CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E;
    GT_U32                                numOfNetIfs;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*  1.1. */
        /* iterate with queue = 0 */
        queue = 0;

        st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        /* iterate with queue = 7 */
        queue = 7;

        st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        /*  1.2.  */
        queue = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queue);

        queue = 0;

        /*  1.3.  */
        st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, NULL);
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
        st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaRxResourceErrorModeGet(dev, queue, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfDsaTagParse
(
    IN  GT_U8                          devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfDsaTagParse)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with dsaBytesPtr[1] and non null dsaInfoPtr.
    Expected: GT_OK.
    1.2. Call with wrong  dsaBytesPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with wrong  dsaInfoPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8                          dsaBytesArr[16];
    CPSS_DXCH_NET_DSA_PARAMS_STC   dsaInfo;

    /* DSA bytes content is not important for the test */
    cpssOsMemSet(dsaBytesArr, 0, sizeof(dsaBytesArr));
    dsaBytesArr[0] = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with dsaBytesPtr[1] and non null dsaInfoPtr.
            Expected: GT_OK.
        */

        st = cpssDxChNetIfDsaTagParse(dev, dsaBytesArr, &dsaInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong  dsaBytesPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChNetIfDsaTagParse(dev, NULL, &dsaInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with wrong  dsaInfoPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChNetIfDsaTagParse(dev, dsaBytesArr, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfDsaTagParse(dev, dsaBytesArr, &dsaInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfDsaTagParse(dev, dsaBytesArr, &dsaInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfInit
(
    IN  GT_U8                          devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfInit)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct dev.
            Expected: GT_OK.
        */

        st = cpssDxChNetIfInit(dev);
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfRemove
(
    IN  GT_U8                          devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfRemove)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct dev.
            Expected: GT_OK.
        */

        st = cpssDxChNetIfRemove(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfRemove(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfRemove(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfRxBufFree
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfRxBufFree)
{
/*
    ITERATE_DEVICES (DxChx)
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
        /* Only buffers that recived by dxChNetIfRxPacketGet can be free*/
        st = cpssDxChNetIfRxBufFree(dev, rxQueue, rxBuffList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        #endif
        /*
            1.2. Call with wrong rxBuffList pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChNetIfRxBufFree(dev, rxQueue, NULL, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfRxBufFree(dev, rxQueue, rxBuffList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfRxBufFree(dev, rxQueue, rxBuffList, buffListLen);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfRxBufFreeWithSize
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfRxBufFreeWithSize)
{
/*
    ITERATE_DEVICES (DxChx)
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

        st = cpssDxChNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
                                            rxBuffSizeList, buffListLen);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->netIf.allocMethod[rxQueue] == CPSS_RX_BUFF_NO_ALLOC_E)
        {
            if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
            {
                if((PRV_CPSS_PP_MAC(dev)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E) &&
                    !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(dev))
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
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
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

        st = cpssDxChNetIfRxBufFreeWithSize(dev, rxQueue, NULL,
                                            rxBuffSizeList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.2. Call with wrong rxBuffSizeList pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
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
        st = cpssDxChNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
                                            rxBuffSizeList, buffListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfRxBufFreeWithSize(dev, rxQueue, rxBuffList,
                                        rxBuffSizeList, buffListLen);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaSyncTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaSyncTxPacketSend)
{
/*
    ITERATE_DEVICES (DxChx)
    ======= DSA_CMD_TO_CPU =======
    1.1. Call with pcktParamsPtr{
                      packetIsTagged [GT_TRUE],
                      cookie [],
                      sdmaInfo {recalcCrc [GT_TRUE],
                                txQueue [0],
                                invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                      dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                              vpt[0],
                                              cfiBit [0],
                                              vid [0],
                                              dropOnSource [GT_FALSE],
                                              packetIsLooped [GT_FALSE]}
                                dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
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
                      dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                              vpt[3],
                                              cfiBit [1],
                                              vid [100],
                                              dropOnSource [GT_TRUE],
                                              packetIsLooped [GT_TRUE]}
                                dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
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
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged[CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E]
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
                                  dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                                          vpt[5],
                                                          cfiBit [0],
                                                          vid [1000],
                                                          dropOnSource [GT_FALSE],
                                                          packetIsLooped [GT_TRUE]}
                                            dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
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
                                  dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                          vpt[7],
                                                          cfiBit [1],
                                                          vid [0xFFF],
                                                          dropOnSource [GT_TRUE],
                                                          packetIsLooped [GT_FALSE]}
                                            dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                                            dsaInfo.forward {srcIsTagged [CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E],
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
    1.33. Call with pcktParamsPtr->dsaParam.dsaInfo.forward.dstInterface.srcIsTagged[CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E]
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
    CPSS_DXCH_NET_TX_PARAMS_STC pcktParams;
    GT_U8                       *buffList[1];
    GT_U32                      buffLenList[1];
    GT_U32                      numOfBufs               = 1;
    GT_U32                      cookie                  = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily               = 0;
    GT_U8                       packetHeader[14] = {0x07, 0x17, 0x27, 0x37, 0x47, 0x57, 0x08, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97};
    GT_U32                      numOfNetIfs = 1;

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
            ITERATE_DEVICES (DxChx)
            ======= DSA_CMD_TO_CPU =======
            1.1. Call with pcktParamsPtr{
                              packetIsTagged [GT_TRUE],
                              cookie [],
                              sdmaInfo {recalcCrc [GT_TRUE],
                                        txQueue [0],
                                        invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                              dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                                      vpt[0],
                                                      cfiBit [0],
                                                      vid [0],
                                                      dropOnSource [GT_FALSE],
                                                      packetIsLooped [GT_FALSE]}
                                        dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
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
        */
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged = GT_TRUE;
        pcktParams.cookie         = &cookie;

        pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
        pcktParams.sdmaInfo.txQueue   = 0;
        pcktParams.sdmaInfo.evReqHndl = 0;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }
        pcktParams.dsaParam.commonParams.vpt            = 0;
        pcktParams.dsaParam.commonParams.cfiBit         = 0;
        pcktParams.dsaParam.commonParams.vid            = 0;
        pcktParams.dsaParam.commonParams.dropOnSource   = GT_FALSE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;

        pcktParams.dsaParam.dsaInfo.toCpu.isEgressPipe    = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.isTagged        = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum          = dev;
        pcktParams.dsaParam.dsaInfo.toCpu.srcIsTrunk      = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.interface.srcTrunkId = 2;
        pcktParams.dsaParam.dsaInfo.toCpu.cpuCode         = CPSS_NET_CONTROL_E;
        pcktParams.dsaParam.dsaInfo.toCpu.wasTruncated    = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.originByteCount = 0;
        pcktParams.dsaParam.dsaInfo.toCpu.timestamp       = 0;

        pcktParams.dsaParam.dsaInfo.toCpu.packetIsTT      = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = 0;
        pcktParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex   = 0;

        /*
            1.3. Call with out of range pcktParamsPtr->sdmaInfo.txQueue [8]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        pcktParams.sdmaInfo.txQueue = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->sdmaInfo.txQueue = %d",
                                         dev, pcktParams.sdmaInfo.txQueue);

        /* skip MII and not connected to CPU devices */
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
            continue;

        pcktParams.sdmaInfo.txQueue = 0;

        /*
            1.4. Call with out of range pcktParamsPtr->dsaParam.commonParams.dsaTagType [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.commonParams.dsaTagType);

        /*
            1.5. Call with out of range pcktParamsPtr->dsaParam.commonParams.vpt [8]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.commonParams.vpt = 8;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->dsaParam.commonParams.vpt = %d",
                                         dev, pcktParams.dsaParam.commonParams.vpt);

        pcktParams.dsaParam.commonParams.vpt = 0;

        /*
            1.6. Call with out of range pcktParamsPtr->dsaParam.commonParams.cfiBit [2]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.commonParams.cfiBit = 2;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->dsaParam.commonParams.cfiBit = %d",
                                         dev, pcktParams.dsaParam.commonParams.cfiBit);

        pcktParams.dsaParam.commonParams.cfiBit = 0;

        /*
            1.7. Call with out of range pcktParamsPtr->dsaParam.commonParams.vid [4096]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (pcktParams.dsaParam.commonParams.dsaTagType != CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT)
        {
            pcktParams.dsaParam.commonParams.vid = 4096;

            st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, ->dsaParam.commonParams.vid = %d",
                                         dev, pcktParams.dsaParam.commonParams.vid);

            pcktParams.dsaParam.commonParams.vid = 100;
        }
        /*
            1.8. Call with out of range pcktParamsPtr->dsaParam.dsaType [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaType);

        /*
            1.9. Call with out of range pcktParamsPtr->dsaParam.toCpu.hwDevNum [128]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, ->dsaParam.toCpu.hwDevNum = %d",
                                     dev, pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum);

        pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum = dev;
        pcktParams.dsaParam.dsaInfo.toCpu.srcIsTrunk = 2;

        /*
            1.11. Call with out of range pcktParamsPtr->dsaParam.toCpu.cpuCode [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaInfo.toCpu.cpuCode);

        pcktParams.dsaParam.dsaInfo.toCpu.packetIsTT = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = 0;

        /*
            1.14. Call with pcktParams->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[BIT_20]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = BIT_20;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = 0;

        /*
            1.16. Call with pcktParams->dsaParam.dsaInfo.toCpu.tag0TpidIndex[BIT_3]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex = BIT_3;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex = 0;

        /*
            ======= DSA_CMD_FROM_CPU =======
            1.17. Call with pcktParamsPtr {
                              packetIsTagged [GT_FALSE],
                              cookie [],
                              sdmaInfo {recalcCrc [GT_FALSE],
                                        txQueue [3],
                                        invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                              dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                      vpt[3],
                                                      cfiBit [1],
                                                      vid [100],
                                                      dropOnSource [GT_TRUE],
                                                      packetIsLooped [GT_TRUE]}
                                        dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
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
                              buffLenList[sizeof(packetBuffsArr)],
                              numOfBufs [1].
            Expected: GT_OK.
        */

        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged     = GT_FALSE;
        pcktParams.cookie             = &cookie;

        pcktParams.sdmaInfo.recalcCrc = GT_FALSE;
        pcktParams.sdmaInfo.txQueue   = 3;
        pcktParams.sdmaInfo.evReqHndl = 0;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }
        pcktParams.dsaParam.commonParams.vpt            = 3;
        pcktParams.dsaParam.commonParams.cfiBit         = 1;
        pcktParams.dsaParam.commonParams.vid            = 100;
        pcktParams.dsaParam.commonParams.dropOnSource   = GT_TRUE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_TRUE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

        pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum  = dev;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 0;

        pcktParams.dsaParam.dsaInfo.fromCpu.tc = 7;
        pcktParams.dsaParam.dsaInfo.fromCpu.dp = CPSS_DP_GREEN_E;
        pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterEn         = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.cascadeControl      = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterRegistered = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.srcId  = 0;
        pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = dev;

        pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstEport = 0;
        pcktParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = 0;
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;

        /*
            1.19. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid[GT_TRUE],
                out of range pcktParams->dsaParam.dsaInfo.fromCpu.dstEport[BIT_17]
                and other valid params same as 1.17.
            Expected: NOT GT_OK for Bobcat2; Caelum; Bobcat3.
        */
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstEport = BIT_17;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstEport = 0;

        /*
            1.20. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid[GT_TRUE],
                out of range pcktParams->dsaParam.dsaInfo.fromCpu.tag0TpidIndex[BIT_3]
                and other valid params same as 1.17.
            Expected: NOT GT_OK.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
            pcktParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = BIT_3;

            st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);

            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

            /* restore previous value */
            pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;
            pcktParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = 0;
        }

        /*
            1.22. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VID_E],
                pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.vid[100](same as commonParams.vid),
                pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludeInterface[GT_TRUE],
                pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludedInterface.type[CPSS_INTERFACE_TRUNK_E],
                pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludedInterface.trunkId[2],
                out of range enum value pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged
                and other valid params same as 1.17.
            Expected: GT_BAD_PARAM.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_VID_E;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 100;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface = GT_TRUE;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type = CPSS_INTERFACE_TRUNK_E;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId = 2;

            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                                (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                                pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged);

            /* restore previous value */
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum  = dev;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId = 0;
        }

        /*
            1.23. Call with out of range pcktParamsPtr->dsaParam.fromCpu.dstInterface.type [wrong enum values]
                and other valid params same as 1.17.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type);

        /*
            1.24. Call with out of range pcktParamsPtr->dsaParam.fromCpu.tc [8]
                and other valid params same as 1.17.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.fromCpu.tc = 8;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.fromCpu.tc = 0;

        /*
            1.26. Call with out of range pcktParamsPtr->dsaParam.fromCpu.srcHwDev [128]
                and other valid params same as 1.17.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = dev;

        /*
            ======= DSA_CMD_TO_ANALYZER =======
            1.27. Call with pcktParamsPtr {packetIsTagged [GT_TRUE],
                                          cookie [],
                                          sdmaInfo {recalcCrc [GT_FALSE],
                                                    txQueue [5],
                                                    invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                                          dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                                                  vpt[5],
                                                                  cfiBit [0],
                                                                  vid [1000],
                                                                  dropOnSource [GT_FALSE],
                                                                  packetIsLooped [GT_TRUE]}
                                                    dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
                                                    dsaInfo.toAnalyzer {rxSniffer [GT_TRUE],
                                                                        isTagged [GT_TRUE],
                                                                        devPort.hwDevNum [dev],
                                                                        devPort.portNum [0]}}},
                              packetBuffsArr[{0}],
                              buffLenList[sizeof(packetBuffsArr)],
                              numOfBufs [1].
            Expected: GT_OK.
        */
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged                    = GT_TRUE;
        pcktParams.cookie                            = &cookie;

        pcktParams.sdmaInfo.recalcCrc                = GT_FALSE;
        pcktParams.sdmaInfo.txQueue                  = 5;
        pcktParams.sdmaInfo.evReqHndl                = 0;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }
        pcktParams.dsaParam.commonParams.vpt            = 5;
        pcktParams.dsaParam.commonParams.cfiBit         = 0;
        pcktParams.dsaParam.commonParams.vid            = 1000;
        pcktParams.dsaParam.commonParams.dropOnSource   = GT_FALSE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_TRUE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E;

        pcktParams.dsaParam.dsaInfo.toAnalyzer.rxSniffer = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.isTagged  = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.hwDevNum = dev;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.portNum = 0;

        /*
            1.28. Call with out of range pcktParamsPtr->dsaParam.toAnalyzer.devPort.hwDevNum [128]
                and other valid params same as 1.27.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.hwDevNum = dev;

        /*
            1.29. Call with out of range pcktParamsPtr->dsaParam.toAnalyzer.devPort.portNum [64]
                and other valid params same as 1.27.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.portNum = CPSS_MAX_PORTS_NUM_CNS;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.portNum = 0;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = 0;

        /*
            1.31. Call with pcktParams->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[BIT_3]
                and other valid params same as 1.27.
            Expected: NOT GT_OK for Bobcat2; Caelum; Bobcat3.
        */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = BIT_3;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = 0;

        /*
            ======= DSA_CMD_FORWARD =======
            1.32. Call with pcktParamsPtr {packetIsTagged [GT_FALSE],
                                          cookie [],
                                          sdmaInfo {recalcCrc [GT_TRUE],
                                                    txQueue [7],
                                                    invokeTxBufferQueueEvent [GT_FALSE] (not relevant)},
                                          dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                                  vpt[7],
                                                                  cfiBit [1],
                                                                  vid [0xFFF],
                                                                  dropOnSource [GT_TRUE],
                                                                  packetIsLooped [GT_FALSE]}
                                                    dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                                                    dsaInfo.forward {srcIsTagged [CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E],
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
                              buffLenList[sizeof(packetBuffsArr)],
                              numOfBufs [1].
            Expected: GT_OK.
        */
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged       = GT_FALSE;
        pcktParams.cookie               = &cookie;

        pcktParams.sdmaInfo.recalcCrc   = GT_TRUE;
        pcktParams.sdmaInfo.txQueue     = 7;
        pcktParams.sdmaInfo.evReqHndl   = 0;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }
        pcktParams.dsaParam.commonParams.vpt            = 7;
        pcktParams.dsaParam.commonParams.cfiBit         = 1;

        if (pcktParams.dsaParam.commonParams.dsaTagType == CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT)
        {
            pcktParams.dsaParam.commonParams.vid = 0xFFFF;
        }
        else
        {
            pcktParams.dsaParam.commonParams.vid = 0xFFF;
        }

        pcktParams.dsaParam.commonParams.dropOnSource   = GT_TRUE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;

        pcktParams.dsaParam.dsaInfo.forward.srcIsTagged     = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
        pcktParams.dsaParam.dsaInfo.forward.srcHwDev          = dev;
        pcktParams.dsaParam.dsaInfo.forward.srcIsTrunk      = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.forward.source.trunkId  = 2;
        pcktParams.dsaParam.dsaInfo.forward.srcId           = 0;
        pcktParams.dsaParam.dsaInfo.forward.egrFilterRegistered = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.forward.wasRouted       = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.forward.qosProfileIndex = 0;

        pcktParams.dsaParam.dsaInfo.forward.dstInterface.type   = CPSS_INTERFACE_VID_E;
        pcktParams.dsaParam.dsaInfo.forward.dstInterface.vlanId = 0xFFF;
        pcktParams.dsaParam.dsaInfo.forward.srcIsTagged = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;

        /*
            1.34. Call with out of range enum value
                pcktParamsPtr->dsaParam.dsaInfo.forward.srcIsTagged
                and other valid params same as 1.32.
            Expected: GT_BAD_PARAM.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                                (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                                pcktParams.dsaParam.dsaInfo.forward.srcIsTagged);
        }

        /*
            1.35. Call with out of range pcktParamsPtr->dsaParam.forward.srcHwDev [128]
                and other valid params same as 1.32.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.forward.srcHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.forward.srcHwDev = dev;

        /*
            1.36. Call with out of range pcktParamsPtr->dsaParam.dsaInfo.forward.source.trunkId [128]
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.forward.source.trunkId = 4096;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.forward.source.trunkId = 2;

        /*
            1.37. Call with out of range pcktParamsPtr->dsaParam.dsaInfo.forward.dstInterface.type [wrong enum values]
                and other valid params same as 1.32.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaInfo.forward.dstInterface.type);

        pcktParams.dsaParam.dsaInfo.forward.tag0TpidIndex = 0;

        /*
            1.39. Call with pcktParams->dsaParam.dsaInfo.forward.tag0TpidIndex[BIT_3]
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.forward.tag0TpidIndex = BIT_3;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.forward.tag0TpidIndex = 0;
        pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;

        /*
            1.43. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_TRUE],
                out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.trunkId[4096](is relevant)
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_TRUE;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.trunkId = 4096;

            st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);

            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

            /* restore previous value */
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.trunkId = 0;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
        }

        /*
            1.44. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_TRUE],
                out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096](is relevant)
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_TRUE;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.trunkId = 4096;

            st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);

            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

            /* restore previous value */
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.portNum = 0;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
        }

        pcktParams.dsaParam.dsaInfo.forward.phySrcMcFilterEnable = GT_FALSE;

        /*
            1.46. Call with pcktParamsPtr [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, NULL, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pcktParamsPtr = NULL", dev);

        /*
            1.47. Call with buffList [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, NULL,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffList = NULL", dev);

        /*
            1.48. Call with buffLenList [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
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
        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                               buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                           buffLenList, numOfBufs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

#ifdef ASIC_SIMULATION
    /* allow all packets to egress the devices */
    cpssOsTimerWkAfter(500);
#endif /*ASIC_SIMULATION*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_PTR             *cookiePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct param value.
    Expected: GT_OK.
    1.2. Call with wrong cookiePtr pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U8       txQueue = 0;
    GT_PTR      cookiePtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct params.
            Expected: GT_OK.
        */

        /*st = cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet(dev, txQueue, cookiePtr);*/
        /*UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);*/

        /*
            1.2. Call with wrong cookiePtr pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet(dev, txQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet(dev, txQueue, &cookiePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet(dev, txQueue, &cookiePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_U32             *numberOfFreeTxDescriptorsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet)
{
/*
    ITERATE_DEVICES (DxChx)
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
    GT_U8       queueIdx = 0;
    GT_U32      numberOfFreeTxDescriptors;
    GT_U32      numOfNetIfs;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.1. Call with txQueue [0].
            Expected: GT_OK.
        */

        st = cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue,
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
        queueIdx = (GT_U8)(8 * numOfNetIfs);
        st = cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet(
            dev, queueIdx, &numberOfFreeTxDescriptors);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call with wrong numberOfFreeTxDescriptorsPtr pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue,
                                                        &numberOfFreeTxDescriptors);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet(dev, txQueue,
                                                    &numberOfFreeTxDescriptors);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxPacketSend)
{
/*
    ITERATE_DEVICES (DxChx)
    ======= DSA_CMD_TO_CPU =======
    1.1. Call with pcktParamsPtr{
                      packetIsTagged [GT_TRUE],
                      cookie [],
                      sdmaInfo {recalcCrc [GT_TRUE],
                                txQueue [1],
                                evReqHndl[evReqHndl]
                                invokeTxBufferQueueEvent [GT_TRUE]},
                      dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                              vpt[0],
                                              cfiBit [0],
                                              vid [0],
                                              dropOnSource [GT_FALSE],
                                              packetIsLooped [GT_FALSE]}
                                dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
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
                      sdmaInfo {recalcCrc [GT_TRUE],
                                txQueue [1],
                                evReqHndl[evReqHndl]
                                invokeTxBufferQueueEvent [GT_TRUE]},
                      dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                              vpt[3],
                                              cfiBit [1],
                                              vid [100],
                                              dropOnSource [GT_TRUE],
                                              packetIsLooped [GT_TRUE]}
                                dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
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
        pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged[CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E]
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
                                  sdmaInfo {recalcCrc [GT_TRUE],
                                            txQueue [1],
                                            evReqHndl[evReqHndl]
                                            invokeTxBufferQueueEvent [GT_TRUE]},
                                  dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                                          vpt[5],
                                                          cfiBit [0],
                                                          vid [1000],
                                                          dropOnSource [GT_FALSE],
                                                          packetIsLooped [GT_TRUE]}
                                            dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
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
                                            txQueue [1],
                                            evReqHndl[evReqHndl]
                                            invokeTxBufferQueueEvent [GT_TRUE]},
                                  dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                          vpt[7],
                                                          cfiBit [1],
                                                          vid [0xFFF],
                                                          dropOnSource [GT_TRUE],
                                                          packetIsLooped [GT_FALSE]}
                                            dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                                            dsaInfo.forward {srcIsTagged [CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E],
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
    1.33. Call with pcktParamsPtr->dsaParam.dsaInfo.forward.dstInterface.srcIsTagged[CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E]
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
    GT_U32                      numOfBufs               = 1;
    GT_U32                      cookie                  = 0;
    CPSS_DXCH_NET_TX_PARAMS_STC pcktParams;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily               = 0;
    CPSS_UNI_EV_CAUSE_ENT       evCause[1] = { CPSS_PP_TX_BUFFER_QUEUE_E };
    GT_U8                       *buffList[1];
    GT_U32                      buffLenList[1];
    GT_U8                       packetHeader[12] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    GT_UINTPTR                  evReqHndl;
    GT_U32                      numOfNetIfs = 1;

    buffList[0] = cpssOsCacheDmaMalloc(64);
    buffLenList[0] = 64;
    cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
    cpssOsBzero((GT_VOID*) buffList[0], 64);
    cpssOsMemCpy(buffList[0], packetHeader, sizeof(packetHeader));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    st = cpssEventBind(evCause, 1, &evReqHndl);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            ITERATE_DEVICES (DxChx)
            ======= DSA_CMD_TO_CPU =======
            1.1. Call with pcktParamsPtr{
                              packetIsTagged [GT_TRUE],
                              cookie [],
                              sdmaInfo {recalcCrc [GT_TRUE],
                                        txQueue [1],
                                        evReqHndl[evReqHndl]
                                        invokeTxBufferQueueEvent [GT_TRUE]},
                              dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                                      vpt[0],
                                                      cfiBit [0],
                                                      vid [0],
                                                      dropOnSource [GT_FALSE],
                                                      packetIsLooped [GT_FALSE]}
                                        dsaType [CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
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
        */
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged = GT_TRUE;
        pcktParams.cookie         = &cookie;

        pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
        pcktParams.sdmaInfo.txQueue = 1;
        pcktParams.sdmaInfo.evReqHndl = evReqHndl;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_TRUE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }

        pcktParams.dsaParam.commonParams.vpt            = 0;
        pcktParams.dsaParam.commonParams.cfiBit         = 0;
        pcktParams.dsaParam.commonParams.vid            = 0;
        pcktParams.dsaParam.commonParams.dropOnSource   = GT_FALSE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;

        pcktParams.dsaParam.dsaInfo.toCpu.isEgressPipe    = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.isTagged        = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum          = dev;
        pcktParams.dsaParam.dsaInfo.toCpu.srcIsTrunk      = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.interface.srcTrunkId = 2;
        pcktParams.dsaParam.dsaInfo.toCpu.cpuCode         = CPSS_NET_CONTROL_E;
        pcktParams.dsaParam.dsaInfo.toCpu.wasTruncated    = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toCpu.originByteCount = 0;
        pcktParams.dsaParam.dsaInfo.toCpu.timestamp       = 0;

        pcktParams.dsaParam.dsaInfo.toCpu.packetIsTT      = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = 0;
        pcktParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex   = 0;
        pcktParams.cookie = &cookie;

        /*
            1.3. Call with out of range pcktParamsPtr->sdmaInfo.txQueue [8]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        pcktParams.sdmaInfo.txQueue = (GT_U8)(8 * numOfNetIfs);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->sdmaInfo.txQueue = %d",
                                         dev, pcktParams.sdmaInfo.txQueue);

        /* skip MII and not connected to CPU devices */
        if (!UTF_CPSS_PP_IS_SDMA_USED_MAC(dev))
            continue;

        pcktParams.sdmaInfo.txQueue = 0;

        /*
            1.4. Call with out of range pcktParamsPtr->dsaParam.commonParams.dsaTagType [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.commonParams.dsaTagType);

        /*
            1.5. Call with out of range pcktParamsPtr->dsaParam.commonParams.vpt [8]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.commonParams.vpt = 8;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->dsaParam.commonParams.vpt = %d",
                                         dev, pcktParams.dsaParam.commonParams.vpt);

        pcktParams.dsaParam.commonParams.vpt = 0;

        /*
            1.6. Call with out of range pcktParamsPtr->dsaParam.commonParams.cfiBit [2]
                and other valid params same as 1.1.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.commonParams.cfiBit = 2;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->dsaParam.commonParams.cfiBit = %d",
                                         dev, pcktParams.dsaParam.commonParams.cfiBit);

        pcktParams.dsaParam.commonParams.cfiBit = 0;

        /*
            1.7. Call with out of range pcktParamsPtr->dsaParam.commonParams.vid [4096]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (pcktParams.dsaParam.commonParams.dsaTagType != CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT)
        {
            pcktParams.dsaParam.commonParams.vid = 4096;

            st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, ->dsaParam.commonParams.vid = %d",
                                         dev, pcktParams.dsaParam.commonParams.vid);

            pcktParams.dsaParam.commonParams.vid = 100;
        }
        /*
            1.8. Call with out of range pcktParamsPtr->dsaParam.dsaType [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaType);

        /*
            1.9. Call with out of range pcktParamsPtr->dsaParam.toCpu.hwDevNum [128]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, ->dsaParam.toCpu.hwDevNum = %d",
                                     dev, pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum);

        pcktParams.dsaParam.dsaInfo.toCpu.hwDevNum = dev;
        pcktParams.dsaParam.dsaInfo.toCpu.srcIsTrunk = 1;

        /*
            1.11. Call with out of range pcktParamsPtr->dsaParam.toCpu.cpuCode [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaInfo.toCpu.cpuCode);

        pcktParams.dsaParam.dsaInfo.toCpu.packetIsTT = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = 0;

        /*
            1.14. Call with pcktParams->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[BIT_20]
                and other valid params same as 1.1.
            Expected: NOT GT_OK for Bobcat2; Caelum; Bobcat3.
        */
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = BIT_20;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = 0;
        pcktParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex = 0;

        /*
            1.16. Call with pcktParams->dsaParam.dsaInfo.toCpu.tag0TpidIndex[BIT_3]
                and other valid params same as 1.1.
            Expected: NOT GT_OK for Bobcat2; Caelum; Bobcat3.
        */
        pcktParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex = BIT_3;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex = 0;

        /*
            ======= DSA_CMD_FROM_CPU =======
            1.17. Call with pcktParamsPtr {
                              packetIsTagged [GT_FALSE],
                              cookie [],
                              sdmaInfo {recalcCrc [GT_TRUE],
                                        txQueue [1],
                                        evReqHndl[evReqHndl]
                                        invokeTxBufferQueueEvent [GT_TRUE]},
                              dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                      vpt[3],
                                                      cfiBit [1],
                                                      vid [100],
                                                      dropOnSource [GT_TRUE],
                                                      packetIsLooped [GT_TRUE]}
                                        dsaType [CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
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
                              buffLenList[sizeof(packetBuffsArr)],
                              numOfBufs [1].
            Expected: GT_OK.
        */

        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged     = GT_FALSE;
        pcktParams.cookie             = &cookie;

        pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
        pcktParams.sdmaInfo.txQueue = 1;
        pcktParams.sdmaInfo.evReqHndl = evReqHndl;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_TRUE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }
        pcktParams.dsaParam.commonParams.vpt            = 3;
        pcktParams.dsaParam.commonParams.cfiBit         = 1;
        pcktParams.dsaParam.commonParams.vid            = 100;
        pcktParams.dsaParam.commonParams.dropOnSource   = GT_TRUE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_TRUE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

        pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum  = dev;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 0;

        pcktParams.dsaParam.dsaInfo.fromCpu.tc = 7;
        pcktParams.dsaParam.dsaInfo.fromCpu.dp = CPSS_DP_GREEN_E;
        pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterEn         = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.cascadeControl      = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterRegistered = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.srcId  = 0;
        pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = dev;

        pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstEport = 0;
        pcktParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = 0;

        /*
            1.19. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid[GT_TRUE],
                out of range pcktParams->dsaParam.dsaInfo.fromCpu.dstEport[BIT_17]
                and other valid params same as 1.17.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstEport = BIT_17;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.fromCpu.dstEport = 0;

        /*
            1.20. call with pcktParams->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid[GT_TRUE],
                out of range pcktParams->dsaParam.dsaInfo.fromCpu.tag0TpidIndex[BIT_3]
                and other valid params same as 1.17.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = BIT_3;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                               buffLenList, numOfBufs);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;
        pcktParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = 0;

        /*
            1.22. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.type[CPSS_INTERFACE_VID_E],
                pcktParams->dsaParam.dsaInfo.fromCpu.dstInterface.vid[100](same as commonParams.vid),
                pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludeInterface[GT_TRUE],
                pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludedInterface.type[CPSS_INTERFACE_TRUNK_E],
                pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.excludedInterface.trunkId[2],
                out of range enum value pcktParams->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged
                and other valid params same as 1.17.
            Expected: GT_BAD_PARAM.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_VID_E;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 100;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface = GT_TRUE;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type = CPSS_INTERFACE_TRUNK_E;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId = 2;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;

            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                                (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                                pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged);

            /* restore previous value */
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum  = dev;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged = 0;
        }

        /*
            1.23. Call with out of range pcktParamsPtr->dsaParam.fromCpu.dstInterface.type [wrong enum values]
                and other valid params same as 1.17.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type);

        /*
            1.24. Call with out of range pcktParamsPtr->dsaParam.fromCpu.tc [8]
                and other valid params same as 1.17.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.fromCpu.tc = 8;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.fromCpu.tc = 0;

        /*
            1.26. Call with out of range pcktParamsPtr->dsaParam.fromCpu.srcHwDev [128]
                and other valid params same as 1.17.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = dev;

        /*
            ======= DSA_CMD_TO_ANALYZER =======
            1.27. Call with pcktParamsPtr {packetIsTagged [GT_TRUE],
                                          cookie [],
                                          sdmaInfo {recalcCrc [GT_TRUE],
                                                    txQueue [1],
                                                    evReqHndl[evReqHndl]
                                                    invokeTxBufferQueueEvent [GT_TRUE]},
                                          dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_REGULAR_E],
                                                                  vpt[5],
                                                                  cfiBit [0],
                                                                  vid [1000],
                                                                  dropOnSource [GT_FALSE],
                                                                  packetIsLooped [GT_TRUE]}
                                                    dsaType [CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
                                                    dsaInfo.toAnalyzer {rxSniffer [GT_TRUE],
                                                                        isTagged [GT_TRUE],
                                                                        devPort.hwDevNum [dev],
                                                                        devPort.portNum [0]}}},
                              packetBuffsArr[{0}],
                              buffLenList[sizeof(packetBuffsArr)],
                              numOfBufs [1].
            Expected: GT_OK.
        */
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged                    = GT_TRUE;
        pcktParams.cookie                            = &cookie;

        pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
        pcktParams.sdmaInfo.txQueue = 1;
        pcktParams.sdmaInfo.evReqHndl = evReqHndl;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_TRUE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }
        pcktParams.dsaParam.commonParams.vpt            = 5;
        pcktParams.dsaParam.commonParams.cfiBit         = 0;
        pcktParams.dsaParam.commonParams.vid            = 1000;
        pcktParams.dsaParam.commonParams.dropOnSource   = GT_FALSE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_TRUE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E;

        pcktParams.dsaParam.dsaInfo.toAnalyzer.rxSniffer = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.isTagged  = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.hwDevNum = dev;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.portNum = 0;

        /*
            1.28. Call with out of range pcktParamsPtr->dsaParam.toAnalyzer.devPort.hwDevNum [128]
                and other valid params same as 1.27.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.hwDevNum = dev;

        /*
            1.29. Call with out of range pcktParamsPtr->dsaParam.toAnalyzer.devPort.portNum [64]
                and other valid params same as 1.27.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.portNum = CPSS_MAX_PORTS_NUM_CNS;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.devPort.portNum = 0;
        pcktParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = 0;

        /*
            1.31. Call with pcktParams->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[BIT_3]
                and other valid params same as 1.27.
            Expected: NOT GT_OK for Bobcat2; Caelum; Bobcat3.
        */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = BIT_3;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = 0;

        /*
            ======= DSA_CMD_FORWARD =======
            1.32. Call with pcktParamsPtr {packetIsTagged [GT_FALSE],
                                          cookie [],
                                          sdmaInfo {recalcCrc [GT_TRUE],
                                                    txQueue [1],
                                                    evReqHndl[evReqHndl]
                                                    invokeTxBufferQueueEvent [GT_TRUE]},
                                          dsaParam {commonParams {dsaTagType [CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E],
                                                                  vpt[7],
                                                                  cfiBit [1],
                                                                  vid [0xFFF],
                                                                  dropOnSource [GT_TRUE],
                                                                  packetIsLooped [GT_FALSE]}
                                                    dsaType [CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                                                    dsaInfo.forward {srcIsTagged [CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E],
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
                              buffLenList[sizeof(packetBuffsArr)],
                              numOfBufs [1].
            Expected: GT_OK.
        */
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));
        cpssOsBzero((GT_VOID*) &pcktParams, sizeof(pcktParams));

        pcktParams.packetIsTagged       = GT_FALSE;
        pcktParams.cookie               = &cookie;

        pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
        pcktParams.sdmaInfo.txQueue = 1;
        pcktParams.sdmaInfo.evReqHndl = evReqHndl;
        pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_TRUE;

        pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }
        pcktParams.dsaParam.commonParams.vpt            = 7;
        pcktParams.dsaParam.commonParams.cfiBit         = 1;
        if (pcktParams.dsaParam.commonParams.dsaTagType == CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT)
        {
            pcktParams.dsaParam.commonParams.vid = 0xFFFF;
        }
        else
        {
            pcktParams.dsaParam.commonParams.vid = 0xFFF;
        }
        pcktParams.dsaParam.commonParams.dropOnSource   = GT_TRUE;
        pcktParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;

        pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;

        pcktParams.dsaParam.dsaInfo.forward.srcIsTagged     = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
        pcktParams.dsaParam.dsaInfo.forward.srcHwDev          = dev;
        pcktParams.dsaParam.dsaInfo.forward.srcIsTrunk      = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.forward.source.trunkId  = 2;
        pcktParams.dsaParam.dsaInfo.forward.srcId           = 0;
        pcktParams.dsaParam.dsaInfo.forward.egrFilterRegistered = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.forward.wasRouted       = GT_TRUE;
        pcktParams.dsaParam.dsaInfo.forward.qosProfileIndex = 0;

        pcktParams.dsaParam.dsaInfo.forward.dstInterface.type   = CPSS_INTERFACE_VID_E;
        pcktParams.dsaParam.dsaInfo.forward.dstInterface.vlanId = 0xFFF;
        pcktParams.dsaParam.dsaInfo.forward.srcIsTagged = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;

        /*
            1.34. Call with out of range enum value
                pcktParamsPtr->dsaParam.dsaInfo.forward.srcIsTagged
                and other valid params same as 1.32.
            Expected: GT_BAD_PARAM.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                                (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                                pcktParams.dsaParam.dsaInfo.forward.srcIsTagged);
        }

        /*
            1.35. Call with out of range pcktParamsPtr->dsaParam.forward.srcHwDev [128]
                and other valid params same as 1.32.
            Expected: GT_BAD_PARAM.
        */
        pcktParams.dsaParam.dsaInfo.forward.srcHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.forward.srcHwDev = dev;

        /*
            1.36. Call with out of range pcktParamsPtr->dsaParam.dsaInfo.forward.source.trunkId [4096]
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.forward.source.trunkId = 4096;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.forward.source.trunkId = 2;

        /*
            1.37. Call with out of range pcktParamsPtr->dsaParam.dsaInfo.forward.dstInterface.type [wrong enum values]
                and other valid params same as 1.32.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaSyncTxPacketSend
                            (dev, &pcktParams, buffList, buffLenList, numOfBufs),
                            pcktParams.dsaParam.dsaInfo.forward.dstInterface.type);

        pcktParams.dsaParam.dsaInfo.forward.tag0TpidIndex = 0;

        /*
            1.39. Call with pcktParams->dsaParam.dsaInfo.forward.tag0TpidIndex[BIT_3]
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        pcktParams.dsaParam.dsaInfo.forward.tag0TpidIndex = BIT_3;

        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              buffLenList, numOfBufs);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /* restore previous value */
        pcktParams.dsaParam.dsaInfo.forward.tag0TpidIndex = 0;
        pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;


        /*
            1.43. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_TRUE],
                out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.trunkId[4096](is relevant)
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_TRUE;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.trunkId = 4096;

            st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);

            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

            /* restore previous value */
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.trunkId = 0;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
        }

        /*
            1.44. For Bobcat2; Caelum; Bobcat3 call with pcktParams->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_TRUE],
                out of range value pcktParams->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096](is relevant)
                and other valid params same as 1.32.
            Expected: NOT GT_OK.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_TRUE;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.trunkId = 4096;

            st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                                  buffLenList, numOfBufs);

            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

            /* restore previous value */
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.portNum = 0;
            pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
        }

        pcktParams.dsaParam.dsaInfo.forward.phySrcMcFilterEnable = GT_FALSE;

        /*
            1.46. Call with pcktParamsPtr [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, NULL, buffList,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pcktParamsPtr = NULL", dev);

        /*
            1.47. Call with buffList [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, NULL,
                                              buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffList = NULL", dev);

        /*
            1.48. Call with buffLenList [NULL] and other valid params same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                              NULL, numOfBufs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffLenList = NULL", dev);
    }

    {
        /* The interrupt locking is needed for mutual exclusion prevention between   */
        /* ISR and user task on the event queues.                                    */

        (void)cpssEventDestroy(evReqHndl);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxPacketSend(dev, &pcktParams, buffList,
                                               buffLenList, numOfBufs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChNetIfSdmaTxPacketSend(dev, &pcktParams, buffList,
                                           buffLenList, numOfBufs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
#ifdef ASIC_SIMULATION
    /* allow all packets to egress the devices */
    cpssOsTimerWkAfter(500);
#endif /*ASIC_SIMULATION*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfTxBufferQueueGet
(
    IN  GT_U32               hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfTxBufferQueueGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct param value.
    Expected: GT_OK.
    1.2. Call with wrong devPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with wrong cookiePtr  [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong queuePtr  [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with wrong statusPtr  [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U32      hndl = 0;
    GT_U8       dev;
    GT_PTR      cookie;
    GT_U8       queue;
    GT_STATUS   status;

    /*
        1.1. Call with correct param value.
        Expected: GT_OK.
    */

    /*st = cpssDxChNetIfTxBufferQueueGet(hndl, &dev, &cookie, &queue, &status);*/
    /*UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);*/

    /*
        1.2. Call with wrong buffLenList pointer [NULL].
        Expected: GT_BAD_PTR.
    */
    st = cpssDxChNetIfTxBufferQueueGet(hndl, NULL, &cookie, &queue, &status);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    /*
        1.3. Call with wrong buffLenList pointer [NULL].
        Expected: GT_BAD_PTR.
    */
    st = cpssDxChNetIfTxBufferQueueGet(hndl, &dev, NULL, &queue, &status);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    /*
        1.4. Call with wrong buffLenList pointer [NULL].
        Expected: GT_BAD_PTR.
    */
    st = cpssDxChNetIfTxBufferQueueGet(hndl, &dev, &cookie, NULL, &status);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    /*
        1.5. Call with wrong buffLenList pointer [NULL].
        Expected: GT_BAD_PTR.
    */
    st = cpssDxChNetIfTxBufferQueueGet(hndl, &dev, &cookie, &queue, NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterDropCntrGet
(
    IN  GT_U8   dev,
    OUT GT_U32  *dropCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeRateLimiterDropCntrGet)
{
/*
    ITERATE_DEVICES(DxCh2 and above)
    1.1. Call with not null dropCntrPtr.
    Expected: GT_OK.
    1.2. Call api with wrong dropCntrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_U32  dropCntr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null dropCntrPtr.
            Expected: GT_OK.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(dev, &dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong dropCntrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropCntrPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(dev, &dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(dev, &dropCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterDropCntrSet
(
    IN  GT_U8  dev,
    IN  GT_U32 dropCntrVal
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeRateLimiterDropCntrSet)
{
/*
    ITERATE_DEVICES(DxCh2 and above)
    1.1. Call with dropCntrVal[0 / 100 / 555],
    Expected: GT_OK.
    1.2. Call cpssDxChNetIfCpuCodeRateLimiterDropCntrGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8  dev;
    GT_U32 dropCntr = 0;
    GT_U32 dropCntrGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with dropCntr[0 / 100 / 555],
            Expected: GT_OK.
        */
        /* call with dropCntr[0] */
        dropCntr = 0;

        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrSet(dev, dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChNetIfCpuCodeRateLimiterDropCntrGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(dev, &dropCntrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChNetIfCpuCodeRateLimiterDropCntrGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dropCntr, dropCntrGet,
                "got another dropCntr than was set: %d", dev);

        /* call with dropCntrVal[100] */
        dropCntr = 100;

        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrSet(dev, dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChNetIfCpuCodeRateLimiterDropCntrGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(dev, &dropCntrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChNetIfCpuCodeRateLimiterDropCntrGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dropCntr, dropCntrGet,
                "got another dropCntr than was set: %d", dev);

        /* call with dropCntr[555] */
        dropCntr = 555;

        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrSet(dev, dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChNetIfCpuCodeRateLimiterDropCntrGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(dev, &dropCntrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChNetIfCpuCodeRateLimiterDropCntrGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dropCntr, dropCntrGet,
                "got another dropCntr than was set: %d", dev);
    }

    /* restore correct values */
    dropCntr = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeRateLimiterDropCntrSet(dev, dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeRateLimiterDropCntrSet(dev, dropCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet
(
    IN  GT_U8   dev,
    IN  GT_U32  rateLimiterIndex,
    OUT GT_U32  *packetCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet)
{
/*
    ITERATE_DEVICES(DxCh2 and above)
    1.1. Call with rateLimiterIndex[1 / 10 / rateLimiterIndexMax],
    Expected: GT_OK.
    1.2. Call api with wrong rateLimiterIndex [0 / rateLimiterIndexMax + 1].
    Expected: NOT GT_OK.
    1.3. Call api with wrong packetCntrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_U32  rateLimiterIndex = 0;
    GT_U32  packetCntr;
    GT_U32  rateLimiterIndexMax;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            rateLimiterIndexMax = 255;
        }
        else
        {
            rateLimiterIndexMax = 31;
        }

        /*
            1.1. Call with rateLimiterIndex[1 / 10 / rateLimiterIndexMax],
            Expected: GT_OK.
        */

        /* call with rateLimiterIndex[1] */
        rateLimiterIndex = 1;

        st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with rateLimiterIndex[10] */
        rateLimiterIndex = 10;

        st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with rateLimiterIndex[31] */
        rateLimiterIndex = rateLimiterIndexMax;

        st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong rateLimiterIndex [0 / rateLimiterIndexMax + 1].
            Expected: NOT GT_OK.
        */
        /* call with rateLimiterIndex[rateLimiterIndexMax + 1] */
        rateLimiterIndex = rateLimiterIndexMax + 1;

        st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with rateLimiterIndex[0] */
        rateLimiterIndex = 0;

        st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        rateLimiterIndex = 1;

        /*
            1.3. Call api with wrong packetCntrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetCntrPtr = NULL", dev);
    }

    /* restore correct values */
    rateLimiterIndex = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(dev, rateLimiterIndex, &packetCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet
(
    IN  GT_U8               dev,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_U32              *dropCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with not null dropCntrPtr.
    Expected: GT_OK.
    1.1.2. Call api with wrong dropCntrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8               dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;
    GT_U32              dropCntr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with not null dropCntrPtr.
                Expected: GT_OK.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong dropCntrPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropCntrPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet
(
    IN  GT_U8              dev,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    IN  GT_U32             dropCntrVal
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with dropCntrVal[0 / 100 / 555],
    Expected: GT_OK.
    1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8              dev;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;
    GT_U32             dropCntr = 0;
    GT_U32             dropCntrGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with dropCntrVal[0 / 100 / 555],
                Expected: GT_OK.
            */

            /* call with dropCntrVal[0] */
            dropCntr = 0;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(dev,
                                     portGroupsBmp, dropCntr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntrGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(dropCntr, dropCntrGet,
                    "got another dropCntr than was set: %d", dev);

            /* call with dropCntrVal[100] */
            dropCntr = 100;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(dev,
                                     portGroupsBmp, dropCntr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntrGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(dropCntr, dropCntrGet,
                    "got another dropCntr than was set: %d", dev);

            /* call with dropCntrVal[555] */
            dropCntr = 555;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(dev,
                                     portGroupsBmp, dropCntr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(dev,
                                     portGroupsBmp, &dropCntrGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(dropCntr, dropCntrGet,
                    "got another dropCntr than was set: %d", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(dev,
                                     portGroupsBmp, dropCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(dev,
                                     portGroupsBmp, dropCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    dropCntr = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(dev,
                                     portGroupsBmp, dropCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(dev,
                                     portGroupsBmp, dropCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet
(
    IN  GT_U8               dev,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              rateLimiterIndex,
    OUT GT_U32              *packetCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with rateLimiterIndex[1 / 10 / 31],
    Expected: GT_OK.
    1.1.2. Call api with wrong rateLimiterIndex [1 / 32].
    Expected: NOT GT_OK.
    1.1.3. Call api with wrong packetCntrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8               dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;
    GT_U32              rateLimiterIndex = 0;
    GT_U32              packetCntr;
    GT_U32              rateLimiterIndexMax;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            rateLimiterIndexMax = 255;
        }
        else
        {
            rateLimiterIndexMax = 31;
        }

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with rateLimiterIndex[1 / 10 / rateLimiterIndexMax],
                Expected: GT_OK.
            */

            /* call with rateLimiterIndex[1] */
            rateLimiterIndex = 1;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with rateLimiterIndex[10] */
            rateLimiterIndex = 10;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with rateLimiterIndex[rateLimiterIndexMax] */
            rateLimiterIndex = rateLimiterIndexMax;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong rateLimiterIndex [0 / rateLimiterIndexMax + 1].
                Expected: NOT GT_OK.
            */
            /* call with rateLimiterIndex[rateLimiterIndexMax + 1] */
            rateLimiterIndex = rateLimiterIndexMax + 1;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with rateLimiterIndex[0] */
            rateLimiterIndex = 0;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            rateLimiterIndex = 1;

            /*
                1.1.3. Call api with wrong packetCntrPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetCntrPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    rateLimiterIndex = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(dev,
                            portGroupsBmp, rateLimiterIndex, &packetCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet
(
    IN  GT_U8                dev,
    IN  GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN  GT_U32               rateLimiterIndex,
    OUT GT_U32               *windowSizePtr,
    OUT GT_U32               *pktLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with rateLimiterIndex[1 / 10 / rateLimiterIndexMax],
    Expected: GT_OK.
    1.1.2. Call api with wrong rateLimiterIndex [0 / rateLimiterIndexMax + 1].
    Expected: NOT GT_OK.
    1.1.3. Call api with wrong windowSizePtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call api with wrong pktLimitPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8                dev;
    GT_PORT_GROUPS_BMP   portGroupsBmp = 1;
    GT_U32               rateLimiterIndex = 1;
    GT_U32               windowSize;
    GT_U32               pktLimit;
    GT_U32               rateLimiterIndexMax;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                rateLimiterIndexMax = 255;
            }
            else
            {
                rateLimiterIndexMax = 31;
            }

            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with rateLimiterIndex[1 / 10 / rateLimiterIndexMax],
                Expected: GT_OK.
            */

            /* call with rateLimiterIndex[1] */
            rateLimiterIndex = 1;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with rateLimiterIndex[10] */
            rateLimiterIndex = 10;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with rateLimiterIndex[rateLimiterIndexMax] */
            rateLimiterIndex = rateLimiterIndexMax;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong rateLimiterIndex [0 / rateLimiterIndexMax + 1].
                Expected: NOT GT_OK.
            */
            /* call with rateLimiterIndex[rateLimiterIndexMax + 1] */
            rateLimiterIndex = rateLimiterIndexMax + 1;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with rateLimiterIndex[0] */
            rateLimiterIndex = 0;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            rateLimiterIndex = 1;

            /*
                1.1.3. Call api with wrong windowSizePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, NULL, &pktLimit);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, windowSizePtr = NULL", dev);

            /*
                1.1.4. Call api with wrong pktLimitPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pktLimitPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    rateLimiterIndex = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSize, &pktLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet
(
    IN  GT_U8              dev,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    IN  GT_U32             rateLimiterIndex,
    IN  GT_U32             windowSize,
    IN  GT_U32             pktLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with rateLimiterIndex[1 / 10 / rateLimiterIndexMax],
                   windowSize[0 / 100 / 0xFFF],
                   pktLimit[0 / 100 / 0xFFFF],
    Expected: GT_OK.
    1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong rateLimiterIndex [0 / rateLimiterIndexMax + 1].
    Expected: NOT GT_OK.
    1.1.4. Call api with wrong windowSize [0xFFF + 1].
    Expected: NOT GT_OK.
    1.1.5. Call api with wrong pktLimit [0xFFFF + 1].
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8              dev;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;
    GT_U32             rateLimiterIndex = 1;
    GT_U32             windowSize = 0;
    GT_U32             pktLimit = 0;
    GT_U32             windowSizeGet = 0;
    GT_U32             pktLimitGet = 0;
    GT_U32             rateLimiterIndexMax;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            rateLimiterIndexMax = 255;
        }
        else
        {
            rateLimiterIndexMax = 31;
        }

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with rateLimiterIndex[1 / 10 / rateLimiterIndexMax],
                               windowSize[0 / 100 / 0xFFF],
                               pktLimit[0 / 100 / 0xFFFF],
                Expected: GT_OK.
            */
            /* call with rateLimiterIndex[1], windowSize[0], pktLimit[0] */
            rateLimiterIndex = 1;
            windowSize = 0;
            pktLimit = 0;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSizeGet, &pktLimitGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet,
                           "got another windowSize then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(pktLimit, pktLimitGet,
                           "got another pktLimit then was set: %d", dev);

            /* call with rateLimiterIndex[10], windowSize[100], pktLimit[100] */
            rateLimiterIndex = 10;
            windowSize = 100;
            pktLimit = 100;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSizeGet, &pktLimitGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet,
                           "got another windowSize then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(pktLimit, pktLimitGet,
                           "got another pktLimit then was set: %d", dev);

            /* call with rateLimiterIndex[31], windowSize[0xFFF], pktLimit[0xFFFF] */
            rateLimiterIndex = rateLimiterIndexMax;
            windowSize = 0xFFF;
            pktLimit = 0xFFFF;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(dev,
                   portGroupsBmp, rateLimiterIndex, &windowSizeGet, &pktLimitGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet,
                           "got another windowSize then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(pktLimit, pktLimitGet,
                           "got another pktLimit then was set: %d", dev);

            /*
                1.1.3. Call api with wrong rateLimiterIndex [0 / rateLimiterIndexMax + 1].
                Expected: NOT GT_OK.
            */
            /* call with rateLimiterIndex[0] */
            rateLimiterIndex = 0;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with rateLimiterIndex[rateLimiterIndexMax + 1] */
            rateLimiterIndex = rateLimiterIndexMax + 1;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            rateLimiterIndex = 1;

            /*
                1.1.4. Call api with wrong windowSize [0xFFF + 1].
                Expected: NOT GT_OK.
            */
            windowSize = 0xFFF + 1;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            windowSize = 0;

            /*
                1.1.5. Call api with wrong pktLimit [0xFFFF + 1].
                Expected: NOT GT_OK.
            */
            pktLimit = 0xFFFF + 1;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            pktLimit = 0;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    rateLimiterIndex = 1;
    windowSize = 0;
    pktLimit = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(dev,
                   portGroupsBmp, rateLimiterIndex, windowSize, pktLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet
(
    IN  GT_U8               dev,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_U32              *windowResolutionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with not nullportGroupsBmp.
    Expected: GT_OK.
    1.1.2. Call api with wrong windowResolutionPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8               dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;
    GT_U32              windowResolution;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with not nullportGroupsBmp.
                Expected: GT_OK.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, &windowResolution);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong windowResolutionPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                    "%d, windowResolutionPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, &windowResolution);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, &windowResolution);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, &windowResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, &windowResolution);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet
(
    IN  GT_U8              dev,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    IN  GT_U32             windowResolution
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with windowResolution[step1 * granularity / step2 * granularity]
    Expected: GT_OK.
    1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call with wrong windowResolution[0].
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8              dev;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;
    GT_U32             windowResolution = 0;
    GT_U32             windowResolutionGet = 0;

    GT_U32 coreClock   = 0;
    GT_U32 granularity = 0;
    GT_U32 systemClockCycles   = 0;
    GT_U32 step1       = 0;
    GT_U32 step2       = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            systemClockCycles = 256;
            step1 = 10;
            step2 = 20;
        }
        else
        {
            systemClockCycles = 32;
            step1 = 1;
            step2 = 2;
        }

        /* Getting device coreClock */
        st = prvUtfCoreClockGet(dev, &coreClock);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCoreClockGet: %d", dev);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, coreClock,
                            "prvUtfCoreClockGet: coreClock = 0, dev = %d", dev);
        if(0 == coreClock)
        {
            continue;
        }

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with windowResolution[step1 * granularity / step2 * granularity]
                Expected: GT_OK.
            */
            /* iterate with windowResolution = step1 * granularity */
            granularity      = (1000 * systemClockCycles) / coreClock;
            windowResolution = step1 * granularity;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(dev,
                                     portGroupsBmp, windowResolution);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, &windowResolutionGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(windowResolution, windowResolutionGet,
                           "got another windowResolution then was set: %d", dev);

            /* iterate with windowResolution = step2 * granularity */
            granularity      = (1000 * systemClockCycles) / coreClock;
            windowResolution = step2 * granularity;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(dev,
                                     portGroupsBmp, windowResolution);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(dev,
                                     portGroupsBmp, &windowResolutionGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(windowResolution, windowResolutionGet,
                           "got another windowResolution then was set: %d", dev);

            /*
                1.1.3. Call with wrong windowResolution[0].
                Expected: NOT GT_OK.
            */
            windowResolution = 0;

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(dev,
                                     portGroupsBmp, windowResolution);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            windowResolution = step1 * granularity;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(dev,
                                     portGroupsBmp, windowResolution);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(dev,
                                     portGroupsBmp, windowResolution);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    windowResolution = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(dev,
                                     portGroupsBmp, windowResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(dev,
                                     portGroupsBmp, windowResolution);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorBurstStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT *burstStatusPtr
)
*/

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorBurstStatusGet)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with txQueue [3 , 6],
                     burstStatusPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and other
        values same as 1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with out of range burstStatusPtr[NULL] and other
        values same as 1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                                   st              = GT_OK;
    GT_U8                                       dev             = 0;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_U32                                      portGroupId     = 0;
    GT_PORT_GROUPS_BMP                          portGroupsBmp   = 1;
    GT_U8                                       txQueue         = 0;
    CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT burstStatus;
    GT_U32                                      boardIdx;
    GT_U32                                      boardRevId;
    GT_U32                                      reloadEeprom;

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with txQueue [3].
                Expected: GT_OK.
            */
            txQueue = 3;

            st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                            txQueue, &burstStatus);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with txQueue [6].
                Expected: GT_OK.
            */
            txQueue = 6;

            st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                            txQueue, &burstStatus);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and other
                    values same as 1.1.
                Expected: GT_BAD_PARAM.
            */
            txQueue = NUM_OF_TX_QUEUES;
            st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                            txQueue, &burstStatus);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            txQueue = 3;

            /*
                1.1.3. Call with out of range burstStatusPtr[NULL] and other
                    values same as 1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                            txQueue, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            /* set valid value*/
            txQueue = 3;

            st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                            txQueue, &burstStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* set valid value*/
        txQueue = 3;

        st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                        txQueue, &burstStatus);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    txQueue = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                        txQueue, &burstStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(dev, portGroupsBmp,
                                                    txQueue, &burstStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorDisable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorDisable)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with txQueue [3, 6].
    Expected: GT_OK.
    1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES].
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS           st              = GT_OK;
    GT_U8               dev             = 0;
    GT_U32              notAppFamilyBmp = 0;
    GT_U32              portGroupId     = 0;
    GT_PORT_GROUPS_BMP  portGroupsBmp   = 1;
    GT_U8               txQueue         = 0;
    GT_U32              boardIdx;
    GT_U32              boardRevId;
    GT_U32              reloadEeprom;

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with txQueue [3].
                Expected: GT_OK.
            */
            txQueue = 3;

            /* enable appropriate Tx SDMA generator */
/*            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, GT_FALSE, 0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
*/
            /* disable Tx SDMA generator */
            st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp,
                                                     txQueue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with txQueue [6].
                Expected: GT_OK.
            */
            txQueue = 6;

            /* enable appropriate Tx SDMA generator */
/*            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, GT_FALSE, 0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
*/
            /* disable Tx SDMA generator */
            st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp,
                                                     txQueue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES].
                Expected: GT_BAD_PARAM.
            */
            txQueue = NUM_OF_TX_QUEUES;
            st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp, txQueue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore valid value */
            txQueue = 3;

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            /* set valid value*/
            txQueue = 3;

            st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp, txQueue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* set valid value*/
        txQueue = 3;

        st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp, txQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    txQueue = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp, txQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp, txQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorEnable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue,
    IN  GT_BOOL                         burstEnable,
    IN  GT_U32                          burstPacketsNumber
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorEnable)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with txQueue [3, 6, 6],
                     burstEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                     burstPacketsNumber[0, 256M, 512M-1].
    Expected: GT_OK.
    1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and
        other parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with burstEnable[GT_FALSE],
        out of range burstPacketsNumber[512M](not relevant) and
        other parameters same as 1.1.1.
    Expected: GT_OK.
    1.1.4. Call with burstEnable[GT_TRUE],
        out of range burstPacketsNumber[512M](relevant value) and
        other parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS           st                  = GT_OK;
    GT_U8               dev                 = 0;
    GT_U32              notAppFamilyBmp     = 0;
    GT_U32              portGroupId         = 0;
    GT_PORT_GROUPS_BMP  portGroupsBmp       = 1;
    GT_U8               txQueue             = 0;
    GT_BOOL             burstEnable         = GT_FALSE;
    GT_U32              burstPacketsNumber  = 0;
    GT_U32              boardIdx;
    GT_U32              boardRevId;
    GT_U32              reloadEeprom;

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with txQueue [3],
                                 burstEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                 burstPacketsNumber[0, 256M, 512M-1].
                Expected: GT_OK.
            */
            txQueue = 3;
            burstEnable = GT_FALSE;
            burstPacketsNumber = 0;

            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, burstEnable,
                                                    burstPacketsNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with txQueue [6],
                                 burstEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                 burstPacketsNumber[0, 256M, 512M-1].
                Expected: GT_OK.
            */
            txQueue = 6;
            burstEnable = GT_TRUE;
            burstPacketsNumber = BIT_28;
            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, burstEnable,
                                                    burstPacketsNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with txQueue [6],
                                 burstEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                 burstPacketsNumber[0, 256M, 512M-1].
                Expected: GT_OK.
            */
            txQueue = 6;
            burstEnable = GT_TRUE;
            burstPacketsNumber = BIT_29-1;
            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, burstEnable,
                                                    burstPacketsNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /*
                1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and
                    other parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            txQueue = NUM_OF_TX_QUEUES;

            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, burstEnable,
                                                    burstPacketsNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore values */
            txQueue = 3;

            /*
                1.1.3. Call with burstEnable[GT_FALSE],
                    out of range burstPacketsNumber[512M](not relevant) and
                    other parameters same as 1.1.1.
                Expected: GT_OK.
            */
            burstEnable = GT_FALSE;
            burstPacketsNumber = BIT_29;
            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, burstEnable,
                                                    burstPacketsNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            burstEnable = GT_TRUE;
            burstPacketsNumber = BIT_29-1;

            /*
                1.1.4. Call with burstEnable[GT_TRUE],
                    out of range burstPacketsNumber[512M](relevant value) and
                    other parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            burstEnable = GT_TRUE;
            burstPacketsNumber = BIT_29;

            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, burstEnable,
                                                    burstPacketsNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore values */
            burstEnable = GT_TRUE;
            burstPacketsNumber = BIT_29-1;

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            /* set valid value*/
            txQueue = 3;
            burstEnable = GT_TRUE;
            burstPacketsNumber = BIT_29-1;

            st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                    txQueue, burstEnable,
                                                    burstPacketsNumber);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* set valid value*/
        txQueue = 3;
        burstEnable = GT_TRUE;
        burstPacketsNumber = BIT_29-1;

        st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                txQueue, burstEnable,
                                                burstPacketsNumber);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);

    }

    /* restore valid values */
    txQueue = 3;
    burstEnable = GT_TRUE;
    burstPacketsNumber = BIT_29-1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                                txQueue, burstEnable,
                                                burstPacketsNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChNetIfSdmaTxGeneratorEnable(dev, portGroupsBmp,
                                            txQueue, burstEnable,
                                            burstPacketsNumber);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketAdd
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength,
    OUT GT_U32                      *packetIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorPacketAdd)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with packetParamsPtr{
                             packetIsTagged[GT_FALSE],
                             cookie[0],
                             sdmaInfo{
                                 recalcCrc[GT_TRUE],
                                 txQueue[3],
                                 evReqHndl[0],
                                 invokeTxBufferQueueEvent[GT_FALSE]
                             },
                             dsaParam{
                                 commonParams{
                                     dsaTagType[CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT],
                                     vpt[0],
                                     cfiBit[0],
                                     vid[0],
                                     dropOnSource[GT_FALSE],
                                     packetIsLooped[GT_FALSE]
                                 },
                                 dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                                 dsaInfo.fromCpu{
                                     dstInterface {
                                           type[CPSS_INTERFACE_PORT_E],
                                           devPort.hwDevNum[hwDevNum],
                                           devPort.portNum[8],
                                           trunkId[0],
                                           vidx[0],
                                           vlanId[0],
                                           devNum[0],
                                           fabricVidx[0],
                                           index[0]
                                     }
                                     tc[5],
                                     dp[CPSS_DP_LEVEL_ENT],
                                     egrFilterEn[GT_FALSE],
                                     cascadeControl[GT_FALSE],
                                     egrFilterRegistered[GT_FALSE],

                                     srcId[0],
                                     srcHwDev[0],
                                     extDestInfo.devPort{
                                         dstIsTagged[GT_FALSE],
                                         ailBoxToNeighborCPU[GT_FALSE]
                                     },
                                     isTrgPhyPortValid[GT_TRUE],
                                     dstEport[0],
                                     tag0TpidIndex[0]
                             },
                        },
                     valid packetDataPtr[NON-NULL],
                     packetDataLength[1024],
                     valid packetIdPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range packetParamsPtr->sdmaInfo.txQueue[8] and other
        values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.3. Call with out of range enum value
        packetParamsPtr->dsaParam.commonParams.dsaTagType and other
        values same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range packetParamsPtr->dsaParam.commonParams.vpt[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.5. Call with out of range packetParamsPtr->dsaParam.commonParams.cfiBit[2]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.6. Call with out of range packetParamsPtr->dsaParam.commonParams.vid[BIT_16]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.7. Call with out of range enum value
        packetParamsPtr->dsaParam.dsaType and other
        values same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.8. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[BIT_20]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.9. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.10. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.dstEport[BIT_17]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.11. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.12. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.13. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
        packetParamsPtr->dsaParam.dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.forward.dstEport[BIT_17]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.14. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.forward.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.15. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
        packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_FALSE],
        out of range packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.16. Call with out of range packetParamsPtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.17. Call with out of range packetDataPtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.18. Call with out of range packetDataLength[0] and other
        values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.19. Call with out of range packetIdPtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st                  = GT_OK;
    GT_U8                       dev                 = 0;
    GT_U32                      notAppFamilyBmp     = 0;
    GT_U32                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp       = 1;
    CPSS_DXCH_NET_TX_PARAMS_STC packetParams;
    GT_HW_DEV_NUM               hwDevNum;
    GT_U8                       packetData[1024];
    GT_U32                      packetDataLength;
    GT_U32                      packetId            = 0;
    GT_U8                       txQueue;
    GT_U32                      boardIdx;
    GT_U32                      boardRevId;
    GT_U32                      reloadEeprom;
    GT_U32                      numOfNetIfs = 1;

    cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    cpssOsMemSet(&packetData, 0, sizeof(packetData));
    packetDataLength    = 64;

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /* disable Traffic Generator to avoid traffic injection.
               mainUt kills configuration of device and wrong to inject traffic.*/
            for (txQueue = 0; txQueue < 8; txQueue++)
            {
                st = cpssDxChNetIfSdmaTxGeneratorDisable(dev, portGroupsBmp,
                                                     txQueue);
                if (st == GT_BAD_PARAM)
                {
                    /* only several TX queues works in Traffic Generator mode */
                    st = GT_OK;
                }
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, txQueue);
            }

            /*
                1.1.1. Call with packetParamsPtr{
                                         packetIsTagged[GT_FALSE],
                                         cookie[0],
                                         sdmaInfo{
                                             recalcCrc[GT_TRUE],
                                             txQueue[3],
                                             evReqHndl[0],
                                             invokeTxBufferQueueEvent[GT_FALSE]
                                         },
                                         dsaParam{
                                             commonParams{
                                                 dsaTagType[CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT],
                                                 vpt[0],
                                                 cfiBit[0],
                                                 vid[0],
                                                 dropOnSource[GT_FALSE],
                                                 packetIsLooped[GT_FALSE]
                                             },
                                             dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                                             dsaInfo.fromCpu{
                                                 dstInterface {
                                                       type[CPSS_INTERFACE_PORT_E],
                                                       devPort.hwDevNum[hwDevNum],
                                                       devPort.portNum[8],
                                                       trunkId[0],
                                                       vidx[0],
                                                       vlanId[0],
                                                       devNum[0],
                                                       fabricVidx[0],
                                                       index[0]
                                                 }
                                                 tc[5],
                                                 dp[CPSS_DP_LEVEL_ENT],
                                                 egrFilterEn[GT_FALSE],
                                                 cascadeControl[GT_FALSE],
                                                 egrFilterRegistered[GT_FALSE],

                                                 srcId[0],
                                                 srcHwDev[0],
                                                 extDestInfo.devPort{
                                                     dstIsTagged[GT_FALSE],
                                                     ailBoxToNeighborCPU[GT_FALSE]
                                                 },
                                                 isTrgPhyPortValid[GT_TRUE],
                                                 dstEport[0],
                                                 tag0TpidIndex[0]
                                         },
                                    },
                                 valid packetDataPtr[NON-NULL],
                                 packetDataLength[1024],
                                 valid packetIdPtr[NON-NULL].
                Expected: GT_OK.
            */

            /* Get HW device number */
            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with out of range packetParamsPtr->sdmaInfo.txQueue[8] and other
                    values same as 1.1.1.
                Expected: NON GT_OK.
            */
            st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            packetParams.sdmaInfo.txQueue = (GT_U8)(8 * numOfNetIfs);

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            packetParams.sdmaInfo.txQueue = 3;

            /*
                1.1.3. Call with out of range enum value
                    packetParamsPtr->dsaParam.commonParams.dsaTagType and other
                    values same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaTxGeneratorPacketAdd
                                (dev, portGroupsBmp, &packetParams, packetData,
                                                   packetDataLength,&packetId),
                                 packetParams.dsaParam.commonParams.dsaTagType);

            /*
                1.1.4. Call with out of range packetParamsPtr->dsaParam.commonParams.vpt[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.commonParams.vpt = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            packetParams.dsaParam.commonParams.vpt = 0;

            /*
                1.1.5. Call with out of range packetParamsPtr->dsaParam.commonParams.cfiBit[2]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.commonParams.cfiBit = 2;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            packetParams.dsaParam.commonParams.cfiBit = 0;


            /*
                1.1.7. Call with out of range enum value
                    packetParamsPtr->dsaParam.dsaType and other
                    values same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaTxGeneratorPacketAdd
                                (dev, portGroupsBmp, &packetParams, packetData,
                                                   packetDataLength,&packetId),
                                 packetParams.dsaParam.dsaType);

            /*
                1.1.8. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[BIT_20]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = BIT_20;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.9. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.10. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.dstEport[BIT_17]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstEport = BIT_17;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.11. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.12. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E;
            packetParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.13. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                    packetParamsPtr->dsaParam.dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.forward.dstEport[BIT_17]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
            packetParams.dsaParam.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.forward.dstEport = BIT_17;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.14. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.forward.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
            packetParams.dsaParam.dsaInfo.forward.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.15. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                    packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_FALSE],
                    out of range packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
            packetParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
            packetParams.dsaParam.dsaInfo.forward.origSrcPhy.portNum = 4096;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.16. Call with out of range packetParamsPtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    NULL, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.17. Call with out of range packetDataPtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, NULL,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.18. Call with out of range packetDataLength[0] and other
                    values same as 1.1.1.
                Expected: NON GT_OK.
            */
            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    0, &packetId);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.19. Call with out of range packetIdPtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            /* set valid value*/
            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = 3;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;


            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* set valid value*/
        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

        cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
        packetParams.packetIsTagged = GT_FALSE;
        packetParams.sdmaInfo.recalcCrc = GT_TRUE;
        packetParams.sdmaInfo.txQueue = 3;
        packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
        packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
        packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

        st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                &packetParams, packetData,
                                                packetDataLength,&packetId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    packetParams.packetIsTagged = GT_FALSE;
    packetParams.sdmaInfo.recalcCrc = GT_TRUE;
    packetParams.sdmaInfo.txQueue = 3;
    packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
    packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
    packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;

        st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                &packetParams, packetData,
                                                packetDataLength,&packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                            &packetParams, packetData,
                                            packetDataLength,&packetId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketRemove
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U8               txQueue,
    IN  GT_U32              packetId
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorPacketRemove)*/
GT_VOID cpssDxChNetIfSdmaTxGeneratorPacketRemoveUT(GT_VOID)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with txQueue [3, 6].
    Expected: GT_OK.
    1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and
        other parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with out of range packetId[0xFFFF] and
        other parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                   st                  = GT_OK;
    GT_U8                       dev                 = 0;
    GT_U32                      notAppFamilyBmp     = 0;
    GT_U32                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp       = 1;
    GT_U8                       txQueue             = 0;
    GT_U32                      packetId            = 0;
    CPSS_DXCH_NET_TX_PARAMS_STC packetParams;
    GT_HW_DEV_NUM               hwDevNum;
    static GT_U8                packetData[1024];   /* impossible to define big arrays on stack */
    GT_U32                      packetDataLength;
    GT_U32                      boardIdx;
    GT_U32                      boardRevId;
    GT_U32                      reloadEeprom;


    cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    cpssOsMemSet(&packetData, 0, sizeof(packetData));
    packetDataLength    = 64;

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with txQueue [3].
                Expected: GT_OK.
            */

            txQueue = 3;

            /* get HW device number */
            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /* add packet */
            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* remove exist packet */
            st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                          txQueue, packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with txQueue [6].
                Expected: GT_OK.
            */
            txQueue = 6;

            /* get HW device number */
            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /* add packet */
            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* remove exist packet */
            st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                          txQueue, packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and
                    other parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            txQueue = NUM_OF_TX_QUEUES;

            st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                          txQueue, packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore values */
            txQueue = 3;

            /*
                1.1.3. Call with out of range packetId[0xFFFF] and
                    other parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            packetId = 0xFFFF;
            st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                          txQueue, packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            /* set valid value*/
            txQueue = 3;
            packetId= 0;

            st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                          txQueue, packetId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* set valid value*/
        txQueue = 3;

        /* get HW device number */
        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

        packetParams.packetIsTagged = GT_FALSE;
        packetParams.sdmaInfo.recalcCrc = GT_TRUE;
        packetParams.sdmaInfo.txQueue = txQueue;
        packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
        packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
        packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

        /* add packet */
        st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                &packetParams, packetData,
                                                packetDataLength,&packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                      txQueue, packetId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    txQueue = 3;
    packetId= 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                      txQueue, packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(dev, portGroupsBmp,
                                                  txQueue, packetId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketUpdate
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      packetId,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorPacketUpdate)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1  Call cpssDxChNetIfSdmaTxGeneratorPacketAdd with
                    packetParamsPtr{
                             packetIsTagged[GT_FALSE],
                             cookie[0],
                             sdmaInfo{
                                 recalcCrc[GT_TRUE],
                                 txQueue[3],
                                 evReqHndl[0],
                                 invokeTxBufferQueueEvent[GT_FALSE]
                             },
                             dsaParam{
                                 commonParams{
                                     dsaTagType[CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT],
                                     vpt[0],
                                     cfiBit[0],
                                     vid[0],
                                     dropOnSource[GT_FALSE],
                                     packetIsLooped[GT_FALSE]
                                 },
                                 dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                                 dsaInfo.fromCpu{
                                     dstInterface {
                                           type[CPSS_INTERFACE_PORT_E],
                                           devPort.hwDevNum[hwDevNum],
                                           devPort.portNum[8],
                                           trunkId[0],
                                           vidx[0],
                                           vlanId[0],
                                           devNum[0],
                                           fabricVidx[0],
                                           index[0]
                                     }
                                     tc[5],
                                     dp[CPSS_DP_LEVEL_ENT],
                                     egrFilterEn[GT_FALSE],
                                     cascadeControl[GT_FALSE],
                                     egrFilterRegistered[GT_FALSE],

                                     srcId[0],
                                     srcHwDev[0],
                                     extDestInfo.devPort{
                                         dstIsTagged[GT_FALSE],
                                         ailBoxToNeighborCPU[GT_FALSE]
                                     },
                                     isTrgPhyPortValid[GT_TRUE],
                                     dstEport[0],
                                     tag0TpidIndex[0]
                             },
                        },
                     valid packetDataPtr[NON-NULL],
                     packetDataLength[64],
                     valid packetIdPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Update with packetParamsPtr{
                             packetIsTagged[GT_FALSE],
                             cookie[0],
                             sdmaInfo{
                                 recalcCrc[GT_TRUE],
                                 txQueue[6],
                                 evReqHndl[0],
                                 invokeTxBufferQueueEvent[GT_FALSE]
                             },
                             dsaParam{
                                 commonParams{
                                     dsaTagType[CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT],
                                     vpt[0],
                                     cfiBit[0],
                                     vid[0],
                                     dropOnSource[GT_FALSE],
                                     packetIsLooped[GT_FALSE]
                                 },
                                 dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                                 dsaInfo.fromCpu{
                                     dstInterface {
                                           type[CPSS_INTERFACE_PORT_E],
                                           devPort.hwDevNum[hwDevNum],
                                           devPort.portNum[8],
                                           trunkId[0],
                                           vidx[0],
                                           vlanId[0],
                                           devNum[0],
                                           fabricVidx[0],
                                           index[0]
                                     }
                                     tc[5],
                                     dp[CPSS_DP_LEVEL_ENT],
                                     egrFilterEn[GT_FALSE],
                                     cascadeControl[GT_FALSE],
                                     egrFilterRegistered[GT_FALSE],

                                     srcId[0],
                                     srcHwDev[0],
                                     extDestInfo.devPort{
                                         dstIsTagged[GT_FALSE],
                                         ailBoxToNeighborCPU[GT_FALSE]
                                     },
                                     isTrgPhyPortValid[GT_TRUE],
                                     dstEport[0],
                                     tag0TpidIndex[0]
                             },
                        },
                     valid packetDataPtr,
                     packetDataLength[64],
                     valid packetId.
    Expected: GT_OK.
    1.1.3. Call with out of range packetParamsPtr->sdmaInfo.txQueue[8] and other
        values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.4. Call with out of range enum value
        packetParamsPtr->dsaParam.commonParams.dsaTagType and other
        values same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with out of range packetParamsPtr->dsaParam.commonParams.vpt[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.6. Call with out of range packetParamsPtr->dsaParam.commonParams.cfiBit[2]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.7. Call with out of range packetParamsPtr->dsaParam.commonParams.vid[BIT_12]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.8. Call with out of range enum value
        packetParamsPtr->dsaParam.dsaType and other
        values same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.9. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[BIT_20]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.10. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.11. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.dstEport[BIT_17]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.12. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.13. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.14. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
        packetParamsPtr->dsaParam.dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.forward.dstEport[BIT_17]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.15. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
        out of range packetParamsPtr->dsaParam.dsaInfo.forward.tag0TpidIndex[8]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.16. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
        packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_FALSE],
        out of range packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096]
        and other values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.17. Call with out of range packetParamsPtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.18. Call with out of range packetDataPtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.19. Call with out of range packetDataLength[0] and other
        values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.20. Call with out of range packetIdPtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st                  = GT_OK;
    GT_U8                       dev                 = 0;
    GT_U32                      notAppFamilyBmp     = 0;
    GT_U32                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp       = 1;
    CPSS_DXCH_NET_TX_PARAMS_STC packetParams;
    GT_HW_DEV_NUM               hwDevNum;
    GT_U8                       packetData[1024];
    GT_U32                      packetDataLength;
    GT_U32                      packetId            = 0;
    GT_U32                      wrongPacketId       = 0xFFFF;
    GT_U8                       txQueue             = 0;
    GT_U32                      boardIdx;
    GT_U32                      boardRevId;
    GT_U32                      reloadEeprom;
    GT_U32                      numOfNetIfs = 1;

    cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    cpssOsMemSet(&packetData, 0, sizeof(packetData));
    packetDataLength    = 64;

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with packetParamsPtr{
                                         packetIsTagged[GT_FALSE],
                                         cookie[0],
                                         sdmaInfo{
                                             recalcCrc[GT_TRUE],
                                             txQueue[3],
                                             evReqHndl[0],
                                             invokeTxBufferQueueEvent[GT_FALSE]
                                         },
                                         dsaParam{
                                             commonParams{
                                                 dsaTagType[CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT],
                                                 vpt[0],
                                                 cfiBit[0],
                                                 vid[0],
                                                 dropOnSource[GT_FALSE],
                                                 packetIsLooped[GT_FALSE]
                                             },
                                             dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                                             dsaInfo.fromCpu{
                                                 dstInterface {
                                                       type[CPSS_INTERFACE_PORT_E],
                                                       devPort.hwDevNum[hwDevNum],
                                                       devPort.portNum[8],
                                                       trunkId[0],
                                                       vidx[0],
                                                       vlanId[0],
                                                       devNum[0],
                                                       fabricVidx[0],
                                                       index[0]
                                                 }
                                                 tc[5],
                                                 dp[CPSS_DP_LEVEL_ENT],
                                                 egrFilterEn[GT_FALSE],
                                                 cascadeControl[GT_FALSE],
                                                 egrFilterRegistered[GT_FALSE],

                                                 srcId[0],
                                                 srcHwDev[0],
                                                 extDestInfo.devPort{
                                                     dstIsTagged[GT_FALSE],
                                                     ailBoxToNeighborCPU[GT_FALSE]
                                                 },
                                                 isTrgPhyPortValid[GT_TRUE],
                                                 dstEport[0],
                                                 tag0TpidIndex[0]
                                         },
                                    },
                                 valid packetDataPtr[NON-NULL],
                                 packetDataLength[1024],
                                 valid packetIdPtr[NON-NULL].
                Expected: GT_OK.
            */
            txQueue             = 3;

            /* Get HW device number */
            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(dev, portGroupsBmp,
                                                    &packetParams, packetData,
                                                    packetDataLength,&packetId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Update with packetParamsPtr{
                                         packetIsTagged[GT_FALSE],
                                         cookie[0],
                                         sdmaInfo{
                                             recalcCrc[GT_TRUE],
                                             txQueue[3],
                                             evReqHndl[0],
                                             invokeTxBufferQueueEvent[GT_FALSE]
                                         },
                                         dsaParam{
                                             commonParams{
                                                 dsaTagType[CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT],
                                                 vpt[0],
                                                 cfiBit[0],
                                                 vid[0],
                                                 dropOnSource[GT_FALSE],
                                                 packetIsLooped[GT_FALSE]
                                             },
                                             dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                                             dsaInfo.fromCpu{
                                                 dstInterface {
                                                       type[CPSS_INTERFACE_PORT_E],
                                                       devPort.hwDevNum[hwDevNum],
                                                       devPort.portNum[8],
                                                       trunkId[0],
                                                       vidx[0],
                                                       vlanId[0],
                                                       devNum[0],
                                                       fabricVidx[0],
                                                       index[0]
                                                 }
                                                 tc[5],
                                                 dp[CPSS_DP_LEVEL_ENT],
                                                 egrFilterEn[GT_FALSE],
                                                 cascadeControl[GT_FALSE],
                                                 egrFilterRegistered[GT_FALSE],

                                                 srcId[0],
                                                 srcHwDev[0],
                                                 extDestInfo.devPort{
                                                     dstIsTagged[GT_FALSE],
                                                     ailBoxToNeighborCPU[GT_FALSE]
                                                 },
                                                 isTrgPhyPortValid[GT_TRUE],
                                                 dstEport[0],
                                                 tag0TpidIndex[0]
                                         },
                                    },
                                 valid packetDataPtr,
                                 packetDataLength[64],
                                 valid packetId.
                Expected: GT_OK.
            */

            /* Get HW device number */
            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.3. Call with out of range packetParamsPtr->sdmaInfo.txQueue[8] and other
                    values same as 1.1.1.
                Expected: NON GT_OK.
            */
            st = prvCpssDxChNetIfMultiNetIfNumberGet(dev, &numOfNetIfs);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            packetParams.sdmaInfo.txQueue = (GT_U8)(8 * numOfNetIfs);

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            packetParams.sdmaInfo.txQueue = txQueue;

            /*
                1.1.4. Call with out of range enum value
                    packetParamsPtr->dsaParam.commonParams.dsaTagType and other
                    values same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaTxGeneratorPacketUpdate
                                (dev, portGroupsBmp, packetId,
                                 &packetParams, packetData, packetDataLength),
                                packetParams.dsaParam.commonParams.dsaTagType);

            /*
                1.1.5. Call with out of range packetParamsPtr->dsaParam.commonParams.vpt[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.commonParams.vpt = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            packetParams.dsaParam.commonParams.vpt = 0;

            /*
                1.1.6. Call with out of range packetParamsPtr->dsaParam.commonParams.cfiBit[2]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.commonParams.cfiBit = 2;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            packetParams.dsaParam.commonParams.cfiBit = 0;


            /*
                1.1.8. Call with out of range enum value
                    packetParamsPtr->dsaParam.dsaType and other
                    values same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaTxGeneratorPacketUpdate
                                (dev, portGroupsBmp, packetId,
                                 &packetParams, packetData, packetDataLength),
                                 packetParams.dsaParam.dsaType);

            /*
                1.1.9. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId[BIT_20]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId = BIT_20;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.10. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.toCpu.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.toCpu.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.11. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.dstEport[BIT_17]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstEport = BIT_17;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.12. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.fromCpu.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.13. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E;
            packetParams.dsaParam.dsaInfo.toAnalyzer.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.14. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                    packetParamsPtr->dsaParam.dsaInfo.forward.dstInterface.type[CPSS_INTERFACE_PORT_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.forward.dstEport[BIT_17]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
            packetParams.dsaParam.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.forward.dstEport = BIT_17;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.15. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                    out of range packetParamsPtr->dsaParam.dsaInfo.forward.tag0TpidIndex[8]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
            packetParams.dsaParam.dsaInfo.forward.tag0TpidIndex = 8;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.16. Call with packetParamsPtr->dsaParam.dsaType[CPSS_DXCH_NET_DSA_CMD_FORWARD_E],
                    packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk[GT_FALSE],
                    out of range packetParamsPtr->dsaParam.dsaInfo.forward.origSrcPhy.portNum[4096]
                    and other values same as 1.1.1.
                Expected: NON GT_OK.
            */
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
            packetParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
            packetParams.dsaParam.dsaInfo.forward.origSrcPhy.portNum = 4096;

            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore values */
            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

            /*
                1.1.17. Call with out of range packetParamsPtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, NULL,
                                                          packetData, packetDataLength);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.18. Call with out of range packetDataPtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          NULL, packetDataLength);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.19. Call with out of range packetDataLength[0] and other
                    values same as 1.1.1.
                Expected: NON GT_OK.
            */
            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, 0);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.20. Call with out of range packetId[0xFFFF] and other
                    values same as 1.1.1.
                Expected: NON GT_OK.
            */
            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          wrongPacketId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            /* set valid value*/
            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

            cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            packetParams.packetIsTagged = GT_FALSE;
            packetParams.sdmaInfo.recalcCrc = GT_TRUE;
            packetParams.sdmaInfo.txQueue = txQueue;
            packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
            packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
            packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
            packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;


            st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                          packetId, &packetParams,
                                                          packetData, packetDataLength);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* set valid value*/
        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);

        cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
        packetParams.packetIsTagged = GT_FALSE;
        packetParams.sdmaInfo.recalcCrc = GT_TRUE;
        packetParams.sdmaInfo.txQueue = txQueue;
        packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
        packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
        packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

        st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                      packetId, &packetParams,
                                                      packetData, packetDataLength);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    packetParams.packetIsTagged = GT_FALSE;
    packetParams.sdmaInfo.recalcCrc = GT_TRUE;
    packetParams.sdmaInfo.txQueue = txQueue;
    packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
    packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
    packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hwDevNum);
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;

        st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                      packetId, &packetParams,
                                                      packetData, packetDataLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(dev, portGroupsBmp,
                                                  packetId, &packetParams,
                                                  packetData, packetDataLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorRateGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    *rateModePtr,
    OUT GT_U64                                      *rateValuePtr,
    OUT GT_U64                                      *actualRateValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorRateGet)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with txQueue [3 , 6],
                     rateModePtr[NON-NULL],
                     rateValuePtr[NON-NULL],
                     actualRateValuePtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and other
        values same as 1.1.
    Expected: NON GT_OK.
    1.1.3. Call with out of range rateModePtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.4. Call with out of range rateValuePtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.5. Call with out of range actualRateValuePtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                                   st                  = GT_OK;
    GT_U8                                       dev                 = 0;
    GT_U32                                      notAppFamilyBmp     = 0;
    GT_U32                                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP                          portGroupsBmp       = 1;
    GT_U8                                       txQueue             = 0;
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode            = 0;
    GT_U64                                      rateValue;
    GT_U64                                      actualRateValue;
    GT_U32                                      boardIdx;
    GT_U32                                      boardRevId;
    GT_U32                                      reloadEeprom;

    cpssOsBzero((GT_VOID*)&actualRateValue, sizeof(actualRateValue));
    cpssOsBzero((GT_VOID*)&rateValue, sizeof(rateValue));

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with txQueue [3].
                Expected: GT_OK.
            */
            txQueue = 3;

            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateMode,
                                                     &rateValue, &actualRateValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with txQueue [6].
                Expected: GT_OK.
            */
            txQueue = 6;

            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateMode,
                                                     &rateValue, &actualRateValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with out of range txQueue[NUM_OF_TX_QUEUES] and other
                    values same as 1.1.
                Expected: NON GT_OK.
            */
            txQueue = NUM_OF_TX_QUEUES;
            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateMode,
                                                     &rateValue, &actualRateValue);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore valid value */
            txQueue = 3;

            /*
                1.1.3. Call with out of range rateModePtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */


            /* set relevant rateMode and rateValue[1M]. */
            txQueue = 3;
            rateMode = CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E;
            rateValue.l[0] = 1000000;
            rateValue.l[1] = 0;

            st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                     txQueue, rateMode,
                                                     rateValue, &actualRateValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, NULL,
                                                     &rateValue, &actualRateValue);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.4. Call with out of range rateValuePtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateMode,
                                                     NULL, &actualRateValue);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.5. Call with out of range actualRateValuePtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateMode,
                                                     &rateValue, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            /* set valid value*/
            txQueue = 3;

            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateMode,
                                                     &rateValue, &actualRateValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* set valid value*/
        txQueue = 3;

        st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                 txQueue, &rateMode,
                                                 &rateValue, &actualRateValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    txQueue = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                 txQueue, &rateMode,
                                                 &rateValue, &actualRateValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                             txQueue, &rateMode,
                                             &rateValue, &actualRateValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNetIfSdmaTxGeneratorRateSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    IN  CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode,
    IN  GT_U64                                      rateValue,
    OUT GT_U64                                      *actualRateValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNetIfSdmaTxGeneratorRateSet)
{
/*
    ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with txQueue [3 , 6],
        rateMode[CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
                 CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E],
        rateValue[1M, 1000],
        actualRateValuePtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call cpssDxChNetIfSdmaTxGeneratorRateGet.
    Expected: GT_OK and the same value as was set.
    1.1.3. Call with out of range txQueue[NUM_OF_TX_QUEUES] and other
        values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.4. Call with out of range enum value rateMode and other
        values same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with out of range rateValue[25M+1] and other
        values same as 1.1.1.
    Expected: NON GT_OK.
    1.1.6. Call with out of range actualRateValuePtr[NULL] and other
        values same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                                   st                  = GT_OK;
    GT_U8                                       dev                 = 0;
    GT_U32                                      notAppFamilyBmp     = 0;
    GT_U32                                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP                          portGroupsBmp       = 1;
    GT_U8                                       txQueue             = 0;
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode            = 0;
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateModeGet         = 0;
    GT_U64                                      rateValue;
    GT_U64                                      rateValueGet;
    GT_U64                                      actualRateValue;
    GT_BOOL                                     isEqual             = GT_FALSE;
    GT_U32                                      boardIdx;
    GT_U32                                      boardRevId;
    GT_U32                                      reloadEeprom;

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* Bobcat2 GM not implements NetIfSdmaTxGenerator */
        SKIP_TEST_MAC;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    cpssOsBzero((GT_VOID*)&rateValue, sizeof(rateValue));
    cpssOsBzero((GT_VOID*)&rateValueGet, sizeof(rateValueGet));
    cpssOsBzero((GT_VOID*)&actualRateValue, sizeof(actualRateValue));

    /* ITERATE_DEVICES (xCat3, Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* clear xCat3 related bit */
    notAppFamilyBmp &= ~UTF_XCAT3_E;

    /* to avoid compilation warning */
    rateValue.l[0]=0;
    rateValue.l[1]=0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with txQueue [3 , 6],
                    rateMode[CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
                             CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E],
                    rateValue[1M, 1000],
                    actualRateValuePtr[NON-NULL].
                Expected: GT_OK.
            */
            txQueue = 3;
            rateMode = CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E;
            rateValue.l[0] = 1000000;
            rateValue.l[1] = 0;

            st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                     txQueue, rateMode,
                                                     rateValue, &actualRateValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfSdmaTxGeneratorRateGet.
                Expected: GT_OK and the same value as was set.
            */

            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateModeGet,
                                                     &rateValueGet,
                                                     &actualRateValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* verifying values */
            UTF_VERIFY_EQUAL0_STRING_MAC(rateMode, rateModeGet,
                                         "got another value than was set");

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&rateValue,
                                         (GT_VOID*)&rateValueGet,
                                         sizeof (rateValue))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                         "got another value than was set");

            /*
                1.1.1. Call with txQueue [3 , 6],
                    rateMode[CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
                             CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E],
                    rateValue[1M, 1000],
                    actualRateValuePtr[NON-NULL].
                Expected: GT_OK.
            */
            txQueue = 6;
            rateMode = CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E;
            rateValue.l[0] = 1000000;
            rateValue.l[1] = 0;

            st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                     txQueue, rateMode,
                                                     rateValue, &actualRateValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChNetIfSdmaTxGeneratorRateGet.
                Expected: GT_OK and the same value as was set.
            */

            st = cpssDxChNetIfSdmaTxGeneratorRateGet(dev, portGroupsBmp,
                                                     txQueue, &rateModeGet,
                                                     &rateValueGet,
                                                     &actualRateValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* verifying values */
            UTF_VERIFY_EQUAL0_STRING_MAC(rateMode, rateModeGet,
                                         "got another value than was set");

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&rateValue,
                                         (GT_VOID*)&rateValueGet,
                                         sizeof (rateValue))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                         "got another value than was set");


            /*
                1.1.3. Call with out of range txQueue[NUM_OF_TX_QUEUES] and other
                    values same as 1.1.1.
                Expected: NON GT_OK.
            */
            txQueue = NUM_OF_TX_QUEUES;

            st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                     txQueue, rateMode,
                                                     rateValue, &actualRateValue);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore valid value */
            txQueue = 3;

            /*
                1.1.4. Call with out of range enum value rateMode and other
                    values same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNetIfSdmaTxGeneratorRateSet
                                (dev, portGroupsBmp, txQueue, rateMode,
                                 rateValue, &actualRateValue),
                                 rateMode);

            /*
                1.1.5. Call with out of range rateValue[25M+1] and other
                    values same as 1.1.1.
                Expected: NON GT_OK.
            */
            txQueue = 3;
            rateMode = CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E;
            rateValue.l[0] = 25000000+1;
            rateValue.l[1] = 0;

            st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                     txQueue, rateMode,
                                                     rateValue, &actualRateValue);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore valid value */
            rateValue.l[0] = 1000000;
            rateValue.l[1] = 0;

            /*
                1.1.6. Call with out of range actualRateValuePtr[NULL] and other
                    values same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                     txQueue, rateMode,
                                                     rateValue, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);


            st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                     txQueue, rateMode,
                                                     rateValue, &actualRateValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;


        st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                 txQueue, rateMode,
                                                 rateValue, &actualRateValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    txQueue = 3;
    rateMode = CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E;
    rateValue.l[0] = 1000000;
    rateValue.l[1] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                                 txQueue, rateMode,
                                                 rateValue, &actualRateValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChNetIfSdmaTxGeneratorRateSet(dev, portGroupsBmp,
                                             txQueue, rateMode,
                                             rateValue, &actualRateValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChNetIf suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChNetIf)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPrePendTwoBytesHeaderSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPrePendTwoBytesHeaderGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaRxPacketGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaRxCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaRxErrorCountGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaRxQueueEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaRxQueueEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxQueueEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxQueueEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfDsaTagBuild)             /*  not Get function */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfDuplicateEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfDuplicateEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortDuplicateToCpuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortDuplicateToCpuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpSynSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpSynGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeIpLinkLocalProtSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeIpLinkLocalProtGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeDesignatedDeviceTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeDesignatedDeviceTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeRateLimiterTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeRateLimiterTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfFromCpuDpSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfFromCpuDpGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaRxResourceErrorModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaRxResourceErrorModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfDsaTagParse)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfRxBufFree)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfRxBufFreeWithSize)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfTxBufferQueueGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeRateLimiterDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeRateLimiterDropCntrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorBurstStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorDisable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorPacketAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorPacketRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorPacketUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorRateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxGeneratorRateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeToPhysicalPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfCpuCodeToPhysicalPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaPhysicalPortToQueueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaQueueToPhysicalPortGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChNetIf)

/* the tests under cpssDxChNetIf_packetSend are tests that may send packets , and
   need a 'correct' tables configurations.
   so those should run as 'first tests' after cpssInitSystem
     */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChNetIf_packetSend)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaTxPacketSend)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNetIfSdmaSyncTxPacketSend)
UTF_SUIT_END_TESTS_MAC(cpssDxChNetIf_packetSend)

/*----------------------------------------------------------------------------*/
/*
    Set dsaInfoPtr->dsaInfo.toCpu structure.
*/
static void prvSetDsaInfoToCpu
(
    OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr
)
{
    /*
    dsaInfoPtr{dsaInfo{toCpu{
                            isEgressPipe[GT_FALSE],
                            isTagged [GT_FALSE],
                            devNum [0],
                            srcIsTrunk[GT_FALSE],
                            interface{portNum[0]},
                            cpuCode [CPSS_NET_CONTROL_BPDU_E],
                            wasTruncated [GT_FALSE],
                            originByteCount[0],
                            }
    */
    dsaInfoPtr->dsaInfo.toCpu.isEgressPipe = GT_FALSE;

    dsaInfoPtr->dsaInfo.toCpu.isTagged   = GT_FALSE;
    dsaInfoPtr->dsaInfo.toCpu.hwDevNum     = 0;
    dsaInfoPtr->dsaInfo.toCpu.srcIsTrunk = GT_FALSE;

    dsaInfoPtr->dsaInfo.toCpu.interface.srcTrunkId = 0;
    dsaInfoPtr->dsaInfo.toCpu.interface.portNum    = NETIF_VALID_PHY_PORT_CNS;

    dsaInfoPtr->dsaInfo.toCpu.cpuCode         = CPSS_NET_CONTROL_BPDU_E;
    dsaInfoPtr->dsaInfo.toCpu.wasTruncated    = GT_FALSE;
    dsaInfoPtr->dsaInfo.toCpu.originByteCount = 0;
}

/*----------------------------------------------------------------------------*/
/*
    Set dsaInfoPtr->dsaInfo.fromCpu structure.
*/
static void prvSetDsaInfoFromCpu
(
    OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr
)
{
    /*
    dsaInfoPtr{dsaInfo{fromCpu{
                                dstInterface [CPSS_INTERFACE_VIDX_E],
                                tc [7],
                                dp [CPSS_DP_GREEN_E],
                                egrFilterEn [GT_FALSE],
                                cascadeControl [GT_FALSE],
                                egrFilterRegistered [GT_FALSE],
                                srcId [0],
                                srcHwDev [0],
                                extDestInfo{
                                            excludeInterface [GT_FALSE],
                                            excludedInterface{
                                                                type[CPSS_INTERFACE_VIDX_E],
                                                                vidx[100]}
                                            mirrorToAllCPUs[GT_FALSE]}
                            }
    */
    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_VIDX_E;
    dsaInfoPtr->dsaInfo.fromCpu.dstInterface.vidx = 100;

    dsaInfoPtr->dsaInfo.fromCpu.tc = 7;
    dsaInfoPtr->dsaInfo.fromCpu.dp = CPSS_DP_GREEN_E;

    dsaInfoPtr->dsaInfo.fromCpu.egrFilterEn         = GT_FALSE;
    dsaInfoPtr->dsaInfo.fromCpu.cascadeControl      = GT_FALSE;
    dsaInfoPtr->dsaInfo.fromCpu.egrFilterRegistered = GT_FALSE;

    dsaInfoPtr->dsaInfo.fromCpu.srcId  = 0;
    dsaInfoPtr->dsaInfo.fromCpu.srcHwDev = 0;

    dsaInfoPtr->dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface       = GT_FALSE;
    dsaInfoPtr->dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type = CPSS_INTERFACE_VIDX_E;
    dsaInfoPtr->dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.vidx = 100;
    dsaInfoPtr->dsaInfo.fromCpu.extDestInfo.multiDest.mirrorToAllCPUs        = GT_FALSE;
}

/*----------------------------------------------------------------------------*/
/*
    Set dsaInfoPtr->dsaInfo.toAnalizer structure.
*/
static void prvSetDsaInfoToAnalizer
(
    OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr
)
{
    /*
    dsaInfoPtr{dsaInfo{toAnalizer{
                                    rxSniffer [GT_FALSE],
                                    isTagged [GT_FALSE],
                                    devPort{
                                            devNum [0],
                                            portNum [0]
                                }
    */
    dsaInfoPtr->dsaInfo.toAnalyzer.rxSniffer = GT_FALSE;
    dsaInfoPtr->dsaInfo.toAnalyzer.isTagged  = GT_FALSE;

    dsaInfoPtr->dsaInfo.toAnalyzer.devPort.hwDevNum  = 0;
    dsaInfoPtr->dsaInfo.toAnalyzer.devPort.portNum = NETIF_VALID_PHY_PORT_CNS;
}

/*----------------------------------------------------------------------------*/
/*
    Set dsaInfoPtr->dsaInfo.forward structure.
*/
static void prvSetDsaInfoForward
(
    OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr
)
{
    /*
    dsaInfoPtr{dsaInfo{forward{
                                srcIsTagged [GT_FALSE],
                                srcHwDev [0],
                                srcIsTrunk [GT_FALSE],
                                source{ portNum [0] }
                                srcId [0],
                                egrFilterRegistered [GT_FALSE],
                                wasRouted [GT_FALSE],
                                qosProfileIndex [10],
                                dstInterface{
                                             type [CPSS_INTERFACE_VIDX_E],
                                             dstInterface.vidx [100]
                                             }
                                }
    */

    dsaInfoPtr->dsaInfo.forward.srcIsTagged = GT_FALSE;
    dsaInfoPtr->dsaInfo.forward.srcHwDev      = 0;
    dsaInfoPtr->dsaInfo.forward.srcIsTrunk  = GT_FALSE;

    dsaInfoPtr->dsaInfo.forward.source.portNum = NETIF_VALID_PHY_PORT_CNS;

    dsaInfoPtr->dsaInfo.forward.srcId = 0;

    dsaInfoPtr->dsaInfo.forward.egrFilterRegistered = GT_FALSE;
    dsaInfoPtr->dsaInfo.forward.wasRouted           = GT_FALSE;
    dsaInfoPtr->dsaInfo.forward.qosProfileIndex     = 10;
    dsaInfoPtr->dsaInfo.forward.dstInterface.type   = CPSS_INTERFACE_VIDX_E;
    dsaInfoPtr->dsaInfo.forward.dstInterface.vidx   = 100;
}

/**
* @internal prvUtfCoreClockGet function
* @endinternal
*
* @brief   This routine returns core clock per device.
*
* @param[in] dev                      -  device id
*
* @param[out] coreClock                -  core clock
*                                      GT_OK           -   Get core clock was OK
*                                      GT_BAD_PARAM    -   Invalid device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS prvUtfCoreClockGet
(
    IN  GT_U8       dev,
    OUT GT_U32      *coreClock
)
{
    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(coreClock);

    /* check if dev active and from DxChx family */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);

    *coreClock = PRV_CPSS_PP_MAC(dev)->coreClock;

    return(GT_OK);
}

