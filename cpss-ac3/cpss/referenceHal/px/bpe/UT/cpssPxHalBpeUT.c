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
* @file cpssPxHalBpeUT.c
*
* @brief Unit tests for cpssPxHalBpe that provides
* CPSS implementation for Pipe 802.1BR processing.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/px/cpssPxTypes.h>
#include "../h/cpssPxHalBpe.h"
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpssCommon/cpssPresteraDefs.h>

#define PRV_CPSS_PX_TRUNKS_NUM_CNS                  17 

/*
GT_STATUS cpssPxHalBpeNumOfChannelsGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    OUT GT_U32                           *numOfChannelsPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeNumOfChannelsGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with NULL numOfChannelsPtr.
    Expected: GT_BAD_PTR.
    1.2  Call with valid numOfChannelsPtr.
    Expected: GT_GT_OK, numOfChannelsPtr == 8173.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_U32                             numOfChannels;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with NULL numOfChannelsPtr.
            Expected: GT_BAD_PTR. */
        expect = GT_BAD_PTR;
        st = cpssPxHalBpeNumOfChannelsGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expect, st, dev);

        /*  1.2  Call with valid numOfChannelsPtr.
            Expected: GT_GT_OK, numOfChannelsPtr == 8173. */
        expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsGet(dev, &numOfChannels);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expect, st, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(8173, numOfChannels);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeNumOfChannelsGet(dev, &numOfChannels);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expect, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    expect = GT_BAD_PARAM;
    st = cpssPxHalBpeNumOfChannelsGet(dev, &numOfChannels);
    UTF_VERIFY_EQUAL1_PARAM_MAC(expect, st, dev);
}
/*
GT_STATUS cpssPxHalBpeNumOfChannelsSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          numOfUnicastChannels,
    IN  GT_U32                          numOfMulticastChannels
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeNumOfChannelsSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with valid numOfUnicastChannels == 50 and out of range numOfMulticastChannels == 8K
    Expected: GT_BAD_PARAM.
    1.2  Call with out of range numOfUnicastChannels == 4K and valid numOfMulticastChannels == 6K
    Expected: GT_BAD_PARAM.
    1.3  Call with valid numOfUnicastChannels == 0 and valid numOfMulticastChannels == 0
    Expected: GT_OK.
    1.4  Call with valid numOfUnicastChannels == 0 and valid numOfMulticastChannels == 8K-19
    Expected: GT_OK.
    1.5  Call with valid numOfUnicastChannels == 4K-19 and valid numOfMulticastChannels == 0
    Expected: GT_OK.
    1.6  Call with valid numOfUnicastChannels == 4K-19 and valid numOfMulticastChannels == 4K
    Expected: GT_OK.
    1.7  Call with valid numOfUnicastChannels == 4K-19 and valid numOfMulticastChannels == 4K+1
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_U32                             numOfUnicastChannels, numOfMulticastChannels;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with valid numOfUnicastChannels == 50 and out of range numOfMulticastChannels == 8K
            Expected: GT_BAD_PARAM. */
        numOfUnicastChannels = 50; numOfMulticastChannels = _8K; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.2  Call with out of range numOfUnicastChannels == 4K and valid numOfMulticastChannels == 6K
            Expected: GT_BAD_PARAM. */
        numOfUnicastChannels = _4K; numOfMulticastChannels = _6K; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.3  Call with valid numOfUnicastChannels == 0 and valid numOfMulticastChannels == 0
            Expected: GT_OK. */
        numOfUnicastChannels = 0; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.4  Call with valid numOfUnicastChannels == 0 and valid numOfMulticastChannels == 8K-19
            Expected: GT_OK. */
        numOfUnicastChannels = 0; numOfMulticastChannels = _8K - 19; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.5  Call with valid numOfUnicastChannels == 4K-19 and valid numOfMulticastChannels == 0
            Expected: GT_OK. */
        numOfUnicastChannels = _4K - 19; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.6  Call with valid numOfUnicastChannels == 4K-19 and valid numOfMulticastChannels == 4K
            Expected: GT_OK. */
        numOfUnicastChannels = _4K - 19; numOfMulticastChannels = _4K; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.7  Call with valid numOfUnicastChannels == 4K-19 and valid numOfMulticastChannels == 4K+1
            Expected: GT_BAD_PARAM. */
        numOfUnicastChannels = _4K-19; numOfMulticastChannels = _4K + 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        numOfUnicastChannels = 0; numOfMulticastChannels = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    numOfUnicastChannels = 0; numOfMulticastChannels = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
    UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);
}
/*
GT_STATUS cpssPxHalBpeDebugCncBlocksSet
(
    IN  GT_SW_DEV_NUM                    devNum,
    IN  GT_U32                           cncBitmap
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeDebugCncBlocksSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with cncBitmap == 4.
    Expect: GT_BAD_PARAM.
    1.2  Call with cncBitmap == 3.
    Expect: GT_OK.
    1.3  Call with cncBitmap == 0.
    Expect: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_U32                             cncBitmap;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with cncBitmap == 4.
            Expect: GT_BAD_PARAM. */
        cncBitmap = 4; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeDebugCncBlocksSet(dev, cncBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cncBitmap);

        /*  1.2  Call with cncBitmap == 3.
            Expect: GT_OK. */
        cncBitmap = 3; expect = GT_OK;
        st = cpssPxHalBpeDebugCncBlocksSet(dev, cncBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cncBitmap);

        /*  1.3  Call with cncBitmap == 0.
            Expect: GT_OK. */
        cncBitmap = 0; expect = GT_OK;
        st = cpssPxHalBpeDebugCncBlocksSet(dev, cncBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cncBitmap);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        cncBitmap = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeDebugCncBlocksSet(dev, cncBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cncBitmap);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    cncBitmap = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeDebugCncBlocksSet(dev, cncBitmap);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cncBitmap);
}
/*
GT_STATUS cpssPxHalBpeCncConfigClient
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_CNC_CLIENT_ENT      client,
    IN  GT_U32                      blockNum
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeCncConfigClient)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with valid client == CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E and out of range blockNum == 2
    Expect: GT_BAD_PARAM.
    1.2  Call with out of range client == 0xffffffff and valid blockNum == 0
    Expect: GT_BAD_PARAM.
    1.3  Call with valid client == CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E and valid blockNum == 0
    Expect: GT_BAD_PARAM. (because CNC block 0 was not set for use by BPE HAL).
    1.4  Set cncBitmap == 1 using cpssPxHalBpeDebugCncBlocksSet.
    Expect: GT_OK.
    1.5  Make same call as in 1.3.
    Expect: GT_OK.
    1.6  Call with valid client == CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E and valid blockNum == 1
    Expect: GT_BAD_PARAM. (because CNC block 1 was not set for use by BPE HAL).
    1.7  Set cncBitmap == 3 using cpssPxHalBpeDebugCncBlocksSet.
    Expect: GT_OK.
    1.8  Make same call as in 1.6.
    Expect: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_U32                             blockNum, cncBitmap;
    CPSS_PX_CNC_CLIENT_ENT             client;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with valid client == CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E and out of range blockNum == 2
            Expect: GT_BAD_PARAM. */
        client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E; blockNum = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);

        /*  1.2  Call with out of range client == 0xffffffff and valid blockNum == 0
            Expect: GT_BAD_PARAM. */
        client = 0xffffffff; blockNum = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);

        /*  1.3  Call with valid client == CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E and valid blockNum == 0
            Expect: GT_BAD_PARAM. (because CNC block 0 was not set for use by BPE HAL). */
        client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E; blockNum = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);

        /*  1.4  Set cncBitmap == 1 using cpssPxHalBpeDebugCncBlocksSet.
            Expect: GT_OK. */
        cncBitmap = 1; expect = GT_OK;
        st = cpssPxHalBpeDebugCncBlocksSet(dev, cncBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cncBitmap);

        /*  1.5  Make same call as in 1.3.
            Expect: GT_OK. */
        client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E; blockNum = 0; expect = GT_OK;
        st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);

        /*  1.6  Call with valid client == CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E and valid blockNum == 1
            Expect: GT_BAD_PARAM. (because CNC block 1 was not set for use by BPE HAL). */
        client = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E; blockNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);

        /*  1.7  Set cncBitmap == 3 using cpssPxHalBpeDebugCncBlocksSet.
            Expect: GT_OK. */
        cncBitmap = 3; expect = GT_OK;
        st = cpssPxHalBpeDebugCncBlocksSet(dev, cncBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cncBitmap);

        /*  1.8  Make same call as in 1.6.
            Expect: GT_OK. */
        client = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E; blockNum = 1; expect = GT_OK;
        st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E; blockNum = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E; blockNum = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeCncConfigClient(dev, client, blockNum);
    UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, client, blockNum);
}
/*
GT_STATUS cpssPxHalBpeInterfaceTypeSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT  mode
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeInterfaceTypeSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with NULL interfacePtr and valid mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E.
    Expected: GT_BAD_PTR.
    1.2  Call with invalid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_NONE_E, {dev, 0}} and valid mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E.
    Expected: GT_BAD_PARAM.
    1.3  Call with out of range interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, PRV_CPSS_PX_PORTS_NUM_CNS}} and valid mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E.
    Expected: GT_BAD_PARAM.
    1.4  Call with valid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, 0}} and out of range mode == 0xffffffff.
    Expected: GT_BAD_PARAM.
    1.5. Call with valid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]}}
    and valid mode == [CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E].
    Expected: GT_OK.
    1.6. Call with valid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]}}
    and valid mode == [CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E, CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E].
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT  mode;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        interface.devPort.devNum = dev;
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with NULL interfacePtr and valid mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E.
            Expected: GT_BAD_PTR. */
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_BAD_PTR;
        st = cpssPxHalBpeInterfaceTypeSet(dev, NULL, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);

        /*  1.2  Call with invalid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_NONE_E, {dev, 0}} and valid mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.3  Call with out of range interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, PRV_CPSS_PX_PORTS_NUM_CNS}} and valid mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.4  Call with valid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, 0}} and out of range mode == 0xffffffff.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = 0xffffffff; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.5. Call with valid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]}}
            and valid mode == [CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E].
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.6. Call with valid interfacePtr == {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {dev, [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]}}
            and valid mode == [CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E, CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E].
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
    mode = CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
    UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);
}
/*
GT_STATUS cpssPxHalBpeUpstreamQosMapSet
(
    IN  GT_SW_DEV_NUM       devNum,    
    IN  GT_U32              pcp,
    IN  GT_U32              queue
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeUpstreamQosMapSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, queue} == {0, 0}.
    Expected: GT_BAD_PARAM (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as upstream port).
    1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as upstream port.
    Expected: GT_OK.
    1.3. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and valid {pcp, queue} == {0, 0}.
    Expected: GT_BAD_PARAM.
    1.4. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid pcp == 0 and out of range queue == 8.
    Expected: GT_BAD_PARAM.
    1.5. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid queue == 0 and out of range pcp == CPSS_PCP_RANGE_CNS.
    Expected: GT_BAD_PARAM.
    1.6. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, queue} == {0, 0},{CPSS_PCP_RANGE_CNS - 1, 7}.
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;    
    GT_U32                             pcp, queue;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);       

        /*  1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as upstream port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);        

        /*  1.4. Call with valid pcp == 0 and out of range queue == 8.
            Expected: GT_BAD_PARAM. */
        pcp = 0; queue = 8; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);

        pcp = 0; queue = 8; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);

        /*  1.5. Call with valid queue == 0 and out of range pcp == CPSS_PCP_RANGE_CNS.
            Expected: GT_BAD_PARAM. */
        pcp = CPSS_PCP_RANGE_CNS; queue = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);

        pcp = CPSS_PCP_RANGE_CNS; queue = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);

        /*  1.6. Call with valid {pcp, queue} == {0, 0},{CPSS_PCP_RANGE_CNS - 1, 7}.
            Expected: GT_OK. */
        pcp = 0; queue = 0; expect = GT_OK;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);

        pcp = 0; queue = 0; expect = GT_OK;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);

        pcp = CPSS_PCP_RANGE_CNS - 1; queue = 7; expect = GT_OK;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);

        pcp = CPSS_PCP_RANGE_CNS - 1; queue = 7; expect = GT_OK;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        pcp = 0; queue = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    pcp = 0; queue = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeUpstreamQosMapSet(dev, pcp, queue);
    UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, pcp, queue);
}
/*
GT_STATUS cpssPxHalBpeExtendedQosMapSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U32          pcp,
    IN  GT_U32          queue
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeExtendedQosMapSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, queue} == {0, 0}.
    Expected: GT_BAD_PARAM (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended/cascade port).
    1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as {extended, cascade} port.
    Expected: GT_OK.
    1.3. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and valid {pcp, queue} == {0, 0}.
    Expected: GT_BAD_PARAM.
    1.4. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid pcp == 0 and out of range queue == 8.
    Expected: GT_BAD_PARAM.
    1.5. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid queue == 0 and out of range pcp == CPSS_PCP_RANGE_CNS.
    Expected: GT_BAD_PARAM.
    1.6. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, queue} == {0, 0},{CPSS_PCP_RANGE_CNS - 1, 7}.
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_PORT_NUM                        portNum;
    GT_U32                             pcp, queue;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, queue} == {0, 0}.
            Expected: GT_BAD_PARAM (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended/cascade port). */
        portNum = 0; pcp = 0; queue = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = 0; queue = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        /*  1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as {extended, cascade} port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.3. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and valid {pcp, queue} == {0, 0}.
            Expected: GT_BAD_PARAM. */
        portNum = PRV_CPSS_PX_PORTS_NUM_CNS; pcp = 0; queue = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        /*  1.4. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid pcp == 0 and out of range queue == 8.
            Expected: GT_BAD_PARAM. */
        portNum = 0; pcp = 0; queue = 8; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = 0; queue = 8; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        /*  1.5. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid queue == 0 and out of range pcp == CPSS_PCP_RANGE_CNS.
            Expected: GT_BAD_PARAM. */
        portNum = 0; pcp = CPSS_PCP_RANGE_CNS; queue = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = CPSS_PCP_RANGE_CNS; queue = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        /*  1.6. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, queue} == {0, 0},{CPSS_PCP_RANGE_CNS - 1, 7}.
            Expected: GT_OK. */
        portNum = 0; pcp = 0; queue = 0; expect = GT_OK;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = 0; queue = 0; expect = GT_OK;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        portNum = 0; pcp = CPSS_PCP_RANGE_CNS - 1; queue = 7; expect = GT_OK;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = CPSS_PCP_RANGE_CNS - 1; queue = 7; expect = GT_OK;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        portNum = 0; pcp = 0; queue = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    portNum = 0; pcp = 0; queue = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeExtendedQosMapSet(dev, portNum, pcp, queue);
    UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, queue);
}
/*
GT_STATUS cpssPxHalBpeInterfaceExtendedPcidSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              pcid
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeInterfaceExtendedPcidSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Set numOfUnicastChannels = 100; numOfMulticastChannels = 0
    Expected: GT_OK.
    1.2  Call with valid pcid == 1 and invalid interfacePtr == NULL.
    Expected: GT_BAD_PTR.
    1.3  Call with valid pcid == 1 and
    out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
    Expected: GT_BAD_PARAM.
    1.4  Call with valid pcid == 1 and
    out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.5  Call with valid pcid == 1 and
    invalid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.6  Call with valid pcid == 1, and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, PRV_CPSS_PX_PORTS_NUM_CNS - 2}.
    Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port).
    1.7  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
    Expected: GT_OK.
    1.8  Call valid  portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and out of range pcid == 100.
    Expected: GT_BAD_PARAM.
    1.9  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid pcid == [0, 99].
    Expected: GT_OK.
    1.10 Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid pcid == [1, 98].
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_U32                             numOfUnicastChannels;
    GT_U32                             numOfMulticastChannels;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;
    GT_U32                             pcid;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Set numOfUnicastChannels = 100; numOfMulticastChannels = 0
            Expected: GT_OK. */
        numOfUnicastChannels = 100; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.2  Call with valid pcid == 1 and invalid interfacePtr == NULL.
            Expected: GT_BAD_PTR. */
        pcid = 1; expect = GT_BAD_PTR;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, NULL, pcid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, pcid);

        /*  1.3  Call with valid pcid == 1 and
            out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS;
        pcid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        /*  1.4  Call with valid pcid == 1 and
            out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        interface.devPort.portNum = 0;
        pcid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        /*  1.5  Call with valid pcid == 1 and
            invalid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        /*  1.6  Call with valid pcid == 1, and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, PRV_CPSS_PX_PORTS_NUM_CNS - 2}.
            Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port). */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        /*  1.7  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.8  Call valid  portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and out of range pcid == 100.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 100; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        pcid = 100; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        /*  1.9  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid pcid == [0, 99].
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 0; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        pcid = 0; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 99; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        pcid = 99; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        /*  1.10 Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid pcid == [1, 98].
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 1; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        pcid = 1; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 98; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        pcid = 98; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
    interface.devPort.devNum = dev;
    interface.devPort.portNum = 0;
    pcid = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
    UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);
}
/*
GT_STATUS cpssPxHalBpePortExtendedUpstreamSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *upstreamPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpePortExtendedUpstreamSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
    valid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 1}.
    Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port and portNum[1] was not set as upstream port).
    1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
    Expected: GT_OK.
    1.3. Make same call as in 1.1.
    Expected: GT_BAD_PARAM. (because portNum[1] was not set as upstream port).
    1.4  Set portNum[1] as upstream port.
    Expected: GT_OK.
    1.5. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
    invalid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 1}.
    Expected: GT_BAD_PARAM.
    1.6. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and
    valid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 1}.
    Expected: GT_BAD_PARAM.
    1.7  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
    out of range upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
    Expected: GT_BAD_PARAM.
    1.8. Call with valid portNum[0] and invalid upstreamPtr == NULL.
    Expected: GT_BAD_PTR.
    1.9 Set numOfUnicastChannels = 100; numOfMulticastChannels = 0
    Expected: GT_OK.
    1.10  Set pcid == [1,2] for portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
    Expected: GT_OK.
    1.11 Create unicast e-channel with cid = [1,2] and
    downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]}.
    Expected: GT_OK.
    1.12 Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
    valid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 1}.
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev, activeDev = 0;
    GT_PORT_NUM                        portNum;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC upstreamInterface, downStreamInterface, interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;
    GT_U32                             pcid, cid, numOfUnicastChannels, numOfMulticastChannels;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        upstreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        upstreamInterface.devPort.devNum = dev;

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
            valid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 1}.
            Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port and portNum[1] was not set as upstream port). */
        portNum = 0; upstreamInterface.devPort.portNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; upstreamInterface.devPort.portNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        /*  1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.3. Make same call as in 1.1.
            Expected: GT_BAD_PARAM. (because portNum[1] was not set as upstream port). */
        portNum = 0; upstreamInterface.devPort.portNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; upstreamInterface.devPort.portNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        /*  1.4  Set portNum[1] as upstream port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 1;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.5. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
            invalid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 1}.
            Expected: GT_BAD_PARAM. */
        upstreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;

        portNum = 0; upstreamInterface.devPort.portNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; upstreamInterface.devPort.portNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        /*  1.6. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and
            valid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 1}.
            Expected: GT_BAD_PARAM. */
        upstreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS; upstreamInterface.devPort.portNum = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        /*  1.7  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
            out of range upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
            Expected: GT_BAD_PARAM. */
        portNum = 0; upstreamInterface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; upstreamInterface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        /*  1.8. Call with valid portNum[0] and invalid upstreamPtr == NULL.
            Expected: GT_BAD_PTR. */
        portNum = 0; expect = GT_BAD_PTR;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        /*  1.9 Set numOfUnicastChannels = 100; numOfMulticastChannels = 0
            Expected: GT_OK. */
        numOfUnicastChannels = 100; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.10  Set pcid == [1,2] for portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        pcid = 1; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        pcid = 2; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        /*  1.11 Create unicast e-channel with cid = [1,2] and
            downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]}.
            Expected: GT_OK. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 1; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        cid = 2; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.12 Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and
            valid upstreamPtr == {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 1}.
            Expected: GT_OK. */
        portNum = 0; upstreamInterface.devPort.portNum = 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; upstreamInterface.devPort.portNum = 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        activeDev = dev;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    upstreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
    upstreamInterface.devPort.portNum = 1;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        portNum = 0; upstreamInterface.devPort.devNum = activeDev; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

        portNum = 0; upstreamInterface.devPort.devNum = dev; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpePortExtendedUpstreamSet(activeDev, portNum, &upstreamInterface);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, activeDev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);
    }

    /* 3. Call function with out of bound value for device id. */
    portNum = 0; upstreamInterface.devPort.devNum = activeDev; expect = GT_BAD_PARAM;
    st = cpssPxHalBpePortExtendedUpstreamSet(PRV_CPSS_MAX_PP_DEVICES_CNS, portNum, &upstreamInterface);
    UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, PRV_CPSS_MAX_PP_DEVICES_CNS, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);

    portNum = 0; upstreamInterface.devPort.devNum = PRV_CPSS_MAX_PP_DEVICES_CNS; expect = GT_BAD_PARAM;
    st = cpssPxHalBpePortExtendedUpstreamSet(activeDev, portNum, &upstreamInterface);
    UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, activeDev, portNum, upstreamInterface.devPort.devNum, upstreamInterface.devPort.portNum);
}
/*
GT_STATUS cpssPxHalBpePortExtendedQosDefaultPcpDeiSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U32          pcp,
    IN  GT_U32          dei
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpePortExtendedQosDefaultPcpDeiSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, dei} == {0, 0}.
    Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port).
    1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
    Expected: GT_OK.
    1.3. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and valid {pcp, dei} == {0, 0}.
    Expected: GT_BAD_PARAM.
    1.4  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid pcp == 0 and out of range dei == CPSS_DEI_RANGE_CNS.
    Expected: GT_BAD_PARAM.
    1.5  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid dei == 0 and out of range pcp == CPSS_PCP_RANGE_CNS.
    Expected: GT_BAD_PARAM.
    1.6  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, dei} = [{0, 0}, {CPSS_PCP_RANGE_CNS - 1, CPSS_DEI_RANGE_CNS - 1}].
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_U32                             pcp, dei;
    GT_PORT_NUM                        portNum;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, dei} == {0, 0}.
            Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port). */
        portNum = 0; pcp = 0; dei = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = 0; dei = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        /*  1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.3. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and valid {pcp, dei} == {0, 0}.
            Expected: GT_BAD_PARAM. */
        portNum = PRV_CPSS_PX_PORTS_NUM_CNS; pcp = 0; dei = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        /*  1.4  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid pcp == 0 and out of range dei == CPSS_DEI_RANGE_CNS.
            Expected: GT_BAD_PARAM. */
        portNum = 0; pcp = 0; dei = CPSS_DEI_RANGE_CNS; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = 0; dei = CPSS_DEI_RANGE_CNS; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        /*  1.5  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2], valid dei == 0 and out of range pcp == CPSS_PCP_RANGE_CNS.
            Expected: GT_BAD_PARAM. */
        portNum = 0; pcp = CPSS_PCP_RANGE_CNS; dei = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = CPSS_PCP_RANGE_CNS; dei = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        /*  1.6  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid {pcp, dei} = [{0, 0}, {CPSS_PCP_RANGE_CNS - 1, CPSS_DEI_RANGE_CNS - 1}].
            Expected: GT_OK. */
        portNum = 0; pcp = 0; dei = 0; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = 0; dei = 0; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        portNum = 0; pcp = CPSS_PCP_RANGE_CNS - 1; dei = CPSS_DEI_RANGE_CNS - 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; pcp = CPSS_PCP_RANGE_CNS - 1; dei = CPSS_DEI_RANGE_CNS - 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        portNum = 0; pcp = 0; dei = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    portNum = 0; pcp = 0; dei = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, portNum, pcp, dei);
    UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, portNum, pcp, dei);
}
/*
GT_STATUS cpssPxHalBpePortExtendedQosMapSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PORT_NUM                 portNum,
    IN  CPSS_PX_HAL_BPE_QOSMAP_STC  qosMapArr[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS]
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpePortExtendedQosMapSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and validAllZerosQosMap.
    Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port).
    1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
    Expected: GT_OK.
    1.3. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and validAllZerosQosMap.
    Expected: GT_BAD_PARAM.
    1.4  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and out of range invalidQosMap.
    Expected: GT_BAD_PARAM.
    1.5  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and invalid qosMap == NULL.
    Expected: GT_BAD_PTR.
    1.6  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid qosMap[validAllZerosQosMap, validAllOnesQosMap].
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    CPSS_PX_HAL_BPE_QOSMAP_STC         validAllZerosQosMap[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];
    CPSS_PX_HAL_BPE_QOSMAP_STC         validAllOnesQosMap[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];
    CPSS_PX_HAL_BPE_QOSMAP_STC         invalidQosMap[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];
    GT_U32                             pcp, dei;
    GT_PORT_NUM                        portNum;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;

    /* fill values in qos maps */
    for(pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
    {
        for(dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
        {
            validAllZerosQosMap[pcp][dei].newPcp = 0;
            validAllZerosQosMap[pcp][dei].newDei = 0;

            validAllOnesQosMap[pcp][dei].newPcp = CPSS_PCP_RANGE_CNS - 1;
            validAllOnesQosMap[pcp][dei].newDei = CPSS_DEI_RANGE_CNS - 1;

            invalidQosMap[pcp][dei].newPcp = CPSS_PCP_RANGE_CNS;
            invalidQosMap[pcp][dei].newDei = CPSS_DEI_RANGE_CNS;
        }
    }
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and validAllZerosQosMap.
            Expected: GT_BAD_PARAM. (because portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] was not set as extended port). */
        portNum = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllZerosQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllZerosQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        /*  1.2  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.3. Call with out of range portNum[PRV_CPSS_PX_PORTS_NUM_CNS] and validAllZerosQosMap.
            Expected: GT_BAD_PARAM. */
        portNum = PRV_CPSS_PX_PORTS_NUM_CNS; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllZerosQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        /*  1.4  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and out of range invalidQosMap.
            Expected: GT_BAD_PARAM. */
        portNum = 0; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, invalidQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, invalidQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        /*  1.5  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and invalid qosMap == NULL.
            Expected: GT_BAD_PTR. */
        portNum = 0; expect = GT_BAD_PTR;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; expect = GT_BAD_PTR;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        /*  1.6  Call with valid portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] and valid qosMap[validAllZerosQosMap, validAllOnesQosMap].
            Expected: GT_OK. */
        portNum = 0; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllZerosQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllZerosQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        portNum = 0; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllOnesQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);

        portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2; expect = GT_OK;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllOnesQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        portNum = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllZerosQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    portNum = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpePortExtendedQosMapSet(dev, portNum, validAllZerosQosMap);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, portNum);
}
/*
GT_STATUS cpssPxHalBpeUnicastEChannelCreate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *downStreamInterfacePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeUnicastEChannelCreate)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Set numOfUnicastChannels == 100, numOfMulticastChannels == 0.
    Expected: GT_OK.
    1.2  Call with valid cid == 2 and invalid downStreamInterfacePtr == NULL.
    Expected: GT_BAD_PTR.
    1.3  Call with valid cid == 2 and
    out of range downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
    Expected: GT_BAD_PARAM.
    1.4  Call with valid cid == 2 and
    out of range downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.5  Call with valid cid == 2 and
    invalid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.6  Call with valid cid == [2, 99], and
    valid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == [0, 1]}.
    Expected: GT_BAD_STATE.
    1.7  Set portNum[0, 1] as extended port and portNum[PRV_CPSS_PX_PORTS_NUM_CNS - 2] as upstream port.
    Expected: GT_OK.
    1.8  Make same call as in 1.6.
    Expected: GT_BAD_STATE.
    1.9  Set pcid == [2, 99] for portNum[0, 1].
    Expected: GT_OK.
    1.10  Call with valid cid == [1, 98], and
    valid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == [0, 1]}.
    Expected: GT_BAD_STATE.
    1.11 Call with out of range cid = 100 and valid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.12 Set numOfUnicastChannels == 0, numOfMulticastChannels == 0.
    Expected: GT_OK.
    1.13 Make same call as in 1.6.
    Expected: GT_BAD_PARAM.
    1.14 Make same call as in 1.1.
    Expected: GT_OK.
    1.15 Make same call as in 1.6.
    Expected: GT_OK.
    1.16 Make same call as in 1.6.
    Expected: GT_ALREADY_EXIST.
    1.17 Delete channel with cid = [2, 99] using cpssPxHalBpeUnicastEChannelDelete API.
    Expected: GT_OK.
    1.18 Make same call as in 1.17.
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC downStreamInterface, upStreamInterface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;
    GT_U32                             cid, pcid;
    GT_U32                             numOfUnicastChannels, numOfMulticastChannels;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Set numOfUnicastChannels == 100, numOfMulticastChannels == 0.
            Expected: GT_OK. */
        numOfUnicastChannels = 100; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.2  Call with valid cid == 2 and invalid downStreamInterfacePtr == NULL.
            Expected: GT_BAD_PTR. */
        cid = 2; expect = GT_BAD_PTR;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.3  Call with valid cid == 2 and
            out of range downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
            Expected: GT_BAD_PARAM. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS;
        cid = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);
