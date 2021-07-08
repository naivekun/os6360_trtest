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
* @file cpssDxChCfgInitUT.c
*
* @brief Unit Tests for CPSS DxCh initialization of PPs
* and shadow data structures, and declarations of global variables.
*
* @version   51
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <trafficEngine/tgfTrafficEngine.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>



static GT_STATUS prvDxChCfgInitAccessPortGroupTblEntryReadCb
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  convertedPortGroupId,
    IN  CPSS_DXCH_TABLE_ENT tableType,
    IN  GT_U32                  convertedEntryIndex,
    OUT GT_U32                 *entryValuePtr,
    IN  GT_U32                  inPortGroupId,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN  GT_STATUS                       status
)
{
    /* to avoid compilation warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(convertedPortGroupId);
    TGF_PARAM_NOT_USED(tableType);
    TGF_PARAM_NOT_USED(convertedEntryIndex);
    TGF_PARAM_NOT_USED(entryValuePtr);
    TGF_PARAM_NOT_USED(inPortGroupId);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(status);

        return GT_OK;
}

static GT_STATUS prvDxChCfgInitAccessPortGroupTblEntryWriteCb
(
    IN GT_U8                   devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  convertedEntryIndex,
    IN GT_U32                 *entryValuePtr,
    IN GT_U32                 *entryMaskPtr,
    IN GT_U32                 inPortGroupId,
    IN CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN  GT_STATUS                       status
)
{
    /* to avoid compilation warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(convertedPortGroupId);
    TGF_PARAM_NOT_USED(tableType);
    TGF_PARAM_NOT_USED(convertedEntryIndex);
    TGF_PARAM_NOT_USED(entryValuePtr);
    TGF_PARAM_NOT_USED(entryMaskPtr);
    TGF_PARAM_NOT_USED(inPortGroupId);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(status);

        return GT_OK;
}

static GT_STATUS prvDxChCfgInitAccessPortGroupTblEntryFieldReadCb
(
    IN GT_U8                   devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  convertedEntryIndex,
    IN GT_U32                  fieldWordNum,
    IN GT_U32                  fieldOffset,
    IN GT_U32                  fieldLength,
    OUT GT_U32                *fieldValuePtr,
    IN GT_U32                  inPortGroupId,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN  GT_STATUS                       status
)
{
    /* to avoid compilation warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(convertedPortGroupId);
    TGF_PARAM_NOT_USED(tableType);
    TGF_PARAM_NOT_USED(convertedEntryIndex);
    TGF_PARAM_NOT_USED(fieldWordNum);
    TGF_PARAM_NOT_USED(fieldOffset);
    TGF_PARAM_NOT_USED(fieldLength);
    TGF_PARAM_NOT_USED(fieldValuePtr);
    TGF_PARAM_NOT_USED(inPortGroupId);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(status);

        return GT_OK;
}

static GT_STATUS prvDxChCfgInitAccessPortGroupTblEntryFieldWriteCb
(
    IN GT_U8                   devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  convertedEntryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue,
    IN GT_U32                 inPortGroupId,
    IN CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN GT_STATUS                       status
)
{
    /* to avoid compilation warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(convertedPortGroupId);
    TGF_PARAM_NOT_USED(tableType);
    TGF_PARAM_NOT_USED(convertedEntryIndex);
    TGF_PARAM_NOT_USED(fieldWordNum);
    TGF_PARAM_NOT_USED(fieldOffset);
    TGF_PARAM_NOT_USED(fieldLength);
    TGF_PARAM_NOT_USED(fieldValue);
    TGF_PARAM_NOT_USED(inPortGroupId);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(status);

        return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgReNumberDevNum
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgReNumberDevNum)
{
/*
    ITERATE_DEVICES
    1.1. Call function with newDevNum [0 / 16 / 31].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8   newDevNum = 0;
    GT_U8   oldDevNum = 0;
    GT_U8   devNumbersToTest[3] = {0,16,31};
    GT_U32  numOfDevices=0;

    GT_U32  ii;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_LION2_E, "JIRA-6762");

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* check which devNums are currently used */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevices++;

        /* need to make sure not to use any number that used by existing device */
        for(ii = 0 ; ii < 3 ; ii++)
        {
            if(dev == devNumbersToTest[ii])
            {
                devNumbersToTest[ii]++;
                break;
            }
        }
    }

    if(numOfDevices == 1)
    {
        devNumbersToTest[0] = 0;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with newDevNum [0 / 16 / 31].
            Expected: GT_OK.
        */

        /* call with newDevNum = 0 */
        oldDevNum = dev;
        newDevNum = devNumbersToTest[0];


        st = cpssDxChCfgReNumberDevNum(oldDevNum, newDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, oldDevNum, newDevNum);


        st = cpssDxChCfgReNumberDevNum(newDevNum, oldDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, newDevNum, oldDevNum);

        /* call with newDevNum = 16 */
        oldDevNum = dev;
        newDevNum = devNumbersToTest[1];


        st = cpssDxChCfgReNumberDevNum(oldDevNum, newDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, oldDevNum, newDevNum);


        st = cpssDxChCfgReNumberDevNum(newDevNum, oldDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, newDevNum, oldDevNum);

        /* call with newDevNum = 31 */
        oldDevNum = dev;
        newDevNum = devNumbersToTest[2];


        st = cpssDxChCfgReNumberDevNum(oldDevNum, newDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, newDevNum);


        st = cpssDxChCfgReNumberDevNum(newDevNum, oldDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, newDevNum, oldDevNum);
    }

    newDevNum = 0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */

        st = cpssDxChCfgReNumberDevNum(dev, newDevNum);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;


    st = cpssDxChCfgReNumberDevNum(dev, newDevNum);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgDevEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgDevEnable)
{
    /*
    ITERATE_DEVICES
    1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for DxCh devices and GT_BAD_PARAM for others.
    1.2. Call cpssDxChCfgDevEnableGet.
    Expected: GT_OK and same enablePtr.
    */
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_BOOL                     enable;
    GT_BOOL                     enableGet;

    /* there is no DFX in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with enable [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK for DxCh devices and GT_BAD_PARAM for others.    */
        enable = GT_FALSE;

        st = cpssDxChCfgDevEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. */
        st = cpssDxChCfgDevEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d, %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChCfgDevEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. */
        st = cpssDxChCfgDevEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d, %d", dev);
    }

    enable = GT_TRUE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChCfgDevEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDevEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgDevEnableGet)
{
    /*
    ITERATE_DEVICES
    1.1. Call function with not NULL enablePtr
    Expected: GT_OK
    1.2. Call function with NULL enablePtr
    Expected: GT_BAD_PTR
    */
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_BOOL                     enable = GT_FALSE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        st = cpssDxChCfgDevEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. */
        st = cpssDxChCfgDevEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChCfgDevEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDevEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     modifyEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet)
{
/*
ITERATE_DEVICES_PHY_PORTS (DxChx)
1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
Expected: GT_OK.
1.1.2. Call cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet.
Expected: GT_OK and same modifyEnable.
*/

    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port;   /* port for 'UT iterations' */
    GT_BOOL                     enable;
    GT_BOOL                     enableGet = GT_TRUE;

    /* 1. Go over all active devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port,GT_TRUE))
        {
            /* 1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK. */
            enable = GT_FALSE;

            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. */
            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", dev, port);

            /* 1.1.1. */
            enable = GT_TRUE;

            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2. */
            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_TRUE;
    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet)
{
/*
ITERATE_DEVICES_PHY_PORTS (DxChx)
1.1.1. Call function with not NULL modifyEnablePtr.
Expected: GT_OK.
1.1.2. Call function with NULL modifyEnablePtr.
Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port;
    GT_BOOL                     enable;

    /* 1. Go over all active devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port,GT_TRUE))
        {
            /* 1.1.1. */
            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. */
            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_TRUE;
    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgDsaTagSrcDevPortModifySet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     modifedDsaSrcDev
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgDsaTagSrcDevPortModifySet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with modifedDsaSrcDev [GT_FALSE and GT_TRUE]. Expected: GT_OK.
*/
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_BOOL                     enable;
    GT_BOOL                     enableGet = GT_FALSE;;

    /* 1. Go over all active devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with enable [GT_FALSE and GT_TRUE].   */
        /* Expected: GT_OK.                                         */
        enable = GT_FALSE;

        st = cpssDxChCfgDsaTagSrcDevPortModifySet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. */
        st = cpssDxChCfgDsaTagSrcDevPortModifyGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d, %d", dev);

        /* 1.1. */
        enable = GT_TRUE;

        st = cpssDxChCfgDsaTagSrcDevPortModifySet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. */
        st = cpssDxChCfgDsaTagSrcDevPortModifyGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d, %d", dev);
    }

    enable = GT_TRUE;

    /*2. Go over all non active/non applicable devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChCfgDsaTagSrcDevPortModifySet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDsaTagSrcDevPortModifySet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgDsaTagSrcDevPortModifyGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with not NULL enablePtr.
Expected: GT_OK.
1.2. Call function with NULL enablePtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_BOOL                     enable = GT_FALSE;

    /* 1. Go over all active devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.  */
        st = cpssDxChCfgDsaTagSrcDevPortModifyGet(dev, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2.  */
        st = cpssDxChCfgDsaTagSrcDevPortModifyGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*2. Go over all non active/non applicable devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChCfgDsaTagSrcDevPortModifyGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDsaTagSrcDevPortModifyGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgTableNumEntriesGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_CFG_TABLES_ENT    table,
    OUT GT_U32                      *numEntriesPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgTableNumEntriesGet)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with table [CPSS_DXCH_CFG_TABLE_VLAN_E /
                          CPSS_DXCH_CFG_TABLE_FDB_E /
                          CPSS_DXCH_CFG_TABLE_PCL_ACTION_E],
              not NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values table.
    Expected: GT_BAD_PARAM.
    1.3. Call with numEntriesPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_CFG_TABLES_ENT    table       = CPSS_DXCH_CFG_TABLE_VLAN_E;
    GT_U32                      numEntries  = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with table [CPSS_DXCH_CFG_TABLE_VLAN_E /
                                  CPSS_DXCH_CFG_TABLE_FDB_E /
                                  CPSS_DXCH_CFG_TABLE_PCL_ACTION_E],
                           not NULL numEntriesPtr.
            Expected: GT_OK.
        */
        /* iterate with table = CPSS_DXCH_CFG_TABLE_VLAN_E */
        table = CPSS_DXCH_CFG_TABLE_VLAN_E;

        st = cpssDxChCfgTableNumEntriesGet(dev, table, &numEntries);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, table);

        /* iterate with table = CPSS_DXCH_CFG_TABLE_FDB_E */
        table = CPSS_DXCH_CFG_TABLE_FDB_E;

        st = cpssDxChCfgTableNumEntriesGet(dev, table, &numEntries);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, table);

        /* iterate with table = CPSS_DXCH_CFG_TABLE_PCL_ACTION_E */
        table = CPSS_DXCH_CFG_TABLE_PCL_ACTION_E;

        st = cpssDxChCfgTableNumEntriesGet(dev, table, &numEntries);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, table);

        /*
            1.2. Call with wrong enum values table.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCfgTableNumEntriesGet
                            (dev, table, &numEntries),
                            table);

        /*
            1.3. Call with numEntriesPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgTableNumEntriesGet(dev, table, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, numEntriesPtr = NULL", dev, table);
    }

    table = CPSS_DXCH_CFG_TABLE_VLAN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev, table, &numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgTableNumEntriesGet(dev, table, &numEntries);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgDevRemove
(
    IN  GT_U8                       devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgDevRemove)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st  = GT_OK;
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

        /*st = cpssDxChCfgDevRemove(dev);*/
        /*UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);*/
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgDevRemove(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDevRemove(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgPpLogicalInit
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_PP_CONFIG_INIT_STC    *ppConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgPpLogicalInit)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with correct ppConfigPtr.
    Expected: GT_OK.
    1.2. Call with wrong ppConfigPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with wrong ppConfigPtr.lpmMemoryMode [wrong enum].
     Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_PP_CONFIG_INIT_STC ppConfigPtr;
    GT_U32      maxNumOfPbrEntries;
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT lpmMemoryMode;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct ppConfigPtr.
            Expected: GT_OK.
        */

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            ppConfigPtr.routingMode = CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E;
        }
        else
        {
            ppConfigPtr.routingMode = CPSS_DXCH_TCAM_ROUTER_BASED_E;
        }

        ppConfigPtr.maxNumOfPbrEntries = 10;
        ppConfigPtr.lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;

        /* save original maxNumOfPbrEntries and lpmMemoryMode*/
        maxNumOfPbrEntries = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(dev)->ip.maxNumOfPbrEntries;

        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E )
        {
            lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
        }
        else
        {
            lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
        }

        /* good value for sip6 , ignored by other devices */
        ppConfigPtr.sharedTableMode = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;

        st = cpssDxChCfgPpLogicalInit(dev, &ppConfigPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call with wrong ppConfigPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChCfgPpLogicalInit(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

          /*
                1.3. Call with wrong ppConfigPtr.lpmMemoryMode [wrong enum].
                Expected: GT_BAD_PARAM.
                */
        if((PRV_CPSS_SIP_5_20_CHECK_MAC(dev) == GT_TRUE)&&
           (PRV_CPSS_SIP_6_CHECK_MAC(dev) == GT_FALSE) &&
        (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(dev) != GT_FALSE))
        {/*This parameter is only applicable in Bobcat3*/
            UTF_ENUMS_CHECK_MAC(cpssDxChCfgPpLogicalInit
                                    (dev, &ppConfigPtr),
                                    ppConfigPtr.lpmMemoryMode);
        }

        /* restore maxNumOfPbrEntries and lpmMemoryMode*/
        ppConfigPtr.maxNumOfPbrEntries = maxNumOfPbrEntries;
        ppConfigPtr.lpmMemoryMode = lpmMemoryMode;
        st = cpssDxChCfgPpLogicalInit(dev, &ppConfigPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgPpLogicalInit(dev, &ppConfigPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgPpLogicalInit(dev, &ppConfigPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
    test cpssDxChCfgPpLogicalInit(...) , but with 'system reset' to allow each time deferent mode(s)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgPpLogicalInit_enhanced)
{
    typedef struct{
        CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT sharedTableMode;
    }TEST_CASE_STC;

    static TEST_CASE_STC  testCaseArr[] = {
        {CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E},
        {CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E},
        {CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E},

        /* restore the value */
        {CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E},
        {GT_NA}
    };
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      numDevs = 0;
    TEST_CASE_STC   *testCasePtr;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    if(prvTgfResetModeGet() == GT_FALSE)
    {
        /* register the test as 'skipped' (not as succeeded) */
        SKIP_TEST_MAC;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numDevs++;

        testCasePtr = &testCaseArr[0];

        while(testCasePtr->sharedTableMode != GT_NA)
        {
            appDemoDbEntryAdd("sharedTableMode",testCasePtr->sharedTableMode);

            /* run full system reset and initialization
                cpssDxChCfgPpLogicalInit(...) will use this value 'testCasePtr->sharedTableMode'
            */
            st = prvTgfResetAndInitSystem();
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            testCasePtr++;
        }

        /* single device ! */
        break;
    }

    if(numDevs == 0)
    {
        /* register the test as 'skipped' (not as succeeded) */
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_CFG_DEV_INFO_STC   *devInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgDevInfoGet)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with not NULL devInfoPtr.
    Expected: GT_OK.
    1.2. Call with devInfoPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL devInfoPtr.
            Expected: GT_OK.
        */
        st = cpssDxChCfgDevInfoGet(dev, &devInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with devInfoPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgDevInfoGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, devInfoPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgDevInfoGet(dev, &devInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDevInfoGet(dev, &devInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static GT_STATUS portToMac
(
    IN    GT_U8        devNum,
    IN    GT_PORT_NUM  portNum,
    OUT   GT_U32       *portMacNumPtr
)
{
    /* Macro contains return on error */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(
        devNum, portNum, (*portMacNumPtr));
    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgBindPortPhymacObject
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN CPSS_MACDRV_OBJ_STC *macDrvObjPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgBindPortPhymacObject)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call function with not NULL macDrvObj pointer.
    Expected: GT_OK.
    1.1.2. Call function with NULL macDrvObj pointer.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port;
    CPSS_MACDRV_OBJ_STC         macDrvObj = {0};
    GT_U32                      portMacNum;
    CPSS_MACDRV_OBJ_STC         *orig_macDrvObjPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port,GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev,port))
            {
                /* Skip Remote Ports ports */
                continue;
            }
            st = portToMac(dev, port, &portMacNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            orig_macDrvObjPtr = PRV_CPSS_PHY_MAC_OBJ(dev,portMacNum);
            /*
                1.1.1. Call function with not NULL macDrvObj pointer.
                Expected: GT_OK.
            */
            st = cpssDxChCfgBindPortPhymacObject(dev, port, &macDrvObj);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /* clean object pointer to avoid failures of other tests */
            st = cpssDxChCfgBindPortPhymacObject(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(orig_macDrvObjPtr)
            {
                /* restore object pointer */
                st = cpssDxChCfgBindPortPhymacObject(dev, port, orig_macDrvObjPtr);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCfgBindPortPhymacObject(dev, port, &macDrvObj);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCfgBindPortPhymacObject(dev, port, &macDrvObj);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCfgBindPortPhymacObject(dev, port, &macDrvObj);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgBindPortPhymacObject(dev, port, &macDrvObj);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgBindPortPhymacObject(dev, port, &macDrvObj);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/* Extern functions - from gtIMP.h */
extern GT_STATUS prvCpssDrvImpRun
(
    IN  GT_VOID         *dev,
    IN  GT_U16          addr
);
extern GT_STATUS prvCpssDrvImpStop
(
    IN  GT_VOID          *dev
);
extern GT_STATUS prvCpssDrvImpReset
(
    IN  GT_VOID          *dev
);
/* End extern functions */
/*
    Check correct firmware load to 88e1690 PHY, when applicable
*/
UTF_TEST_CASE_MAC(cpssDxChCfgPortRemotePhyMacBind_FwLoadTo88e1690)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. On ports connecting to a remote 88e1690 PHY:
        - Disable FW
        - Query CnFcTimers.
            Expected: GT_FAIL.
        - Enable FW
 */

#ifndef ASIC_SIMULATION
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       devNum;
    GT_PHYSICAL_PORT_NUM                        portNum;
    PRV_CPSS_DXCH_PORT_INFO_STC                *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;
    GT_PHYSICAL_PORT_NUM                        firstRemotePortNum;
    GT_VOID                                    *drvInfoPtr;
    GT_U32                                      timerGet;
#endif  /* ASIC_SIMULATION */

    /* there is no Firmware in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;
    /* there is no Firmware in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum,GT_TRUE))
        {
            remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[portNum];
            if(remotePhyMacInfoPtr == NULL)
            {
                /* Port is not connecting to remote PHY - skip */
                continue;
            }
            switch(remotePhyMacInfoPtr->connectedPhyMacInfo.phyMacType)
            {
                case CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E:
                    break;
                default:
                    /* Port is not connecting to remote 88E1690 - skip */
                    continue;
            }
            if ((remotePhyMacInfoPtr->connectedPhyMacInfo.numOfRemotePorts == 0) ||
                (remotePhyMacInfoPtr->connectedPhyMacInfo.remotePortInfoArr == NULL))
            {
                /* Remote port is not known - skip */
                continue;
            }

            firstRemotePortNum = remotePhyMacInfoPtr->connectedPhyMacInfo.remotePortInfoArr[0].remotePhysicalPortNum;
            drvInfoPtr = remotePhyMacInfoPtr->drvInfoPtr;
            /* Issue firmware query while firmware is running - expect OK */
            st = cpssDxChPortCnFcTimerGet(devNum, firstRemotePortNum, 0, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, firstRemotePortNum);

            /* Stop firmware */
            st = prvCpssDrvImpStop(drvInfoPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, firstRemotePortNum);

            /* Issue firmware query while firmware is stopped - expect NOK */
            st = cpssDxChPortCnFcTimerGet(devNum, firstRemotePortNum, 0, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FAIL, st, devNum, firstRemotePortNum);

            /* Restart firmware */
            st = prvCpssDrvImpReset(drvInfoPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, firstRemotePortNum);
            st = prvCpssDrvImpRun(drvInfoPtr, 0);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, firstRemotePortNum);

            /* Issue firmware query while firmware is running again - expect OK */
            st = cpssDxChPortCnFcTimerGet(devNum, firstRemotePortNum, 0, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, firstRemotePortNum);
        }

    }
#endif  /* ASIC_SIMULATION */

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgPortDefaultSourceEportNumberSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         ePort
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgPortDefaultSourceEportNumberSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with ePort [0 / max/2 / max].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCfgPortDefaultSourceEportNumberGet.
    Expected: GT_OK and get same value.
    1.1.3. Call with out of range of ePort.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                   st = GT_OK;

    GT_U8                       devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_PORT_NUM                 ePort;
    GT_PORT_NUM                 ePortGet;
    GT_PHYSICAL_PORT_NUM        utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. For all active devices go over all available physical ports.
        */
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call with ePort [0 / max/2 / max].
                Expected: GT_OK.
            */
            ePort = 0;

            st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, ePort);

            /*
                1.1.2. Call cpssDxChCfgPortDefaultSourceEportNumberGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePortGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCfgPortDefaultSourceEportNumberGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(ePort, ePortGet,
                "get another ePort than was set: %d, %d", devNum, portNum);

            /* Call with ePort [max/2]. */
            ePort = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum) / 2;

            st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, ePort);

            /*
                1.1.2. Call cpssDxChCfgPortDefaultSourceEportNumberGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePortGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCfgPortDefaultSourceEportNumberGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(ePort, ePortGet,
                "get another ePort than was set: %d, %d, %d", devNum, portNum);

            /* Call with ePort [max]. */
            ePort = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum) - 1;

            st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, ePort);

            /*
                1.1.2. Call cpssDxChCfgPortDefaultSourceEportNumberGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePortGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCfgPortDefaultSourceEportNumberGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(ePort, ePortGet,
                "get another ePort than was set: %d, %d, %d", devNum, portNum);

            /*
                1.1.3. Call function with out of range of ePort.
                Expected: NOT GT_OK.
            */
            ePort = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);

            st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, ePort);

            /* Restoring correct values. */
            ePort = 0;
        }

        portNum = 0;
        ePort = 0;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;
    ePort = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgPortDefaultSourceEportNumberGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *ePortPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgPortDefaultSourceEportNumberGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1 Call function with not NULL ePortPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL ePortPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_PORT_NUM                             ePort;
    GT_PHYSICAL_PORT_NUM                    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. For all active devices go over all available physical ports.
        */
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1 Call function with not NULL ePortPtr.
                Expected: GT_OK.
            */
            st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum,
                                                            &ePort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &ePort);

            /*
                1.1.2. Call function with NULL ePortPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum,
                                                            NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum, &ePort);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum,
                                                            &ePort);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgEgressHashSelectionModeSet
(
    IN GT_U8                                                            devNum
    IN CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT
                                                                 selectionMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgEgressHashSelectionModeSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with selectionMode [CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E /
                                  CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E /
                                  CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E].
    Expected: GT_OK.
    1.2. Call cpssDxChCfgEgressHashSelectionModeSet.
    Expected: GT_OK and the same enable.
    1.3. Call function with wrong selectionMode enum values.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS                                                   st = GT_OK;

    GT_U8                                                       devNum;
    CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT   selectionMode;
    CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT
                                                             selectionModeGet;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with selectionMode [CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E /
                                          CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E /
                                          CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E].
            Expected: GT_OK.
        */
        selectionMode = CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E;

        st = cpssDxChCfgEgressHashSelectionModeSet(devNum, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, selectionMode);

        /*
            1.2. Call cpssDxChCfgEgressHashSelectionModeSet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCfgEgressHashSelectionModeGet(devNum, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChCfgEgressHashSelectionModeGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
            "get another selectionMode than was set: %d", devNum);

        /* Call with selectionMode [CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E]. */
        selectionMode = CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E;

        st = cpssDxChCfgEgressHashSelectionModeSet(devNum, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, selectionMode);

        /*
            1.2. Call cpssDxChCfgEgressHashSelectionModeSet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCfgEgressHashSelectionModeGet(devNum, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChCfgEgressHashSelectionModeGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
            "get another selectionMode than was set: %d", devNum);

        /* Call with selectionMode [CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E]. */
        selectionMode = CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E;

        st = cpssDxChCfgEgressHashSelectionModeSet(devNum, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, selectionMode);

        /*
            1.2. Call cpssDxChCfgEgressHashSelectionModeSet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCfgEgressHashSelectionModeGet(devNum, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChCfgEgressHashSelectionModeGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
            "get another selectionMode than was set: %d", devNum);

        /*
            1.3. Call function with wrong selectionMode enum values.
            Expected: GT_OUT_OF_RANGE.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCfgEgressHashSelectionModeSet
                            (devNum, selectionModeGet),
                            selectionModeGet);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    selectionMode = CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCfgEgressHashSelectionModeSet(devNum, selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgEgressHashSelectionModeSet(devNum, selectionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgEgressHashSelectionModeGet
(
    IN  GT_U8                                                           devNum
    OUT CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT
                                                             *selectionModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgEgressHashSelectionModeGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call function with not NULL selectionModePtr.
    Expected: GT_OK.
    1.2. Call function with NULL selectionModePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                                   st = GT_OK;

    GT_U8                                                       devNum;
    CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT   selectionMode;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL selectionModePtr.
            Expected: GT_OK
        */
        st = cpssDxChCfgEgressHashSelectionModeGet(devNum, &selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &selectionMode);

        /*
            1.2. Call function with NULL selectionModePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgEgressHashSelectionModeGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "%d, selectionMode = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCfgEgressHashSelectionModeGet(devNum, &selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgEgressHashSelectionModeGet(devNum, &selectionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgIngressDropCntrSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      counter
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgIngressDropCntrSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with counter [0xFFFF]. Expected: GT_OK.
     Read the counter and get same value.
1.2. Call function with counter [0xFFFFFFFF]. Expected: GT_OK.
     Read the counter and get same value.
1.3. Call function with counter [0x0]. Expected: GT_OK.
     Read the counter and get same value.
*/
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      counter;
    GT_U32                      counterGet;

    /* 1. Go over all active devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with counter [0xFFFF].   */
        /* Expected: GT_OK.                                         */
        counter = 0xFFFF;

        st = cpssDxChCfgIngressDropCntrSet(dev, counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, counter);

        st = cpssDxChCfgIngressDropCntrGet(dev, &counterGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgIngressDropCntrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(counter, counterGet,
                                     "get another ingress drop counter value than was set: %d", dev);

        /* 1.2. Call function with counter [0xFFFFFFFF].   */
        /* Expected: GT_OK.                                         */
        counter = 0xFFFFFFFF;

        st = cpssDxChCfgIngressDropCntrSet(dev, counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, counter);

        st = cpssDxChCfgIngressDropCntrGet(dev, &counterGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgIngressDropCntrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(counter, counterGet,
                                     "get another ingress drop counter value than was set: %d", dev);

        /* 1.3. Call function with counter [0x0].   */
        /* Expected: GT_OK.                                         */
        counter = 0x0;

        st = cpssDxChCfgIngressDropCntrSet(dev, counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, counter);

        st = cpssDxChCfgIngressDropCntrGet(dev, &counterGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgIngressDropCntrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(counter, counterGet,
                                     "get another ingress drop counter value than was set: %d", dev);
    }

    counter = 0x0;

    /*2. Go over all non active/non applicable devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChCfgIngressDropCntrSet(dev, counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgIngressDropCntrSet(dev, counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgIngressDropCntrGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgIngressDropCntrGet)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with not NULL counterPtr.
    Expected: GT_OK.
    1.2. Call with counterPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      counter;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL devInfoPtr.
            Expected: GT_OK.
        */
        st = cpssDxChCfgIngressDropCntrGet(dev, &counter);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgIngressDropCntrGet: %d", dev);

        /*
            1.2. Call with devInfoPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgIngressDropCntrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, counterPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgIngressDropCntrGet(dev, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgIngressDropCntrGet(dev, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgGlobalEportSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChCfgGlobalEportSet)
{
/*
    ITERATE_DEVICES (Bobcat2; Caelum; Bobcat3)
*/
    GT_STATUS                       st = GT_OK;

    GT_U8                           devNum;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  ecmp;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  global;

    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  ecmpGet;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  globalGet;

    GT_U32    ePortMask;/* mask of ePort */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    ecmp.enable  = GT_TRUE;
    ecmp.pattern = 0xAA55AA55;
    ecmp.mask    = 0xFFFFFFFF;

    global.enable  = GT_TRUE;
    global.pattern = 0x55AA55AA;
    global.mask    = 0xFFFFFFFF;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        ecmp.enable  = GT_TRUE;
        ecmp.pattern = 0xAA55AA55;
        ecmp.mask    = 0xFFFFFFFF;

        global.enable  = GT_TRUE;
        global.pattern = 0x55AA55AA;
        global.mask    = 0xFFFFFFFF;

        ePortMask = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);

        ecmp.pattern   &= ePortMask;
        ecmp.mask      &= ePortMask;
        global.pattern &= ePortMask;
        global.mask    &= ePortMask;

        st = cpssDxChCfgGlobalEportSet(devNum, &global ,&ecmp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgGlobalEportSet: %d", devNum);

        if( GT_OK == st )
        {
            st = cpssDxChCfgGlobalEportGet(devNum, &globalGet, &ecmpGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChCfgGlobalEportGet: %d", devNum);

            if(0 != cpssOsMemCmp(&global,&globalGet,sizeof(globalGet)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_STATE,
                                             "cpssDxChCfgGlobalEportGet: %d", devNum);
            }

            if(0 != cpssOsMemCmp(&ecmp,&ecmpGet,sizeof(ecmpGet)))
            {
                GT_BOOL didError = GT_TRUE;
                if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    /* this API is 'legacy' support with limitation on the mask,
                        the returned mask is the minimal mask that is more/equal
                        than the pattern */
                    if(ecmp.pattern == ecmpGet.pattern &&
                       ecmpGet.mask == (ecmp.mask >> 1))/*lost single bit*/
                    {
                        /* not error ... just limitation of the 'legacy support' */
                        didError = GT_FALSE;
                    }
                }

                if(didError == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_STATE,
                                             "cpssDxChCfgGlobalEportGet: %d", devNum);
                }
            }
        }

        /* check when ecmp.enable  = GT_FALSE , global.enable  = GT_TRUE */
        ecmp.enable  = GT_FALSE;
        ecmp.pattern = 0xFFFFFFFF;
        ecmp.mask    = 0xFFFFFFFF;

        global.pattern &= ePortMask;
        global.mask    &= ePortMask;

        st = cpssDxChCfgGlobalEportSet(devNum, &global ,&ecmp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgGlobalEportSet: %d", devNum);

        if( GT_OK == st )
        {
            st = cpssDxChCfgGlobalEportGet(devNum, &globalGet, &ecmpGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChCfgGlobalEportGet: %d", devNum);

            if(0 != cpssOsMemCmp(&global,&globalGet,sizeof(globalGet)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_STATE,
                                             "cpssDxChCfgGlobalEportGet: %d", devNum);
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(ecmp.enable, ecmpGet.enable,
                                         "cpssDxChCfgGlobalEportGet: %d", devNum);
        }

        /* check when ecmp.enable  = GT_TRUE , global.enable  = GT_FALSE */
        ecmp.enable  = GT_TRUE;
        ecmp.pattern = 0xAA55AA55;
        ecmp.mask    = ecmp.pattern;

        global.enable  = GT_FALSE;
        global.pattern = 0xFFFFFFFF;
        global.mask    = 0xFFFFFFFF;

        ecmp.pattern   &= ePortMask;
        ecmp.mask      &= ePortMask;

        st = cpssDxChCfgGlobalEportSet(devNum, &global ,&ecmp);
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* The cpss expected to generate next error , because the ecmp.mask
               must be 'be with continues bits'.

                ERROR GT_BAD_PARAM in function: prvCpssDxChSip6GlobalEportMaskCheck,
                file cpssdxchcfginit.c, line[4504]. the MASK[0x00a55] must be with continues bits
            */
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgGlobalEportSet: %d", devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgGlobalEportSet: %d", devNum);
        }

        if( GT_OK == st )
        {
            st = cpssDxChCfgGlobalEportGet(devNum, &globalGet, &ecmpGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChCfgGlobalEportGet: %d", devNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(global.enable, globalGet.enable,
                                         "cpssDxChCfgGlobalEportGet: %d", devNum);

            if(0 != cpssOsMemCmp(&ecmp,&ecmpGet,sizeof(ecmpGet)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_STATE,
                                             "cpssDxChCfgGlobalEportGet: %d", devNum);
            }
        }

        /* check when ecmp.enable  = GT_FALSE , global.enable  = GT_FALSE */
        ecmp.enable  = GT_FALSE;
        ecmp.pattern = 0x12345678;
        ecmp.mask    = 0x87654321;

        global.enable  = GT_FALSE;
        global.pattern = 0x11223344;
        global.mask    = 0xaabbccdd;

        st = cpssDxChCfgGlobalEportSet(devNum, &global ,&ecmp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgGlobalEportSet: %d", devNum);
        st = cpssDxChCfgGlobalEportGet(devNum, &globalGet, &ecmpGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgGlobalEportGet: %d", devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(global.enable, globalGet.enable,
                                     "cpssDxChCfgGlobalEportGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmp.enable, ecmpGet.enable,
                                     "cpssDxChCfgGlobalEportGet: %d", devNum);



        /* check bad param on 'no mtach' cases */
        ecmp.enable  = GT_TRUE;
        ecmp.pattern = 0x12345678;
        ecmp.mask    = 0x87654321;

        global = ecmp;
        global.enable  = GT_FALSE;

        st = cpssDxChCfgGlobalEportSet(devNum, &global ,&ecmp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM , st ,
                                     "cpssDxChCfgGlobalEportSet: %d", devNum);

        ecmp.enable  = GT_FALSE;
        ecmp.pattern = 0x12345678;
        ecmp.mask    = 0x87654321;

        global = ecmp;
        global.enable  = GT_TRUE;

        st = cpssDxChCfgGlobalEportSet(devNum, &global ,&ecmp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM , st ,
                                     "cpssDxChCfgGlobalEportSet: %d", devNum);

    }
    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCfgGlobalEportSet(devNum, &global , &ecmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgGlobalEportSet(devNum, &global , &ecmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgGlobalEportGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChCfgGlobalEportGet)
{
/*
    ITERATE_DEVICE (SIP5)
    1.1. Call with not NULL ecmpPtr,
                   not NULL trunkPtr,
                   not NULL globalPtr.
    Expected: GT_OK.
    1.2. Call with ecmpPtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
    1.3. Call with trunkPtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with globalPtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;

    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  ecmp   ;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  global ;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL ecmpPtr,
                           not NULL trunkPtr,
                           not NULL globalPtr.
            Expected: GT_OK.
        */
        st = cpssDxChCfgGlobalEportGet(dev, &global,&ecmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with ecmpPtr [NULL],
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgGlobalEportGet(dev, &global , NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ecmpPtr = NULL", dev);

        /*
            1.4. Call with globalPtr [NULL],
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgGlobalEportGet(dev, NULL , &ecmp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, globalPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgGlobalEportGet(dev, &global , &ecmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgGlobalEportGet(dev, &global , &ecmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgIngressDropCntrModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT mode,
    IN  GT_PORT_NUM                                 portNum,
    IN  GT_U16                                      vlan
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgIngressDropCntrModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E,
                        CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E,
                        CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E],
                   portNum[0, max/2, max],
                   vlan[0, max/2, max].
    Expected: GT_OK.
    1.2. cpssDxChCfgIngressDropCntrModeGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range mode and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E],
        out of range portNum(is relevant) and other valid params.
    Expected: NON GT_OK.
    1.5. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E],
        out of range portNum(not relevant) and other valid params.
    Expected: GT_OK.
    1.6. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E],
        out of range vlan(is relevant) and other valid params.
    Expected: NON GT_OK.
    1.7. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E],
        out of range vlan(not relevant) and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                                   st          = GT_OK;
    GT_U8                                       dev         = 0;
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT mode        = 0;
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT modeRet     = 0;
    GT_PORT_NUM                                 portNum     = 0;
    GT_PORT_NUM                                 portNumRet  = 0;
    GT_U16                                      vlan        = 0;
    GT_U16                                      vlanRet     = 0;
    GT_PORT_NUM                                 portMax;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portMax = PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_PP_MAC(dev)->hwDevNum) ?
                    127 :
                    PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev);

        /*
            1.1. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E,
                                CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E,
                                CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E],
                           portNum[0, max/2, max],
                           vlan[0, max/2, max].
            Expected: GT_OK.
        */

        /* call with mode [CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E] */
        mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E;
        portNum = 0;
        vlan = 0;

        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. cpssDxChCfgIngressDropCntrModeGet.
            Expected: GT_OK and the same values as was set,
                except portNum(not relevant with this mode),
                vlan(not relevant with this mode).
        */
        st = cpssDxChCfgIngressDropCntrModeGet(dev, &modeRet, &portNumRet, &vlanRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgIngressDropCntrModeGet: %d", dev);

        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                         "got another mode: %d",
                                         modeRet);
        }

        /* call with mode [CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E] */
        mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E;
        portNum = portMax / 2;
        vlan = (GT_U16)((UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)/2)-1);

        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. cpssDxChCfgIngressDropCntrModeGet.
            Expected: GT_OK and the same values as was set,
                except portNum(not relevant with this mode).
        */
        st = cpssDxChCfgIngressDropCntrModeGet(dev, &modeRet, &portNumRet, &vlanRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgIngressDropCntrModeGet: %d", dev);

        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                         "got another mode: %d",
                                         modeRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(vlan, vlanRet,
                                         "got another vlan: %d",
                                         vlanRet);
        }

        /* call with mode [CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E] */
        mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E;
        portNum = portMax;
        vlan = (GT_U16)(UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev));

        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. cpssDxChCfgIngressDropCntrModeGet.
            Expected: GT_OK and the same values as was set,
                except vlan(not relevant with this mode).
        */
        st = cpssDxChCfgIngressDropCntrModeGet(dev, &modeRet, &portNumRet, &vlanRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChCfgIngressDropCntrModeGet: %d", dev);

        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                         "got another mode: %d",
                                         modeRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(portNum, portNumRet,
                                         "got another portNum: %d",
                                         portNumRet);
        }

        /*
            1.3. Call with out of range mode and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCfgIngressDropCntrModeSet
                            (dev, mode, portNum, vlan),
                            mode);

        /*
            1.4. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E],
                out of range portNum(is relevant) and other valid params.
            Expected: NON GT_OK.
        */
        mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E;
        portNum = portMax + 1;

        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E],
                out of range portNum(not relevant) and other valid params.
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E;
        portNum = portMax + 1;

        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        portNum = portMax;

        /*
            1.6. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E],
                out of range vlan(is relevant) and other valid params.
            Expected: NON GT_OK.
        */
        mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E;
        vlan = (GT_U16) UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.7. Call with mode[CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E],
                out of range vlan(not relevant) and other valid params.
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E;
        vlan = (GT_U16) UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        vlan = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev) - 1);
    }

    /* restore value */
    mode = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E;

    /*2. Go over all non active/non applicable devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgIngressDropCntrModeSet(dev, mode, portNum, vlan);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgIngressDropCntrModeGet
(
    IN   GT_U8                                       devNum,
    OUT  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT *modePtr,
    OUT  GT_PORT_NUM                                 *portNumPtr,
    OUT  GT_U16                                      *vlanPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgIngressDropCntrModeGet)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with non-null modePtr, non-null portNumPtr, non-null vlanPtr.
    Expected: GT_OK.
    1.2. Call with out of range modePtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st      = GT_OK;
    GT_U8                                           dev     = 0;
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT     mode    = 0;
    GT_PORT_NUM                                     portNum = 0;
    GT_U16                                          vlan    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null modePtr, non-null portNumPtr, non-null vlanPtr.
            Expected: GT_OK.
        */
        st = cpssDxChCfgIngressDropCntrModeGet(dev, &mode, &portNum, &vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range modePtr[NULL] and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgIngressDropCntrModeGet(dev, NULL, &portNum, &vlan);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgIngressDropCntrModeGet(dev, &mode, &portNum, &vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgIngressDropCntrModeGet(dev, &mode, &portNum, &vlan);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgNextDevGet)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with valid nextDevNumPtr.
    Expected: GT_OK.
    1.2. Call with out of range nextDevNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st          = GT_OK;
    GT_U8       dev         = 0;
    GT_U8       nextDevNum  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with valid nextDevNumPtr.
            Expected: GT_OK.
        */
        st = cpssDxChCfgNextDevGet(dev, &nextDevNum);
        st = GT_NO_MORE == st ? GT_OK : st;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range nextDevNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgNextDevGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, nextDevNumPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgNextDevGet(dev, &nextDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgNextDevGet(dev, &nextDevNum);
    st = GT_NO_MORE == st ? GT_BAD_PARAM : st;
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgHwAccessObjectBind)
{
/*
    1. Call with valid cfgAccessObj,
                 bind [GT_TRUE \ GT_FALSE].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;

    CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC cfgAccessObj;
    GT_BOOL                         bind;

    /* 1. */
    cfgAccessObj.hwAccessTableEntryReadFunc = &prvDxChCfgInitAccessPortGroupTblEntryReadCb;
    cfgAccessObj.hwAccessTableEntryWriteFunc = &prvDxChCfgInitAccessPortGroupTblEntryWriteCb;
    cfgAccessObj.hwAccessTableEntryFieldReadFunc = &prvDxChCfgInitAccessPortGroupTblEntryFieldReadCb;
    cfgAccessObj.hwAccessTableEntryFieldWriteFunc = &prvDxChCfgInitAccessPortGroupTblEntryFieldWriteCb;

    bind = GT_FALSE;

    st = cpssDxChCfgHwAccessObjectBind(&cfgAccessObj, bind);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, bind);

    /* 1. */
        bind = GT_TRUE;

        st = cpssDxChCfgHwAccessObjectBind(&cfgAccessObj, bind);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, bind);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgSharedTablesIllegalClientAccessInfoGet)
{
    GT_STATUS   st          = GT_OK;
    GT_U8       dev         = 0;
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC  illegalClientInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        do
        {
            /* get the next info about illegal access */
            st = cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(dev, &illegalClientInfo);
            if(st != GT_OK)
            {
                /* the 'GT_OK' and the 'GT_NO_MORE' are 'ok' returned value
                but other returned values are not 'ok' */
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NO_MORE, st, dev);
            }
        }
        while(st == GT_OK); /* drain the indications if multiple of them exists */

        /* handle NULL pointer */
        st = cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(dev, &illegalClientInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(dev, &illegalClientInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static GT_STATUS sharedTables_forceIllegalAccess_setReg_wm(
    IN GT_U8    devNum,
    IN GT_U32  regAddr,
    IN GT_U32  regValue
)
{
    GT_STATUS   rc;
    static GT_U32  portGroupId = 0;

    /* write register value */
    rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId , regAddr ,regValue);

    /* calculate portGroupId for next time */
    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile)
    {
        portGroupId += PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
    }
    else
    {
        portGroupId ++;
    }

    if(portGroupId >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
    {
        portGroupId = 0;
    }


    return rc;
}

#if (defined ASIC_SIMULATION) && (!defined GM_USED)
static GT_BOOL isWm = GT_TRUE;
#else
static GT_BOOL isWm = GT_FALSE;
#endif

/* the WM not supporting the recognition of 'illegal access' , but ... */
/* the WM allow to write to the register (it is not 'ROC' in WM) */
/* generate illegal access from ARP table */
static GT_STATUS sharedTables_forceIllegalAccess_arp_wm(
    IN GT_U8    devNum
)
{
    GT_U32  regAddr,regValue;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.arp_illegal_address;
    regValue = BIT_4  | /*b valid*/
               BIT_12 | /*pipe 1*/
               (0x1234) <<16;/*address*/

    /* write register value */
    return sharedTables_forceIllegalAccess_setReg_wm(devNum, regAddr , regValue);
}
/* the WM not supporting the recognition of 'illegal access' , but ... */
/* the WM allow to write to the register (it is not 'ROC' in WM) */
/* generate illegal access from FDB table */
static GT_STATUS sharedTables_forceIllegalAccess_fdb_wm(
    IN GT_U8    devNum
)
{
    GT_U32  regAddr,regValue;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.fdb_illegal_address;
    regValue = BIT_4  | /*b valid*/
               0      | /*pipe 0*/
               (16 << 8) | /* MHT 16 */
               (0x2345) <<16;/*address*/

    /* write register value */
    return sharedTables_forceIllegalAccess_setReg_wm(devNum, regAddr , regValue);
}
/* the WM not supporting the recognition of 'illegal access' , but ... */
/* the WM allow to write to the register (it is not 'ROC' in WM) */
/* generate illegal access from EM table */
static GT_STATUS sharedTables_forceIllegalAccess_em_wm(
    IN GT_U8    devNum
)
{
    GT_U32  regAddr,regValue;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.em_illegal_address;

    regValue = BIT_0  | /*a valid*/
               BIT_12 | /*pipe 1*/
               (8 << 8) | /* MHT 8 */
               (0x3456) <<16;/*address*/

    /* write register value */
    return sharedTables_forceIllegalAccess_setReg_wm(devNum, regAddr , regValue);
}
/* the WM not supporting the recognition of 'illegal access' , but ... */
/* the WM allow to write to the register (it is not 'ROC' in WM) */
/* generate illegal access from lpm table */
static GT_STATUS sharedTables_forceIllegalAccess_lpm_wm(
    IN GT_U8    devNum
)
{
    GT_U32  regAddr,regValue;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.lpm_illegal_address;

    regValue = BIT_0  | /*valid*/
               BIT_12 | /*pipe 1*/
               (13 << 4) | /* bank 13 */
               (0x4567) <<16;/*address*/

    /* write register value */
    return sharedTables_forceIllegalAccess_setReg_wm(devNum, regAddr , regValue);
}
/* the WM not supporting the recognition of 'illegal access' , but ... */
/* the WM allow to write to the register (it is not 'ROC' in WM) */
/* generate illegal access from lpm-aging table */
static GT_STATUS sharedTables_forceIllegalAccess_lpm_aging_wm(
    IN GT_U8    devNum
)
{
    GT_U32  regAddr,regValue;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.lpm_aging_illegal_address;

    regValue = BIT_0  | /*valid*/
               0      | /*pipe 0*/
               (0x5678) <<16;/*address*/

    /* write register value */
    return sharedTables_forceIllegalAccess_setReg_wm(devNum, regAddr , regValue);
}
/* the WM allow to clear to the register (it is not 'ROC' in WM) */
static GT_STATUS sharedTables_clearIllegalAccess_wm(
    IN GT_U8    devNum,
    IN CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC *illegalClientInfoPtr
)
{
    GT_U32  portGroupId;
    GT_U32  regAddr;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    switch(illegalClientInfoPtr->clientId)
    {
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_E        :
            regAddr = regsAddrPtr->SHM.lpm_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_EM_E         :
            regAddr = regsAddrPtr->SHM.em_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_FDB_E        :
            regAddr = regsAddrPtr->SHM.fdb_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ARP_E        :
            regAddr = regsAddrPtr->SHM.arp_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_AGING_E  :
            regAddr = regsAddrPtr->SHM.lpm_aging_illegal_address;
            break;
        default:
            return GT_BAD_PARAM;
    }

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile)
    {
        /* calculate representative port group */
        portGroupId = (illegalClientInfoPtr->portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile) *
                       PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
    }
    else
    {
        portGroupId = 0;
    }

    /* reset register value */
    return prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId , regAddr ,0/*reset the register*/);
}


