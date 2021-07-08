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
* @file cpssPxCfgInitUT.c
*
* @brief Unit Tests for CPSS PX general config APIs.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/config/cpssPxCfgInit.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

static GT_STATUS prvPxCfgInitAccessPortGroupTblEntryReadCb
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  convertedPortGroupId,
    IN  CPSS_PX_TABLE_ENT tableType,
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

static GT_STATUS prvPxCfgInitAccessPortGroupTblEntryWriteCb
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_PX_TABLE_ENT tableType,
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

static GT_STATUS prvPxCfgInitAccessPortGroupTblEntryFieldReadCb
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_PX_TABLE_ENT tableType,
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

static GT_STATUS prvPxCfgInitAccessPortGroupTblEntryFieldWriteCb
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_PX_TABLE_ENT tableType,
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

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCfgReNumberDevNum
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
)
*/
UTF_TEST_CASE_MAC(cpssPxCfgReNumberDevNum)
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

    if(prvUtfIsAc3xWithPipeSystem())
    {
        devNumbersToTest[0] = 1;
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


        st = cpssPxCfgReNumberDevNum(oldDevNum, newDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, oldDevNum, newDevNum);


        st = cpssPxCfgReNumberDevNum(newDevNum, oldDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, newDevNum, oldDevNum);

        /* call with newDevNum = 16 */
        oldDevNum = dev;
        newDevNum = devNumbersToTest[1];


        st = cpssPxCfgReNumberDevNum(oldDevNum, newDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, oldDevNum, newDevNum);


        st = cpssPxCfgReNumberDevNum(newDevNum, oldDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, newDevNum, oldDevNum);

        /* call with newDevNum = 31 */
        oldDevNum = dev;
        newDevNum = devNumbersToTest[2];


        st = cpssPxCfgReNumberDevNum(oldDevNum, newDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, newDevNum);


        st = cpssPxCfgReNumberDevNum(newDevNum, oldDevNum);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, newDevNum, oldDevNum);
    }

    newDevNum = 0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */

        st = cpssPxCfgReNumberDevNum(dev, newDevNum);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;


    st = cpssPxCfgReNumberDevNum(dev, newDevNum);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCfgDevEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssPxCfgDevEnable)
{
    /*
    ITERATE_DEVICES
    1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Px devices and GT_BAD_PARAM for others.
    1.2. Call cpssPxCfgDevEnableGet.
    Expected: GT_OK and same enablePtr.
    */
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_BOOL                     enable;
    GT_BOOL                     enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with enable [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK for Px devices and GT_BAD_PARAM for others.    */
        enable = GT_FALSE;

        st = cpssPxCfgDevEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. */
        st = cpssPxCfgDevEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d, %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssPxCfgDevEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. */
        st = cpssPxCfgDevEnableGet(dev, &enableGet);
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
        st = cpssPxCfgDevEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCfgDevEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxCfgDevEnableGet)
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
        st = cpssPxCfgDevEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. */
        st = cpssPxCfgDevEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxCfgDevEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCfgDevEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCfgTableNumEntriesGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PX_CFG_TABLES_ENT    table,
    OUT GT_U32                      *numEntriesPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxCfgTableNumEntriesGet)
{
/*
    ITERATE_DEVICE (Pxx)
    1.1. Call with table [CPSS_PX_CFG_TABLE_VLAN_E /
                          CPSS_PX_CFG_TABLE_FDB_E /
                          CPSS_PX_CFG_TABLE_PCL_ACTION_E],
              not NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values table.
    Expected: GT_BAD_PARAM.
    1.3. Call with numEntriesPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_TABLE_ENT    table;
    GT_U32                      numEntries  = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(table = 0 ; table < CPSS_PX_TABLE_LAST_E ; table ++)
        {
            st = cpssPxCfgTableNumEntriesGet(dev, table, &numEntries);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, table);
        }

        /*
            1.2. Call with wrong enum values table.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxCfgTableNumEntriesGet
                            (dev, table, &numEntries),
                            table);

        /*
            1.3. Call with numEntriesPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxCfgTableNumEntriesGet(dev, table, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, numEntriesPtr = NULL", dev, table);
    }

    table = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCfgTableNumEntriesGet(dev, table, &numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCfgTableNumEntriesGet(dev, table, &numEntries);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCfgDevRemove
(
    IN  GT_U8                       devNum,
)
*/
UTF_TEST_CASE_MAC(cpssPxCfgDevRemove)
{
/*
    ITERATE_DEVICE (Pxx)
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

        /*st = cpssPxCfgDevRemove(dev);*/
        /*UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);*/
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCfgDevRemove(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCfgDevRemove(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCfgDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_PX_CFG_DEV_INFO_STC   *devInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxCfgDevInfoGet)
{
/*
    ITERATE_DEVICE (Pxx)
    1.1. Call with not NULL devInfoPtr.
    Expected: GT_OK.
    1.2. Call with devInfoPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_CFG_DEV_INFO_STC   devInfo;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL devInfoPtr.
            Expected: GT_OK.
        */
        st = cpssPxCfgDevInfoGet(dev, &devInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with devInfoPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxCfgDevInfoGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, devInfoPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCfgDevInfoGet(dev, &devInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCfgDevInfoGet(dev, &devInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxCfgNextDevGet)
{
/*
    ITERATE_DEVICE (Pxx)
    1.1. Call with valid nextDevNumPtr.
    Expected: GT_OK.
    1.2. Call with out of range nextDevNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st          = GT_OK;
    GT_U8       dev         = 0;
    GT_SW_DEV_NUM       nextDevNum  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with valid nextDevNumPtr.
            Expected: GT_OK.
        */
        st = cpssPxCfgNextDevGet(dev, &nextDevNum);
        st = GT_NO_MORE == st ? GT_OK : st;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range nextDevNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxCfgNextDevGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, nextDevNumPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCfgNextDevGet(dev, &nextDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCfgNextDevGet(dev, &nextDevNum);
    st = GT_NO_MORE == st ? GT_BAD_PARAM : st;
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxCfgHwAccessObjectBind)
{
/*
    1. Call with valid cfgAccessObj,
                 bind [GT_TRUE \ GT_FALSE].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;

    CPSS_PX_CFG_HW_ACCESS_OBJ_STC cfgAccessObj;
    GT_BOOL                         bind;

    /* 1. */
    cfgAccessObj.hwAccessTableEntryReadFunc         = &prvPxCfgInitAccessPortGroupTblEntryReadCb;
    cfgAccessObj.hwAccessTableEntryWriteFunc        = &prvPxCfgInitAccessPortGroupTblEntryWriteCb;
    cfgAccessObj.hwAccessTableEntryFieldReadFunc    = &prvPxCfgInitAccessPortGroupTblEntryFieldReadCb;
    cfgAccessObj.hwAccessTableEntryFieldWriteFunc   = &prvPxCfgInitAccessPortGroupTblEntryFieldWriteCb;

    /* 1. */
    bind = GT_TRUE;

    st = cpssPxCfgHwAccessObjectBind(&cfgAccessObj, bind);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, bind);

    /* unbind --> restore */
    bind = GT_FALSE;

    st = cpssPxCfgHwAccessObjectBind(&cfgAccessObj, bind);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, bind);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCfgCntrGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_PX_CFG_CNTR_ENT  counterType, 
    OUT GT_U32  *cntPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxCfgCntrGet)
{
/*
    ITERATE_DEVICE (Pxx)
    1.1. Call with all counter types from enum CPSS_PX_CFG_CNTR_ENT
              not NULL cntPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values table.
    Expected: GT_BAD_PARAM.
    1.3. Call with cntPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_CFG_CNTR_ENT    counterType;
    GT_U32                  counterVal = 0;


    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all counter types from enum CPSS_PX_CFG_CNTR_ENT
            not NULL cntPtr.
            Expected: GT_OK.
        */
        for(counterType = 0; counterType < CPSS_PX_CFG_CNTR_LAST_E; counterType++)
        {
            st = cpssPxCfgCntrGet(dev, counterType, &counterVal);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, counterType);
        }

        /*
            1.2. Call with wrong enum values table.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxCfgCntrGet
                            (dev, counterType, &counterVal),
                            counterType);

        /*
            1.3. Call with counterVal [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxCfgCntrGet(dev, counterType, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, counterValPtr = NULL", dev, counterType);
    }

    counterType = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCfgCntrGet(dev, counterType, &counterVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCfgCntrGet(dev, counterType, &counterVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxCfgInit suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxCfgInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgReNumberDevNum)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgDevEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgDevEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgTableNumEntriesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgDevRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgDevInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgNextDevGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgHwAccessObjectBind)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCfgCntrGet)
UTF_SUIT_END_TESTS_MAC(cpssPxCfgInit)