#if 0
        /*  1.4  Call with valid cid == 2 and
            out of range downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        downStreamInterface.devPort.portNum = 0;
        cid = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);
#endif
        /*  1.5  Call with valid cid == 2 and
            invalid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.6  Call with valid cid == [2, 99], and
            valid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == [0, 1]}.
            Expected: GT_BAD_STATE. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 2; expect = GT_BAD_STATE;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 1;
        cid = 99; expect = GT_BAD_STATE;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.7  Set portNum[0, 1] as extended port and portNum[PRV_CPSS_PX_PORTS_NUM_CNS - 2] as upstream port.
            Expected: GT_OK. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; downStreamInterface.devPort.devNum = dev; downStreamInterface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &downStreamInterface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum, mode);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; downStreamInterface.devPort.devNum = dev; downStreamInterface.devPort.portNum = 1;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &downStreamInterface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum, mode);

        upStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; upStreamInterface.devPort.devNum = dev;
        upStreamInterface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &upStreamInterface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, upStreamInterface.type, upStreamInterface.devPort.devNum, upStreamInterface.devPort.portNum, mode);

        /*  1.8  Make same call as in 1.6.
            Expected: GT_BAD_STATE. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 2; expect = GT_BAD_STATE;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 1;
        cid = 99; expect = GT_BAD_STATE;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.9  Set pcid == [2, 99] for portNum[0, 1].
            Expected: GT_OK. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        pcid = 2; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &downStreamInterface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum, pcid);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 1;
        pcid = 99; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &downStreamInterface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum, pcid);

        /*  1.10  Call with valid cid == [1, 98], and
            valid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == [0, 1]}.
            Expected: GT_BAD_STATE. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 1; expect = GT_BAD_STATE;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 1;
        cid = 98; expect = GT_BAD_STATE;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.11 Call with out of range cid = 100 and
            valid downStreamInterfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 100; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.12 Set numOfUnicastChannels == 0, numOfMulticastChannels == 0.
            Expected: GT_OK. */
        numOfUnicastChannels = 0; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.13 Make same call as in 1.6.
            Expected: GT_BAD_PARAM. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 1;
        cid = 99; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.14 Make same call as in 1.1.
            Expected: GT_OK. */
        numOfUnicastChannels = 100; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.15 Make same call as in 1.6.
            Expected: GT_OK. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 2; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 1;
        cid = 99; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.16 Make same call as in 1.6.
            Expected: GT_ALREADY_EXIST. */
        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 0;
        cid = 2; expect = GT_ALREADY_EXIST;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        downStreamInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        downStreamInterface.devPort.devNum = dev;
        downStreamInterface.devPort.portNum = 1;
        cid = 99; expect = GT_ALREADY_EXIST;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        /*  1.17 Delete channel with cid = [0, 99] using cpssPxHalBpeUnicastEChannelDelete API.
            Expected: GT_OK. */
        cid = 2; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        cid = 99; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.18 Make same call as in 1.17.
            Expected: GT_NOT_FOUND. */
        cid = 2; expect = GT_NOT_FOUND;
        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        cid = 99; expect = GT_NOT_FOUND;
        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        cid = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

        cid = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    cid = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &downStreamInterface);
    UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, downStreamInterface.type, downStreamInterface.devPort.devNum, downStreamInterface.devPort.portNum);

    cid = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeUnicastEChannelDelete(dev, cid);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);
}
/*
GT_STATUS cpssPxHalBpePortExtendedUntaggedVlanAdd
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              vid
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpePortExtendedUntaggedVlanAdd)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with valid vid == 1 and invalid interfacePtr == NULL.
    Expected: GT_BAD_PTR.
    1.2  Call with valid vid == 1 and
    out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
    Expected: GT_BAD_PARAM.
    1.3  Call with valid vid == 1 and
    out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.4  Call with valid vid == 1 and
    invalid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.5  Call with valid vid == 1, and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM. (because portNum[0] was not set as extended port).
    1.6  Set portNum[0, 1] as extended port.
    Expected: GT_OK.
    1.7  Call with out of range vid = 0xFFFF and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.8  Call with valid vid = [1, 0xFFF] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
    Expected: GT_OK.
    1.9  Call with valid vid = [2, 0xFFF - 1] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;
    GT_U32                             vid;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with valid vid == 1 and invalid interfacePtr == NULL.
            Expected: GT_BAD_PTR. */
        vid = 1; expect = GT_BAD_PTR;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, NULL, vid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, vid);

        /*  1.2  Call with valid vid == 1 and
            out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.3  Call with valid vid == 1 and
            out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.4  Call with valid vid == 1 and
            invalid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.5  Call with valid vid == 1, and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. (because portNum[0] was not set as extended port). */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.6  Set portNum[0, 1] as extended port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 1;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.7  Call with out of range vid = 0xFFFF and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 0xFFFF; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.8  Call with valid vid = [1, 0xFFF] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 1;
        vid = 0xFFF; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.9  Call with valid vid = [2, 0xFFF - 1] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 2; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 1;
        vid = 0xFFF - 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
    interface.devPort.devNum = dev;
    interface.devPort.portNum = 0;
    vid = 1; expect = GT_BAD_PARAM;
    st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &interface, vid);
    UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);
}
/*
GT_STATUS cpssPxHalBpePortExtendedUntaggedVlanDel
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              vid
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpePortExtendedUntaggedVlanDel)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with valid vid == 1 and invalid interfacePtr == NULL.
    Expected: GT_BAD_PTR.
    1.2  Call with valid vid == 1 and
    out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
    Expected: GT_BAD_PARAM.
    1.3  Call with valid vid == 1 and
    out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.4  Call with valid vid == 1 and
    invalid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.5  Call with valid vid == 1, and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM. (because portNum[0] was not set as extended port).
    1.6  Set portNum[0, 1] as extended port.
    Expected: GT_OK.
    1.7  Call with out of range vid = 0xFFFF and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
    Expected: GT_BAD_PARAM.
    1.8  Call with valid vid = [1, 0xFFF] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
    Expected: GT_OK.
    1.9  Call with valid vid = [2, 0xFFF - 1] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;
    GT_U32                             vid;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with valid vid == 1 and invalid interfacePtr == NULL.
            Expected: GT_BAD_PTR. */
        vid = 1; expect = GT_BAD_PTR;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, NULL, vid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, vid);

        /*  1.2  Call with valid vid == 1 and
            out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == PRV_CPSS_PX_PORTS_NUM_CNS}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.3  Call with valid vid == 1 and
            out of range interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == PRV_CPSS_MAX_PP_DEVICES_CNS, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.4  Call with valid vid == 1 and
            invalid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.5  Call with valid vid == 1, and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. (because portNum[0] was not set as extended port). */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.6  Set portNum[0, 1] as extended port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 1;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.7  Call with out of range vid = 0xFFFF and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0}.
            Expected: GT_BAD_PARAM. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 0xFFFF; expect = GT_BAD_PARAM;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.8  Call with valid vid = [1, 0xFFF] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 1;
        vid = 0xFFF; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        /*  1.9  Call with valid vid = [2, 0xFFF - 1] and valid interfacePtr = {type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E, devNum == dev, portNum == 0, 1}.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 2; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 1;
        vid = 0xFFF - 1; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        interface.devPort.devNum = dev;
        interface.devPort.portNum = 0;
        vid = 1; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
    interface.devPort.devNum = dev;
    interface.devPort.portNum = 0;
    vid = 1; expect = GT_BAD_PARAM;
    st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &interface, vid);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, vid);
}
/*
GT_STATUS cpssPxHalBpeTrunkCreate
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_TRUNK_ID                     trunkId,
    IN  CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT  trunkType
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeTrunkCreate)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with valid trunkId == 1 and invalid trunkType == 0xffff.
    Expected: GT_BAD_PARAM.
    1.2  Call with out of range trunkId == PRV_CPSS_PX_TRUNKS_NUM_CNS and valid trunkType == CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E.
    Expected: GT_BAD_PARAM.
    1.3  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and
    valid trunkType == [CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E, CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E].
    Expected: GT_OK.
    1.4  Make same call as in 1.3.
    Expected: GT_ALREADY_EXIST.
    1.5  Call cpssPxHalBpeTrunkRemove API with trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1].
    Expected: GT_OK.
    1.6  Make same call as in 1.5
    Expected: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_TRUNK_ID                        trunkId;
    CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT     trunkType;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with valid trunkId == 1 and invalid trunkType == 0xffff.
            Expected: GT_BAD_PARAM. */
        trunkId = 1; trunkType = 0xffff; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        /*  1.2  Call with out of range trunkId == PRV_CPSS_PX_TRUNKS_NUM_CNS and valid trunkType == CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E.
            Expected: GT_BAD_PARAM. */
        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS; trunkType = CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        /*  1.3  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and
            valid trunkType == [CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E, CPSS_PX_HAL_BPE_TRUNK_EXTENDED].
            Expected: GT_OK. */
        trunkId = 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        /*  1.4  Make same call as in 1.3.
            Expected: GT_ALREADY_EXIST. */
        trunkId = 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E; expect = GT_ALREADY_EXIST;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E; expect = GT_ALREADY_EXIST;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        /*  1.5  Call cpssPxHalBpeTrunkRemove API with trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1].
            Expected: GT_OK. */
        trunkId = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        /*  1.6  Make same call as in 1.5
            Expected: GT_OK. */
        trunkId = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        trunkId = 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        trunkId = 1; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    trunkId = 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
    UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

    trunkId = 1; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeTrunkRemove(dev, trunkId);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
}
/*
GT_STATUS cpssPxHalBpeTrunkLoadBalanceModeSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT loadBalanceMode
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeTrunkLoadBalanceModeSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with invalid loadBalanceMode == 0xffffffff.
    Expected: GT_BAD_PARAM.
    1.2  Call with valid loadBalanceMode == CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E.
    Expected: GT_OK.
    1.3  Make same call as in 1.2.
    Expected: GT_OK.
    1.4  Call with valid loadBalanceMode == CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_PORT_BASE_E.
    Expected: GT_OK.
    1.5  Make same call as in 1.4.
    Expected: GT_OK.

*/
    GT_STATUS                                   st, expect;
    GT_U8                                       dev;
    CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT mode;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Call with invalid loadBalanceMode == 0xffffffff.
            Expected: GT_BAD_PARAM. */
        mode = 0xffffffff; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);

        /*  1.2  Call with valid loadBalanceMode == CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E.
            Expected: GT_OK. */
        mode = CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);

        /*  1.3  Make same call as in 1.2.
            Expected: GT_OK. */
        mode = CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);

        /*  1.4  Call with valid loadBalanceMode == CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E.
            Expected: GT_NOT_SUPPORTED for A0; GT_OK for A1. */
        expect = (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) ? GT_OK : GT_NOT_SUPPORTED);        
        mode = CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E; 
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);

        /*  1.5  Make same call as in 1.4.
            Expected: GT_NOT_SUPPORTED for A0; GT_OK for A1. */
        mode = CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E; 
        expect = (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) ? GT_OK : GT_NOT_SUPPORTED);  
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        mode = CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    mode = CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, mode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, mode);
}
/*
GT_STATUS cpssPxHalBpeTrunkPortsAdd
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_TRUNK_ID                           trunkId,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC    *portListPtr,
    IN  GT_U8                                 portListLen
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeTrunkPortsAdd)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Set portNum[0, 1] as upstream port, portNum[2, 3] as extended port,
    portNum[PRV_CPSS_PX_PORTS_NUM_CNS - 5, PRV_CPSS_PX_PORTS_NUM_CNS - 4] as internal port and
    portNum[PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as cascade port.
    Expected: GT_OK.
    1.2  Call with out of range trunkId == [0, PRV_CPSS_PX_TRUNKS_NUM_CNS] and with same port type lists:
    portsList == [0, 1], [PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
    Expected: GT_BAD_PARAM.
    1.3  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and with same type portLists:
    portsList == [0, 1], [PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
    Expected: GT_BAD_PARAM. (because trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] was not created).
    1.4  Create trunk with trunkId == [1, 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and
    trunkType == [CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E, CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E, CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E, CPSS_PX_HAL_BPE_TRUNK_CASCADE_E]
    Expected: GT_OK.
    1.5  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and
    out of range portsList == [PRV_CPSS_PX_PORTS_NUM_CNS, PRV_CPSS_PX_PORTS_NUM_CNS + 1]
    Expected: GT_BAD_PARAM.
    1.6 Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and NULL portList with length 1.
    Expected: GT_BAD_PTR.
    1.7  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and with wrong type portLists:
    portsList == [2, 3], [PRV_CPSS_PX_PORTS_NUM_CNS - 5, PRV_CPSS_PX_PORTS_NUM_CNS - 4]
    Expected: GT_BAD_PARAM.
    1.8  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and with mixed type portLists (with one port of the correct type):
    portList == [0, 2], [1, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
    Expected: GT_BAD_PARAM.
    1.9  Call with
    valid trunkId[1] and valid portList == [0, 1].
    valid trunkId[2] and valid portList == [2, 3].
    valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 2] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 5, PRV_CPSS_PX_PORTS_NUM_CNS - 4].
    valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
    Expected: GT_OK.
    1.10 Set portNum[4] as extended port. Set numOfUnicastChannels == 2 and numOfMulticastChannels == 0.
    Set pcid == 1 for portNum[4]. Set trunkId == 1 as upstream of portNum[4]. Create unicast E-channel with cid == 1 and
    upstream interface == trunkId[1]
    Expected: GT_OK.
    1.11 Call with valid trunkId[1] and valid portList == [0, 1].
    Expected: GT_OK.
    1.12 Remove trunkId[1, 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] using cpssPxHalBpeTrunkRemove API
    Expected: GT_BAD_STATE. (because ports were not deleted from trunks).
    1.13 Call cpssPxHalBpeTrunkPortsDelete API with trunkId[1, 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1]
    and empty portLists.
    Expected: GT_OK.
    1.14 Call cpssPxHalBpeTrunkPortsDelete API with
    valid trunkId[1] and valid portList == [0].
    valid trunkId[2] and valid portList == [2].
    valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 2] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 5]
    valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 3]
    Expected: GT_OK.
    1.15 Make same call as in 1.10
    Expected: GT_BAD_STATE. (because not ALL ports were not deleted from trunks).
    1.16 Call cpssPxHalBpeTrunkPortsDelete API with
    valid trunkId[1] and valid portList == [1].
    valid trunkId[2] and valid portList == [3].
    valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 2] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 4]
    valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 2]
    Expected: GT_OK.
    1.17 Make same call as in 1.12
    Expected: GT_BAD_STATE (because unicase E-channel was not deleted).
    1.18 Delete unicast E-channel with cid == 1.
    Expected: GT_OK.
    1.19 Make same call as in 1.12
    Expected: GT_OK.
*/
    GT_STATUS                             st, expect;
    GT_U8                                 dev;
    GT_TRUNK_ID                           trunkId;
    CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT        trunkType;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC    *portListPtr, interface;
    GT_U8                                 portListLen;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT    mode;
    GT_U32                                numOfUnicastChannels, numOfMulticastChannels, pcid/*,cid*/;
    GT_PORT_NUM                           portNum;

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoUpstPorts[2] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 0}},
                                                          {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 1}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoExtdPorts[2] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 2}},
                                                          {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 3}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoIntrPorts[2] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 5}},
                                                          {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 4}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoCscdPorts[2] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 3}},
                                                          {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 2}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC oneUpstOneExtdPorts[2] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 0}},
                                                                 {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 2}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC oneUpstOneCscdPorts[2] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 1}},
                                                                 {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 2}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC oneOutOfTwoUpstPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 0}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoOutOfTwoUpstPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 1}}};