/* generate illegal access from ARP table */
static GT_STATUS sharedTables_forceIllegalAccess_arp(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32  origSize;
    GT_ETHERADDR  arpMacAddr;

    if(isWm)
    {
        return sharedTables_forceIllegalAccess_arp_wm(devNum);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp >= _192K)
    {
        return GT_FULL;
    }
    /**************************************/
    /* we can try to cause illegal access */
    /**************************************/
    origSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp;
    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp = _192K;

    rc = cpssDxChIpRouterArpAddrRead(devNum,
        _192K - 1,/*index*/
        &arpMacAddr);

    /* restore */
    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp = origSize;

    return rc;
}
/* generate illegal access from FDB table */
static GT_STATUS sharedTables_forceIllegalAccess_fdb(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32  origSize;
    GT_BOOL                 valid;
    GT_BOOL                 skip;
    GT_BOOL                 aged;
    GT_HW_DEV_NUM           associatedHwDevNum;
    CPSS_MAC_ENTRY_EXT_STC  entryInfo;

    if(isWm)
    {
        return sharedTables_forceIllegalAccess_fdb_wm(devNum);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb >= _256K)
    {
        return GT_FULL;
    }
    /**************************************/
    /* we can try to cause illegal access */
    /**************************************/
    origSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb;

    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb = _256K;
    rc = cpssDxChBrgFdbPortGroupMacEntryRead(devNum,
        BIT_0,/*portGroupsBmp*/
        _256K - 1,/*index*/
        &valid,&skip,&aged,&associatedHwDevNum,&entryInfo);

    /* restore */
    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb = origSize;

    return rc;
}
/* generate illegal access from EM table */
static GT_STATUS sharedTables_forceIllegalAccess_em(
    IN GT_U8    devNum
)
{
    if(isWm)
    {
        return sharedTables_forceIllegalAccess_em_wm(devNum);
    }

    /* we not have CPSS API nor definition for the table ... yet */
    return GT_NOT_IMPLEMENTED;
}
/* generate illegal access from EM table */
static GT_STATUS sharedTables_forceIllegalAccess_lpm(
    IN GT_U8    devNum
)
{
    if(isWm)
    {
        return sharedTables_forceIllegalAccess_lpm_wm(devNum);
    }

    return GT_NOT_IMPLEMENTED;
}
/* generate illegal access from lpm-aging table */
static GT_STATUS sharedTables_forceIllegalAccess_lpm_aging(
    IN GT_U8    devNum
)
{
    if(isWm)
    {
        return sharedTables_forceIllegalAccess_lpm_aging_wm(devNum);
    }

    return GT_NOT_IMPLEMENTED;
}



/* call to cpssDxChCfgSharedTablesIllegalClientAccessInfoGet , after generated
   'reasons' for illegal access */
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgSharedTablesIllegalClientAccessInfoGet_enhanced)
{
    GT_U32      numDevs = 0;
    GT_U32      bmpOfClients,supported_bmpOfClients;

    GT_STATUS   st          = GT_OK;
    GT_U8       dev         = 0;
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC  illegalClientInfo;
    GT_U32      currIteration,maxIterations = 16;
    GT_U32      brokeIterations;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* The GM not supports the SHM unit */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    supported_bmpOfClients =
        1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_E |
        1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_FDB_E |
        1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_EM_E  |
        1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ARP_E |
        1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_AGING_E;
    if(! isWm)
    {
        supported_bmpOfClients &= ~(
            1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_E |
            1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_EM_E  |
            1 << CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_AGING_E);
    }


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numDevs++;
        bmpOfClients = 0;

        /* generate lpm access violation */
        (void) sharedTables_forceIllegalAccess_lpm(dev);
        /* generate ARP access violation */
        (void) sharedTables_forceIllegalAccess_arp(dev);
        /* generate FDB access violation */
        (void) sharedTables_forceIllegalAccess_fdb(dev);
        /* generate EM access violation */
        (void) sharedTables_forceIllegalAccess_em(dev);
        /* generate lpm aging access violation */
        (void) sharedTables_forceIllegalAccess_lpm_aging(dev);

        currIteration = 0;
        brokeIterations = 0;
        do
        {
            /* get the next info about illegal access */
            st = cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(dev, &illegalClientInfo);
            if(st != GT_OK)
            {
                /* the 'GT_OK' and the 'GT_NO_MORE' are 'ok' returned value
                but other returned values are not 'ok' */
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NO_MORE, st, dev);
            }

            if(st == GT_OK)
            {
                if(isWm)
                {
                    /* clear the client */
                    sharedTables_clearIllegalAccess_wm(dev,&illegalClientInfo);
                }
                /* state that the client gave at least single error */
                bmpOfClients |= 1 << illegalClientInfo.clientId;

                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0,illegalClientInfo.lineIndex);
            }

            if(currIteration >= maxIterations)
            {
                brokeIterations = 1;
                break;
            }
            currIteration++;
        }
        while(st == GT_OK); /* drain the indications if multiple of them exists */

        /* check that we not broke the iterations */
        UTF_VERIFY_EQUAL0_PARAM_MAC(0,brokeIterations);

        /* check that we got indication from the supported clients */
        UTF_VERIFY_EQUAL0_PARAM_MAC(supported_bmpOfClients,bmpOfClients);
    }

    if(numDevs == 0)
    {
        /* register the test as 'skipped' (not as succeeded) */
        SKIP_TEST_MAC;
    }
}