#if 0
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC oneOutOfTwoExtdPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 2}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoOutOfTwoExtdPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 3}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC oneOutOfTwoIntrPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 5}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoOutOfTwoIntrPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 4}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC oneOutOfTwoCscdPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 3}}};

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC twoOutOfTwoCscdPorts[1] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 2}}};
#endif
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC outOfRangePorts[2] = {{CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS}},
                                                             {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS + 1}}};

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        twoUpstPorts[0].devPort.devNum = twoUpstPorts[1].devPort.devNum = twoExtdPorts[0].devPort.devNum = twoExtdPorts[1].devPort.devNum = dev;
        twoIntrPorts[0].devPort.devNum = twoIntrPorts[1].devPort.devNum = twoCscdPorts[0].devPort.devNum = twoCscdPorts[1].devPort.devNum = dev;
        oneUpstOneExtdPorts[0].devPort.devNum = oneUpstOneExtdPorts[1].devPort.devNum = dev;
        oneUpstOneCscdPorts[0].devPort.devNum = oneUpstOneCscdPorts[1].devPort.devNum = dev;
        oneOutOfTwoUpstPorts[0].devPort.devNum = twoOutOfTwoUpstPorts[0].devPort.devNum = dev;
#if 0
        oneOutOfTwoExtdPorts[0].devPort.devNum = twoOutOfTwoExtdPorts[0].devPort.devNum = dev;
        oneOutOfTwoIntrPorts[0].devPort.devNum = twoOutOfTwoIntrPorts[0].devPort.devNum = dev;
        oneOutOfTwoCscdPorts[0].devPort.devNum = twoOutOfTwoCscdPorts[0].devPort.devNum = dev;