void dxChSharedTabled_DebugEnable
(
    IN GT_U32  enableDebug
);
/* test from the 'terminal' with parameters from the caller */
GT_STATUS testCasesFor_falcon_cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT        clientId,
    IN  GT_U32                portGroupId
)
{
    GT_STATUS   rc;
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC    clientInfo;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* set the 'initial values' to be according to the input parameters */
    PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextTableToServe     = clientId;
    PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextPortGroupToServe = portGroupId;

    dxChSharedTabled_DebugEnable(1);

    rc = cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(devNum,&clientInfo);

    dxChSharedTabled_DebugEnable(0);

    return rc;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgReplicationCountersGet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_CFG_REPLICATION_TYPE_ENT            counterType,
    OUT GT_U32                                        *counterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgReplicationCountersGet)
{
/*
    1.1 Call with non-null counterPtr
    Expected: GT_OK.
    1.2 Call api with wrong enable counterPtr [NULL].
    Expected: GT_BAD_PTR.
*/


    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;

    GT_U32      counterPtr;

    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT repType = CPSS_DXCH_CFG_QCN_REPLICATION_E;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non-null counterPtr
            Expected: GT_OK.
        */
        st = cpssDxChCfgReplicationCountersGet(devNum, repType, &counterPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call api with wrong enable counterPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgReplicationCountersGet(devNum, repType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.3 Call api with all valid replication type
            Expected : GT_OK
        */
        for (repType = CPSS_DXCH_CFG_QCN_REPLICATION_E  ;repType <= CPSS_DXCH_CFG_MIRROR_FIFO_DROP_E ; repType++)
        {
            st = cpssDxChCfgReplicationCountersGet(devNum, repType, &counterPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, ~UTF_FALCON_E);


 /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCfgReplicationCountersGet(devNum, repType, &counterPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCfgReplicationCountersGet(devNum, repType, &counterPtr);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssDxChCfgReplicationCountersGet(devNum, repType, &counterPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------
*
GT_STATUS internal_cpssDxChCfgProbePacketConfigSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_PROBE_PACKET_CFG_STC *probeCfgPtr
)
-----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgProbePacketConfigSet)
{
/*
    1.1 Call with valid config
    Expected: GT_OK.
    1.2 Call api with invalid bitLocation value.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;
    GT_U8       i;
    CPSS_DXCH_CFG_PROBE_PACKET_STC  probeCfg;
    CPSS_DXCH_CFG_PROBE_PACKET_STC  probeCfgTmp;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, ~UTF_FALCON_E);

     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        for( i = 0; i < 12 ; i++)
        {
            probeCfg.bitLocation = i;
            probeCfg.enable = GT_TRUE;

            st = cpssDxChCfgProbePacketConfigSet( devNum, &probeCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            st = cpssDxChCfgProbePacketConfigGet(devNum, &probeCfgTmp);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(probeCfgTmp.enable, probeCfg.enable, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(probeCfgTmp.bitLocation, probeCfg.bitLocation, devNum);
       }

       /* call with invalid bitLocation */
       probeCfg.bitLocation  = 12;
       st = cpssDxChCfgProbePacketConfigSet(devNum, &probeCfg);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);

       probeCfg.enable =  GT_FALSE;
       probeCfg.bitLocation  = 0;

       st = cpssDxChCfgProbePacketConfigSet(devNum, &probeCfg);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, ~UTF_FALCON_E);

    /*Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCfgProbePacketConfigSet(devNum, &probeCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }
}

/*----------------------------------------------------------------------------
*
GT_STATUS cpssDxChCfgProbePacketConfigGet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_PROBE_PACKET_CFG_STC *probeCfgPtr
)
-----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgProbePacketConfigGet)
{
/*
    1.1 Call with valid probe packet config.
    Expected: GT_OK.
    1.2 Call api with wrong probeCFgPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;
    CPSS_DXCH_CFG_PROBE_PACKET_STC  probeCfg;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, ~UTF_FALCON_E);

     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
       probeCfg.enable =  GT_FALSE;
       probeCfg.bitLocation = 12;

       st = cpssDxChCfgProbePacketConfigSet(devNum, &probeCfg);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);

       st = cpssDxChCfgProbePacketConfigSet(devNum, NULL);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
    }
}

/*----------------------------------------------------------------------------
*
GT_STATUS cpssDxChCfgProbePacketConfigGet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_PROBE_PACKET_CFG_STC *probeCfgPtr
)
-----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCfgProbePacketDropCodeGet)
{
/*
    1.1 Call with valid probe packet config.
    Expected: GT_OK.
    1.2 Call api with wrong probeCFgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChCfgInit suit
 */
    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;
    CPSS_DIRECTION_ENT    direction;
    CPSS_NET_RX_CPU_CODE_ENT dropCode;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, ~UTF_FALCON_E);

    direction = CPSS_DIRECTION_INGRESS_E;
     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st  = cpssDxChCfgProbePacketDropCodeGet( devNum, direction, &dropCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        st  = cpssDxChCfgProbePacketDropCodeGet( devNum, direction, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgDevDbInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_IF_CFG_STC        **prevCpssInit_netIfCfgPtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_auqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_fuqUseSeparatePtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_fuqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_useMultiNetIfSdmaPtrPtr,
    OUT CPSS_MULTI_NET_IF_CFG_STC  **prevCpssInit_multiNetIfCfgPtrPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChCfgDevDbInfoGet)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Call with not NULL OUT parameters.
    Expected: GT_OK.
    1.2. Call with [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_NET_IF_CFG_STC        *prevCpssInit_netIfCfgPtr = NULL;
    CPSS_AUQ_CFG_STC           *prevCpssInit_auqCfgPtr   = NULL;
    GT_BOOL                    *prevCpssInit_fuqUseSeparatePtr = NULL;
    CPSS_AUQ_CFG_STC           *prevCpssInit_fuqCfgPtr = NULL;
    GT_BOOL                    *prevCpssInit_useMultiNetIfSdmaPtr = NULL;
    CPSS_MULTI_NET_IF_CFG_STC  *prevCpssInit_multiNetIfCfgPtr = NULL;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL .
            Expected: GT_OK.
        */
        st = cpssDxChCfgDevDbInfoGet(dev, &prevCpssInit_netIfCfgPtr,
                                          &prevCpssInit_auqCfgPtr,
                                          &prevCpssInit_fuqUseSeparatePtr,
                                          &prevCpssInit_fuqCfgPtr,
                                          &prevCpssInit_useMultiNetIfSdmaPtr,
                                          &prevCpssInit_multiNetIfCfgPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChCfgDevDbInfoGet(dev, NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgDevDbInfoGet(dev, &prevCpssInit_netIfCfgPtr,
                                          &prevCpssInit_auqCfgPtr,
                                          &prevCpssInit_fuqUseSeparatePtr,
                                          &prevCpssInit_fuqCfgPtr,
                                          &prevCpssInit_useMultiNetIfSdmaPtr,
                                          &prevCpssInit_multiNetIfCfgPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgDevDbInfoGet(dev, &prevCpssInit_netIfCfgPtr,
                                      &prevCpssInit_auqCfgPtr,
                                      &prevCpssInit_fuqUseSeparatePtr,
                                      &prevCpssInit_fuqCfgPtr,
                                      &prevCpssInit_useMultiNetIfSdmaPtr,
                                      &prevCpssInit_multiNetIfCfgPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChCfgInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgReNumberDevNum)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDevEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDevEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDsaTagSrcDevPortModifySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDsaTagSrcDevPortModifyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgTableNumEntriesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDevRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgPpLogicalInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgPpLogicalInit_enhanced)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDevInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgBindPortPhymacObject)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgPortRemotePhyMacBind_FwLoadTo88e1690)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgPortDefaultSourceEportNumberSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgPortDefaultSourceEportNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgEgressHashSelectionModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgEgressHashSelectionModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgIngressDropCntrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgIngressDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgGlobalEportSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgGlobalEportGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgIngressDropCntrModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgIngressDropCntrModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgNextDevGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgHwAccessObjectBind)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgSharedTablesIllegalClientAccessInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgSharedTablesIllegalClientAccessInfoGet_enhanced)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgReplicationCountersGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgProbePacketConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgProbePacketConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgProbePacketDropCodeGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgDevDbInfoGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChCfgInit)