#endif
        outOfRangePorts[0].devPort.devNum = outOfRangePorts[1].devPort.devNum = dev;

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Set portNum[0, 1] as upstream port, portNum[2, 3] as extended port,
            portNum[PRV_CPSS_PX_PORTS_NUM_CNS - 5, PRV_CPSS_PX_PORTS_NUM_CNS - 4] as internal port and
            portNum[PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as cascade port.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 1;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 3;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);
#if 0
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 5;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 4;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 3;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);
#endif
        /*  1.2  Call with out of range trunkId == [0, PRV_CPSS_PX_TRUNKS_NUM_CNS] and with same port type lists:
            portsList == [0, 1], [PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
            Expected: GT_BAD_PARAM. */
        trunkId = 0; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS; portListPtr = twoCscdPorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.3  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and with same type portLists:
            portsList == [0, 1], [PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
            Expected: GT_BAD_PARAM. (because trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] was not created). */
        trunkId = 1; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = twoCscdPorts; portListLen = 2; expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.4  Create trunk with trunkId == [1, 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and
            trunkType == [CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E, CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E, CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E, CPSS_PX_HAL_BPE_TRUNK_CASCADE_E]
            Expected: GT_OK. */
        trunkId = 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);
#if 0
        trunkId = 2; trunkType = CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2; trunkType = CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; trunkType = CPSS_PX_HAL_BPE_TRUNK_CASCADE_E; expect = GT_OK;
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, trunkType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, trunkId, trunkType);
#endif
        /*  1.5  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and
            out of range portsList == [PRV_CPSS_PX_PORTS_NUM_CNS, PRV_CPSS_PX_PORTS_NUM_CNS + 1]
            Expected: GT_BAD_PARAM. */
        trunkId = 1; portListPtr = outOfRangePorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = outOfRangePorts; portListLen = 2; expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif

        /*  1.6 Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and NULL portList with length 1.
            Expected: GT_BAD_PTR. */
        trunkId = 1; portListPtr = NULL; portListLen = 1; expect = GT_BAD_PTR;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = NULL; portListLen = 1; expect = GT_BAD_PTR;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        /*  1.7  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and with wrong type portLists:
            portsList == [2, 3], [PRV_CPSS_PX_PORTS_NUM_CNS - 5, PRV_CPSS_PX_PORTS_NUM_CNS - 4]
            Expected: GT_BAD_PARAM. */
        trunkId = 1; portListPtr = twoExtdPorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = twoIntrPorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.8  Call with valid trunkId == [1, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and with mixed type portLists (with one port of the correct type):
            portList == [0, 2], [1, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
            Expected: GT_BAD_PARAM. */
        trunkId = 1; portListPtr = oneUpstOneExtdPorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = oneUpstOneCscdPorts; portListLen = 2; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.9   Call with
            valid trunkId[1] and valid portList == [0, 1].
            valid trunkId[2] and valid portList == [2, 3].
            valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 2] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 5, PRV_CPSS_PX_PORTS_NUM_CNS - 4].
            valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 3, PRV_CPSS_PX_PORTS_NUM_CNS - 2].
            Expected: GT_OK. */
        trunkId = 1; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = 2; portListPtr = twoExtdPorts; portListLen = 2; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2; portListPtr = twoIntrPorts; portListLen = 2; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = twoCscdPorts; portListLen = 2; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.10 Set portNum[4] as extended port. Set numOfUnicastChannels == 2 and numOfMulticastChannels == 0.
            Set pcid == 1 for portNum[4]. Set trunkId == 1 as upstream of portNum[4]. Create unicast E-channel with cid == 1 and
            upstream interface == trunkId[1]
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 4;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        numOfUnicastChannels = 2; numOfMulticastChannels = 0; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 4;
        pcid = 1; expect = GT_OK;
        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &interface, pcid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(expect, st, dev, interface.devPort.devNum, interface.devPort.portNum, pcid);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E; interface.trunkId = 1;
        portNum = 4; expect = GT_OK;
        st = cpssPxHalBpePortExtendedUpstreamSet(dev, portNum, &interface);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, portNum, interface.trunkId);        

        /*interface.type = CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E; interface.trunkId = 1;
        cid = 1; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid, &interface);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, cid, interface.type, interface.devPort.devNum, interface.devPort.portNum);*/

        /*  1.11 Call with valid trunkId[1] and valid portList == [0, 1].
            Expected: GT_OK. */
        trunkId = 1; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        /*  1.12 Remove trunkId[1, 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] using cpssPxHalBpeTrunkRemove API
            Expected: GT_BAD_STATE. (because ports were not deleted from trunks). */
        trunkId = 1, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = 2, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.13 Call cpssPxHalBpeTrunkPortsDelete API with trunkId[1, 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, PRV_CPSS_PX_TRUNKS_NUM_CNS - 1]
            and empty portLists.
            Expected: GT_OK. */
        trunkId = 1; portListPtr = outOfRangePorts; portListLen = 0; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = 2; portListPtr = outOfRangePorts; portListLen = 0; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2; portListPtr = outOfRangePorts; portListLen = 0; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = outOfRangePorts; portListLen = 0; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.14 Call cpssPxHalBpeTrunkPortsDelete API with
            valid trunkId[1] and valid portList == [0].
            valid trunkId[2] and valid portList == [2].
            valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 2] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 5]
            valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 3]
            Expected: GT_OK. */
        trunkId = 1; portListPtr = oneOutOfTwoUpstPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = 2; portListPtr = oneOutOfTwoExtdPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2; portListPtr = oneOutOfTwoIntrPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = oneOutOfTwoCscdPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.15 Make same call as in 1.10
            Expected: GT_BAD_STATE. (because not ALL ports were not deleted from trunks). */
        trunkId = 1, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = 2, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.16 Call cpssPxHalBpeTrunkPortsDelete API with
            valid trunkId[1] and valid portList == [1].
            valid trunkId[2] and valid portList == [3].
            valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 2] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 4]
            valid trunkId[PRV_CPSS_PX_TRUNKS_NUM_CNS - 1] and valid portList == [PRV_CPSS_PX_PORTS_NUM_CNS - 2]
            Expected: GT_OK. */
        trunkId = 1; portListPtr = twoOutOfTwoUpstPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = 2; portListPtr = twoOutOfTwoExtdPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2; portListPtr = twoOutOfTwoIntrPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1; portListPtr = twoOutOfTwoCscdPorts; portListLen = 1; expect = GT_OK;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.17 Make same call as in 1.12
            Expected: GT_BAD_STATE (because unicase E-channel was not deleted). */
        /*trunkId = 1, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);*/
#if 0
        trunkId = 2, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1, expect = GT_BAD_STATE;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
        /*  1.18 Delete unicast E-channel with cid == 0.
            Expected: GT_OK. */
     /*   cid = 1; expect = GT_OK;
        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);*/

        /*  1.19 Make same call as in 1.12
            Expected: GT_OK. */
        trunkId = 1, expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#if 0
        trunkId = 2, expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 2, expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = PRV_CPSS_PX_TRUNKS_NUM_CNS - 1, expect = GT_OK;
        st = cpssPxHalBpeTrunkRemove(dev, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
#endif
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        trunkId = 1; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

        trunkId = 1; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    trunkId = 1; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, portListPtr, portListLen);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);

    trunkId = 1; portListPtr = twoUpstPorts; portListLen = 2; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, portListPtr, portListLen);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, trunkId);
}
/*
GT_STATUS cpssPxHalBpeMulticastEChannelCreate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfaceListPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeMulticastEChannelCreate)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Set numOfUnicastChannels == 0, numOfMulticastChannels == 8K - 19.
    Expected: GT_OK.
    1.2  Call with valid cid == 50 | BIT_12 and NULL interfaceListPtr
    Expected: GT_BAD_PTR.
    1.3  Call with valid cid == 50 | BIT_12 and valid interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
    Expected: GT_BAD_PARAM. (because ports were not set as extended).
    1.4  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended ports.
    Expected: GT_OK.
    1.5  Call with valid cid == 50 | BIT_12 and out of range interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS]
    Expected: GT_BAD_PARAM.
    1.6  Call with out of range cid == 200 | BIT_12 | BIT_13 and valid interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
    Expected: GT_OUT_OF_RANGE.
    1.7  Call with valid cid == 50 | BIT_12 and mixed type interfaceListPtr == [0, 1]
    Expected: GT_BAD_PARAM. (because port 1 was not set as extended).
    1.8 Set numOfUnicastChannels == 100, numOfMulticastChannels == 0.
    Expected: GT_OK.
    1.9  Call with valid cid == [1 | BIT_12, 1 | BIT_13] and extended interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
    Expected: GT_OK.
    1.10 Set numOfUnicastChannels == 0, numOfMulticastChannels == 8K - 19.
    Expected: GT_OK.
    1.11 Make same call as in 1.9.
    Expected: GT_OK.
    1.12 Make same call as in 1.9.
    Expected: GT_OK.
    1.13 Set numOfUnicastChannels == 100, numOfMulticastChannels == 0.
    Expected: GT_BAD_PARAM. (because MC E-channel was not deleted).
    1.14 Call cpssPxHalBpeMulticastEChannelUpdate with cid == [1 | BIT_12, 1 | BIT_13] and NULL interfaceListPtr.
    Expected: GT_BAD_PTR.
    1.15 Call cpssPxHalBpeMulticastEChannelUpdate with cid == [1 | BIT_12, 1 | BIT_13] and empty interfaceListPtr.
    Expected: GT_OK.
    1.16 Call cpssPxHalBpeMulticastEChannelUpdate with cid == [1 | BIT_12, 1 | BIT_13] and
    interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
    Expected: GT_OK.
    1.17 Remove MC channel with cid == [1 | BIT_12, 1 | BIT_13] using cpssPxHalBpeMulticastEChannelDelete API.
    Expected: GT_OK.
    1.18 Make same call as in 1.17
    Expect: GT_OK.
    1.19 Make same call as in 1.16
    Expect: GT_OK.
*/
    GT_STATUS                          st, expect;
    GT_U8                              dev;
    GT_U32                             cid, numOfUnicastChannels, numOfMulticastChannels;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC *interfaceListPtr, interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT mode;

    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC validInterfaceList[3] = {
        {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 0}},
        {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS - 2}},
        {CPSS_PX_HAL_BPE_INTERFACE_NONE_E, {0}}
    };
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC outOfRangeInterfaceList[3] = {
        {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 0}},
        {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, PRV_CPSS_PX_PORTS_NUM_CNS}},
        {CPSS_PX_HAL_BPE_INTERFACE_NONE_E, {0}}
    };
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC mixedTypeInterfaceList[3] = {
        {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 0}},
        {CPSS_PX_HAL_BPE_INTERFACE_PORT_E, {0, 1}},
        {CPSS_PX_HAL_BPE_INTERFACE_NONE_E, {0}}
    };
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC emptyInterfaceList[1] = {
        {CPSS_PX_HAL_BPE_INTERFACE_NONE_E, {0}}
    };

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        validInterfaceList[0].devPort.devNum = validInterfaceList[1].devPort.devNum = dev;
        outOfRangeInterfaceList[0].devPort.devNum = outOfRangeInterfaceList[1].devPort.devNum = dev;
        mixedTypeInterfaceList[0].devPort.devNum = mixedTypeInterfaceList[1].devPort.devNum = dev;

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1  Set numOfUnicastChannels == 0, numOfMulticastChannels == 8K - 19.
            Expected: GT_OK. */
        numOfUnicastChannels = 0; numOfMulticastChannels = _8K - 19; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.2  Call with valid cid == 50 | BIT_12 and NULL interfaceListPtr
            Expected: GT_BAD_PTR. */
        cid = 50 | BIT_12; interfaceListPtr = NULL; expect = GT_BAD_PTR;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.3  Call with valid cid == 50 | BIT_12 and valid interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
            Expected: GT_BAD_PARAM. (because ports were not set as extended). */
        cid = 50 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.4  Set portNum[0, PRV_CPSS_PX_PORTS_NUM_CNS - 2] as extended ports.
            Expected: GT_OK. */
        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = 0;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E; interface.devPort.devNum = dev; interface.devPort.portNum = PRV_CPSS_PX_PORTS_NUM_CNS - 2;
        mode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E; expect = GT_OK;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &interface, mode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(expect, st, dev, interface.type, interface.devPort.devNum, interface.devPort.portNum, mode);

        /*  1.5  Call with valid cid == 50 | BIT_12 and out of range interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS]
            Expected: GT_BAD_PARAM. */
        cid = 50 | BIT_12; interfaceListPtr = outOfRangeInterfaceList; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.6  Call with out of range cid == 200 | BIT_12 | BIT_13 and valid interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
            Expected: GT_OUT_OF_RANGE. */

        cid = 200 | BIT_12 | BIT_13; interfaceListPtr = validInterfaceList; expect = GT_OUT_OF_RANGE;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.7  Call with valid cid == 50 | BIT_12 and mixed type interfaceListPtr == [0, 1]
            Expected: GT_BAD_PARAM. (because port 1 was not set as extended). */
        cid = 50 | BIT_12; interfaceListPtr = mixedTypeInterfaceList; expect = GT_BAD_PARAM;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.8 Set numOfUnicastChannels == 100, numOfMulticastChannels == 0.
            Expected: GT_OK. */
        numOfUnicastChannels = 100; numOfMulticastChannels = _4K; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.9  Call with valid cid == [99 | BIT_12, 99] and extended interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
            Expected: GT_OK. */
        cid = 99 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);        

        /*  1.10 Set numOfUnicastChannels == 0, numOfMulticastChannels == 8K - 19.
            Expected: GT_OK. */
        numOfUnicastChannels = 0; numOfMulticastChannels = _8K - 19; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.11 Make same call as in 1.9.
            Expected: GT_OK. */
        cid = 1 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);        

        /*  1.12 Make same call as in 1.9.
            Expected: GT_OK. */
        cid = 1 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.13 Set numOfUnicastChannels == 100, numOfMulticastChannels == 0.
            Expected: GT_OK. */
        numOfUnicastChannels = 100; numOfMulticastChannels = _4K; expect = GT_OK;
        st = cpssPxHalBpeNumOfChannelsSet(dev, numOfUnicastChannels, numOfMulticastChannels);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expect, st, dev, numOfUnicastChannels, numOfMulticastChannels);

        /*  1.14 Call cpssPxHalBpeMulticastEChannelUpdate with cid == [101 | BIT_12, 101 | BIT_13] and NULL interfaceListPtr.
            Expected: GT_BAD_PTR. */
        cid = 101 | BIT_12; interfaceListPtr = NULL; expect = GT_BAD_PTR;
        st = cpssPxHalBpeMulticastEChannelUpdate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        cid = 101 | BIT_13; interfaceListPtr = NULL; expect = GT_BAD_PTR;
        st = cpssPxHalBpeMulticastEChannelUpdate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        /*  1.15 Call cpssPxHalBpeMulticastEChannelUpdate with cid == [99 | BIT_12] and empty interfaceListPtr.
            Expected: GT_OK. */
        cid = 99 | BIT_12; interfaceListPtr = emptyInterfaceList; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelUpdate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);       

        /*  1.16 Call cpssPxHalBpeMulticastEChannelUpdate with cid == [99 | BIT_12] and
            interfaceListPtr == [0, PRV_CPSS_PX_PORTS_NUM_CNS - 2]
            Expected: GT_OK. */
        cid = 99 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelUpdate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);        

        /*  1.17 Remove MC channel with cid == [99 | BIT_12] using cpssPxHalBpeMulticastEChannelDelete API.
            Expected: GT_OK. */
        cid = 99 | BIT_12; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);        

        /*  1.18 Make same call as in 1.17
            Expect: GT_OK. */
        cid = 99 | BIT_12; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);       

        /*  1.19 Make same call as in 1.16
            Expect: GT_OK. */
        cid = 99 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_OK;
        st = cpssPxHalBpeMulticastEChannelUpdate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);        
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        cid = 1 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        cid = 1 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeMulticastEChannelUpdate(dev, cid, interfaceListPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

        cid = 1 | BIT_12; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    cid = 1 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeMulticastEChannelCreate(dev, cid, interfaceListPtr);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

    cid = 1 | BIT_12; interfaceListPtr = validInterfaceList; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeMulticastEChannelUpdate(dev, cid, interfaceListPtr);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);

    cid = 1 | BIT_12; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeMulticastEChannelDelete(dev, cid);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, cid);
}
/*
GT_STATUS cpssPxHalBpeDataControlQosRuleAdd
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      ruleIndex,
    IN  CPSS_PX_HAL_BPE_QOSMAP_DATA_STC            *qosMapPtr,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_TYPE_ENT   encapsulationType,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC   *maskPtr,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC   *patternPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxHalBpeDataControlQosRuleAdd)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1  Call with out of range ruleIndex == CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.2  Call with out of range encapsulationType == 0xFFFFFFFF and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.3  Call with qosMapPtr == NULL and rest of parameters being valid.
    Expected: GT_BAD_PTR.
    1.4  Call with qosMapPtr->newPcp == CPSS_PCP_RANGE_CNS and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.5  Call with qosMapPtr->newDei == CPSS_DEI_RANGE_CNS and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.6  Call with qosMapPtr->queue == BIT_3 and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.7  Call with maskPtr == NULL and rest of parameters being valid.
    Expected: GT_BAD_PTR.
    1.8  Call with maskPtr->dscp == BIT_6, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.9  Call with maskPtr->ipProtocol == BIT_8, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.10 Call with patternPtr == NULL and rest of parameters being valid.
    Expected: GT_BAD_PTR.
    1.11 Call with patternPtr->dscp == BIT_6, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.12 Call with patternPtr->ipProtocol == BIT_8, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E and rest of parameters being valid.
    Expected: GT_BAD_PARAM.
    1.13 Call with ruleIndex == 0, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E and patternPtr->macDa == 01:02:03:04:05:06.
    Expected: GT_OK.
    1.14 Make same call as in 1.13.
    Expected: GT_OK.
    1.15 Call with ruleIndex == 1, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E
    and patternPtr->ipProtocol == 0x11, patternPtr->dscp == 0x2A.
    Expected: GT_OK.
    1.16 Call with ruleIndex == 2, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E
    and patternPtr->ipProtocol == 0x11, patternPtr->dscp == 0x2A.
    Expected: GT_OK.
    1.17 Call with ruleIndex == CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 2, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ETHERNET_OR_LLC_SNAP_E
    and patternPtr->etherType == 0xAAAA.
    Expected: GT_OK.
    1.18 Call with ruleIndex == CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 1, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_LLC_NOT_SNAP_E
    and patternPtr->etherType == 0x4242.
    Expected: GT_OK.
    1.19 Delete ruleIndex == [0, 1, 2, CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 2, CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 1] using cpssPxHalBpeDataControlQosRuleDelete API.
    Expected: GT_OK.
*/
    GT_STATUS                                st, expect;
    GT_U8                                    dev;
    GT_U32                                   ruleIndex;
    CPSS_PX_HAL_BPE_QOSMAP_DATA_STC          qosMap, *qosMapPtr;
    CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_TYPE_ENT encapsulationType;
    CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC mask, *maskPtr;
    CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC pattern, *patternPtr;
    GT_ETHERADDR                             macDa = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}};

    cpssOsMemSet(&mask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
    cpssOsMemSet(&pattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
    cpssOsMemSet(&qosMap, 0x00, sizeof(CPSS_PX_HAL_BPE_QOSMAP_DATA_STC));

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1  Call with out of range ruleIndex == CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        /* 1.2  Call with out of range encapsulationType == 0xFFFFFFFF and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = 0xFFFFFFFF;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        /*  1.3  Call with qosMapPtr == NULL and rest of parameters being valid.
            Expected: GT_BAD_PTR. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = NULL; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PTR;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        /* 1.4  Call with qosMapPtr->newPcp == CPSS_PCP_RANGE_CNS and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;
        qosMapPtr->newPcp = CPSS_PCP_RANGE_CNS;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        qosMapPtr->newPcp = 0;

        /* 1.5  Call with qosMapPtr->newDei == CPSS_DEI_RANGE_CNS and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;
        qosMapPtr->newDei = CPSS_DEI_RANGE_CNS;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        qosMapPtr->newDei = 0;

        /* 1.6  Call with qosMapPtr->queue == BIT_3 and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;
        qosMapPtr->queue = BIT_3;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        qosMapPtr->queue = 0;

        /* 1.7  Call with maskPtr == NULL and rest of parameters being valid.
           Expected: GT_BAD_PTR. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = &qosMap; maskPtr = NULL; patternPtr = &pattern; expect = GT_BAD_PTR;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        /* 1.8  Call with maskPtr->dscp == BIT_6, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;
        maskPtr->dscp = BIT_6;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        maskPtr->dscp = 0;

        /* 1.9  Call with maskPtr->ipProtocol == BIT_8, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;
        maskPtr->ipProtocol = BIT_8;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        maskPtr->ipProtocol = 0;

        /* 1.10 Call with patternPtr == NULL and rest of parameters being valid.
           Expected: GT_BAD_PTR. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = NULL; expect = GT_BAD_PTR;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        /* 1.11 Call with patternPtr->dscp == BIT_6, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;
        patternPtr->dscp = BIT_6;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        patternPtr->dscp = 0;

        /* 1.12 Call with patternPtr->ipProtocol == BIT_8, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E and rest of parameters being valid.
           Expected: GT_BAD_PARAM. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_BAD_PARAM;
        patternPtr->ipProtocol = BIT_8;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        patternPtr->ipProtocol = 0;

        /* 1.13 Call with ruleIndex == 0, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E and patternPtr->macDa == 01:02:03:04:05:06.
           Expected: GT_OK. */
        ruleIndex =  0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_OK;
        patternPtr->macDa = macDa;
        cpssOsMemSet(&(maskPtr->macDa), 0xFF, sizeof(GT_ETHERADDR));

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        /* 1.14 Make same call as in 1.13.
           Expected: GT_OK. */

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        cpssOsMemSet(&(maskPtr->macDa), 0x00, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(patternPtr->macDa), 0x00, sizeof(GT_ETHERADDR));

        /* 1.15 Call with ruleIndex == 1, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E
           and patternPtr->ipProtocol == 0x11, patternPtr->dscp == 0x2A.
           Expected: GT_OK. */
        ruleIndex =  1; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_OK;
        patternPtr->ipProtocol = 0x11; patternPtr->dscp = 0x2A;
        maskPtr->ipProtocol = 0xFF; maskPtr->dscp = 0x3F;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        patternPtr->ipProtocol = 0; patternPtr->dscp = 0;
        maskPtr->ipProtocol = 0; maskPtr->dscp = 0;

        /* 1.16 Call with ruleIndex == 2, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E
           and patternPtr->ipProtocol == 0x11, patternPtr->dscp == 0x2A.
           Expected: GT_OK. */
        ruleIndex =  2; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_OK;
        patternPtr->ipProtocol = 0x11; patternPtr->dscp = 0x2A;
        maskPtr->ipProtocol = 0xFF; maskPtr->dscp = 0x3F;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        patternPtr->ipProtocol = 0; patternPtr->dscp = 0;
        maskPtr->ipProtocol = 0; maskPtr->dscp = 0;

        /* 1.17 Call with ruleIndex == CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 2, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ETHERNET_OR_LLC_SNAP_E
           and patternPtr->etherType == 0xAAAA.
           Expected: GT_OK. */
        ruleIndex =  CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 2; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ETHERNET_OR_LLC_SNAP_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_OK;
        patternPtr->etherType = 0xAAAA; maskPtr->etherType = 0xFFFF;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        patternPtr->etherType = 0; maskPtr->etherType = 0;

        /* 1.18 Call with ruleIndex == CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 1, encapsulationType == CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_LLC_NOT_SNAP_E
           and patternPtr->etherType == 0x4242.
           Expected: GT_OK. */
        ruleIndex =  CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 1; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_LLC_NOT_SNAP_E;
        qosMapPtr = &qosMap; maskPtr = &mask; patternPtr = &pattern; expect = GT_OK;
        patternPtr->etherType = 0x4242; maskPtr->etherType = 0xFFFF;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        patternPtr->etherType = 0; maskPtr->etherType = 0;

        /* 1.19 Delete ruleIndex == [0, 1, 2, CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 2, CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 1] using cpssPxHalBpeDataControlQosRuleDelete API.
           Expected: GT_OK. */
        ruleIndex = 0; expect = GT_OK;
        st = cpssPxHalBpeDataControlQosRuleDelete(dev, ruleIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        ruleIndex = 1; expect = GT_OK;
        st = cpssPxHalBpeDataControlQosRuleDelete(dev, ruleIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        ruleIndex = 2; expect = GT_OK;
        st = cpssPxHalBpeDataControlQosRuleDelete(dev, ruleIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        ruleIndex = CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 2; expect = GT_OK;
        st = cpssPxHalBpeDataControlQosRuleDelete(dev, ruleIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        ruleIndex = CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS - 1; expect = GT_OK;
        st = cpssPxHalBpeDataControlQosRuleDelete(dev, ruleIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        ruleIndex = 0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

        ruleIndex = 0; expect = GT_NOT_APPLICABLE_DEVICE;
        st = cpssPxHalBpeDataControlQosRuleDelete(dev, ruleIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    ruleIndex = 0; encapsulationType = CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeDataControlQosRuleAdd(dev, ruleIndex, qosMapPtr, encapsulationType, maskPtr, patternPtr);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);

    ruleIndex = 0; expect = GT_BAD_PARAM;
    st = cpssPxHalBpeDataControlQosRuleDelete(dev, ruleIndex);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expect, st, dev, ruleIndex);
}
/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxHalBpe suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxHalBpe)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeNumOfChannelsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeNumOfChannelsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeDebugCncBlocksSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeCncConfigClient)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeInterfaceTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeUpstreamQosMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeExtendedQosMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeInterfaceExtendedPcidSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpePortExtendedUpstreamSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpePortExtendedQosDefaultPcpDeiSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpePortExtendedQosMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeUnicastEChannelCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpePortExtendedUntaggedVlanAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpePortExtendedUntaggedVlanDel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeTrunkCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeTrunkLoadBalanceModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeTrunkPortsAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeMulticastEChannelCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHalBpeDataControlQosRuleAdd)
UTF_SUIT_END_TESTS_MAC(cpssPxHalBpe)
