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
* @file cpssDxChLogicalTargetMappingUT.c
*
* @brief Unit tests for
* Logical Target mapping.
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/logicalTarget/cpssDxChLogicalTargetMapping.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Tests use this vlan id for testing VLAN functions */
#define LTM_VLAN_TESTED_VLAN_ID_CNS     100

/* Tests use this port id for testing port functions */
#define LTM_TESTED_PORT_NUM_CNS           0

/* Invalid port id for testing port functions */
#define LTM_INVALID_PORT_NUM_CNS         64

/* Correct logical dev num value */
#define LTM_LOGICAL_DEV_NUM_CNS          24

/* Invalid logical dev num value */
#define LTM_INVALID_LOGICAL_DEV_NUM_CNS  23

/* Invalid logical dev num value */
#define LTM_INVALID_LOGICAL_DEV_NUM_FOR_VPLS_CNS 32

/* maximum num for logical device number */
#define LTM_MAX_LOGICAL_DEV_NUM_FOR_VPLS_CNS 31

/* maximum num for logical port number */
#define LTM_MAX_LOGICAL_PORT_NUM_FOR_VPLS_CNS 63

/* minimum num for logical device number for device other then XCAT C0*/
#define LTM_XCAT_MIN_LOGICAL_DEV_NUM_CNS    24

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingDeviceEnableSet
(
    IN GT_U8      devNum,
    IN GT_U8      logicalDevNum,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingDeviceEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS]
              and enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingDeviceEnableGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8       logicalDevNum = LTM_LOGICAL_DEV_NUM_CNS;
    GT_BOOL     enable    = GT_TRUE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS]
                      and enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */

        /*call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS] and enable [GT_TRUE]*/
        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;
        enable = GT_TRUE;

        st = cpssDxChLogicalTargetMappingDeviceEnableSet(dev, logicalDevNum, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, enable);

        /*
            1.2. Call cpssDxChLogicalTargetMappingDeviceEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChLogicalTargetMappingDeviceEnableGet(dev, logicalDevNum, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

        /*call with enable [GT_FALSE]*/
        enable = GT_FALSE;

        st = cpssDxChLogicalTargetMappingDeviceEnableSet(dev, logicalDevNum, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, enable);

        /*
            1.2. Call cpssDxChLogicalTargetMappingDeviceEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChLogicalTargetMappingDeviceEnableGet(dev, logicalDevNum, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

        /*
            1.3. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            logicalDevNum  = LTM_INVALID_LOGICAL_DEV_NUM_FOR_VPLS_CNS;
        }
        else
        {
            logicalDevNum  = LTM_INVALID_LOGICAL_DEV_NUM_CNS;
        }
        enable = GT_TRUE;

        st = cpssDxChLogicalTargetMappingDeviceEnableSet(dev, logicalDevNum, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, logicalDevNum, enable);

        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingDeviceEnableSet(dev, logicalDevNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingDeviceEnableSet(dev, logicalDevNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingDeviceEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     logicalDevNum,
    OUT  GT_BOOL   *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingDeviceEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS].
    Expected: GT_OK.
    1.2. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enable pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8       logicalDevNum = LTM_LOGICAL_DEV_NUM_CNS;
    GT_BOOL     enable    = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS].
            Expected: GT_OK.
        */

        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;

        st = cpssDxChLogicalTargetMappingDeviceEnableGet(dev, logicalDevNum, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, enable);

        /*
            1.2. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            logicalDevNum  = LTM_INVALID_LOGICAL_DEV_NUM_FOR_VPLS_CNS;
        }
        else
        {
            logicalDevNum  = LTM_INVALID_LOGICAL_DEV_NUM_CNS;
        }

        st = cpssDxChLogicalTargetMappingDeviceEnableGet(dev, logicalDevNum, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, logicalDevNum, enable);

        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;

        /*
            1.3. Call with wrong enable pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChLogicalTargetMappingDeviceEnableGet(dev, logicalDevNum, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, logicalDevNum, enable);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingDeviceEnableGet(dev, logicalDevNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingDeviceEnableGet(dev, logicalDevNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEnableSet
(
    IN GT_U8      devNum,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with  enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingEnableGet with the same params.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable    = GT_TRUE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */

        /*call with enable [GT_TRUE]*/
        enable = GT_TRUE;

        st = cpssDxChLogicalTargetMappingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChLogicalTargetMappingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChLogicalTargetMappingEnableGet(dev, &enableGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

        /*call with enable [GT_FALSE]*/
        enable = GT_FALSE;

        st = cpssDxChLogicalTargetMappingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChLogicalTargetMappingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChLogicalTargetMappingEnableGet(dev, &enableGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with non-NULL enable pointer.
    Expected: GT_OK.
    1.3. Call with wrong enable pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable    = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL enable pointer.
            Expected: GT_OK.
        */

        st = cpssDxChLogicalTargetMappingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call with wrong enable pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChLogicalTargetMappingEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, enable);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingTableEntrySet
(
    IN GT_U8                                 devNum,
    IN GT_U8                                 logicalDevNum,
    IN GT_U8                                 logicalPortNum,
    IN CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  *logicalPortMappingTablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingTableEntrySet)
{
/*
    ITERATE_DEVICES (xCat and above)
   1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                  logicalPortNum [LTM_TESTED_PORT_NUM_CNS]
                  and logicalPortMappingTable.isTunnelStart [GT_GT_TRUE],
                  logicalPortMappingTable.tunnelStartInfo.passengerPacketType [CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E],
                  logicalPortMappingTable.tunnelStartInfo.ptr [1].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                   logicalPortNum [LTM_TESTED_PORT_NUM_CNS]
                   and logicalPortMappingTable.isTunnelStart [GT_FALSE],
                   logicalPortMappingTable.tunnelStartInfo.passengerPacketType [wrong enum values],
                   logicalPortMappingTable.tunnelStartInfo.ptr [1].
    Expected: GT_OK
    1.4. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                   logicalPortNum [LTM_TESTED_PORT_NUM_CNS]
                   and logicalPortMappingTable.isTunnelStart [GT_TRUE],
                   logicalPortMappingTable.tunnelStartInfo.passengerPacketType [wrong enum values],
                   logicalPortMappingTable.tunnelStartInfo.ptr [1].
    Expected: GT_BAD_PARAM
    1.5. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong logicalPortNum [LTM_INVALID_PORT_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong enum values for logicalPortMappingTable.egressVlanTagState and
    and logicalPortMappingTable.egressVlanTagStateEnable [GT_TRUE].
    Expected: GT_BAD_PARAM.
    1.8. Same as 1.1 except logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E]
    and logicalPortMappingTable.egressVlanTagStateEnable [GT_TRUE].
    Expected: GT_OK.
    1.9. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params as 1.8
    Expected: GT_OK, logicalPortMappingTable.egressVlanTagStateEnable [GT_TRUE]
    and logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E].
    1.10. Same as 1.1 except logicalPortMappingTable.egressVlanTagStateEnable [GT_FALSE]
    and logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E]..
    Expected: GT_OK.
    1.11. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params as 1.10.
    Expected: GT_OK, logicalPortMappingTable.egressVlanTagStateEnable [GT_FALSE] and
    logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E].
    1.12. Same as 1.1 except logicalPortMappingTable.egressVlanTagStateEnable [GT_FALSE].
    Expected: GT_OK.
    1.13. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params as 1.12.
    Expected: GT_OK, logicalPortMappingTable.egressVlanTagStateEnable [GT_FALSE] and
    logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E].
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      i,j;
    GT_U32      numErrors = 0;

    GT_U8       logicalDevNum = LTM_LOGICAL_DEV_NUM_CNS;
    GT_U8       logicalPortNum = LTM_TESTED_PORT_NUM_CNS;

    CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  logicalPortMappingTable;
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  logicalPortMappingTableGet;

    GT_BOOL     isEqual = GT_FALSE;
    GT_U8   hwDev , hwPort;

    cpssOsBzero((GT_VOID*)&logicalPortMappingTable, sizeof(logicalPortMappingTable));
    cpssOsBzero((GT_VOID*)&logicalPortMappingTableGet, sizeof(logicalPortMappingTableGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                 logicalPortNum [LTM_TESTED_PORT_NUM_CNS] and logicalPortMappingTable.outputInterface.isTunnelStart [GT_TRUE],
                 logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType [CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E],
                 logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr [1].
            Expected: GT_OK.
        */

        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;
        logicalPortNum = LTM_TESTED_PORT_NUM_CNS;
        logicalPortMappingTable.outputInterface.isTunnelStart = GT_TRUE;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr = 1;

        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, logicalPortNum);

        /*
            1.2. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                               logicalPortNum, &logicalPortMappingTableGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, logicalDevNum);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(logicalPortMappingTable),
                                     (GT_VOID*) &(logicalPortMappingTableGet),
                                     sizeof(logicalPortMappingTable))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another logicalPortMappingTable than was set: %d", dev);


        /*
            1.3. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                           logicalPortNum [LTM_TESTED_PORT_NUM_CNS]
                           and logicalPortMappingTable.isTunnelStart [GT_FALSE],
                           logicalPortMappingTable.tunnelStartInfo.passengerPacketType [wrong enum values],
                           logicalPortMappingTable.tunnelStartInfo.ptr [1].
            Expected: GT_OK
        */

        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;
        logicalPortNum = LTM_TESTED_PORT_NUM_CNS;

        logicalPortMappingTable.outputInterface.isTunnelStart = GT_FALSE;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr = 1;

        for(enumsIndex = 0; enumsIndex < utfInvalidEnumArrSize; enumsIndex++)
        {
            logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType
                                                        = utfInvalidEnumArr[enumsIndex];
            st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                        logicalPortNum, &logicalPortMappingTable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, logicalPortNum);
        }

        /*
            1.4. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                           logicalPortNum [LTM_TESTED_PORT_NUM_CNS]
                           and logicalPortMappingTable.isTunnelStart [GT_TRUE],
                           logicalPortMappingTable.tunnelStartInfo.passengerPacketType
                           [wrong enum values],
                           logicalPortMappingTable.tunnelStartInfo.ptr [1].
            Expected: GT_BAD_PARAM
        */
        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;
        logicalPortNum = LTM_TESTED_PORT_NUM_CNS;
        logicalPortMappingTable.outputInterface.isTunnelStart = GT_TRUE;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr = 1;

        UTF_ENUMS_CHECK_MAC(cpssDxChLogicalTargetMappingTableEntrySet
                            (dev, logicalDevNum, logicalPortNum, &logicalPortMappingTable),
                            logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType);

        /*
            1.5. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            logicalDevNum  = LTM_INVALID_LOGICAL_DEV_NUM_FOR_VPLS_CNS;
        }
        else
        {
            logicalDevNum  = LTM_INVALID_LOGICAL_DEV_NUM_CNS;
        }

        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, logicalDevNum, logicalPortNum);

        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;

        /*
            1.6. Call with wrong logicalPortNum [LTM_INVALID_PORT_NUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        logicalPortNum = LTM_INVALID_PORT_NUM_CNS;

        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, logicalDevNum, logicalPortNum);

        logicalPortNum = LTM_TESTED_PORT_NUM_CNS;

        /*
            1.7. Call with wrong enum values for logicalPortMappingTable.egressVlanTagState
            and logicalPortMappingTable.outputInterface.egressVlanTagStateEnable = GT_TRUE.
            Expected: GT_BAD_PARAM
         */

        logicalPortMappingTable.egressVlanTagStateEnable = GT_TRUE;

        UTF_ENUMS_CHECK_MAC(cpssDxChLogicalTargetMappingTableEntrySet
                            (dev, logicalDevNum, logicalPortNum, &logicalPortMappingTable),
                            logicalPortMappingTable.egressVlanTagState);

        /*
            1.8. Same as 1.1 except logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E]
            and logicalPortMappingTable.egressVlanTagStateEnable [GT_TRUE].
            Expected: GT_OK
         */

        logicalPortMappingTable.outputInterface.isTunnelStart = GT_TRUE;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr = 1;
        logicalPortMappingTable.egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        logicalPortMappingTable.egressVlanTagStateEnable = GT_TRUE;

        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, logicalPortNum);

        /*
            1.9. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params as 1.8.
            Expected: GT_OK, logicalPortMappingTable.egressVlanTagStateEnable [GT_TRUE]
            and logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E]..
         */

        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                               logicalPortNum, &logicalPortMappingTableGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, logicalDevNum);

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(logicalPortMappingTable),
                                         (GT_VOID*) &(logicalPortMappingTableGet),
                                         sizeof(logicalPortMappingTable))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another logicalPortMappingTable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, logicalPortMappingTableGet.egressVlanTagStateEnable,
                                      "egressVlanTagStateEnable expected GT_TRUE, got %d",
                                      logicalPortMappingTableGet.egressVlanTagStateEnable);
        }

        /*
            1.10. Same as 1.8 except logicalPortMappingTable.outputInterface.egressVlanTagStateEnable [GT_FALSE]
            and logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E].
            Expected: GT_OK
         */

        logicalPortMappingTable.outputInterface.isTunnelStart = GT_TRUE;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr = 1;
        logicalPortMappingTable.egressVlanTagStateEnable = GT_FALSE;
        logicalPortMappingTable.egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;

        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, logicalPortNum);

        /*
            1.11. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params as 1.10.
            Expected: GT_OK, logicalPortMappingTable.egressVlanTagStateEnable [GT_FALSE] and
            logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E].
         */

        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                               logicalPortNum, &logicalPortMappingTableGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, logicalDevNum);

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(logicalPortMappingTable),
                                         (GT_VOID*) &(logicalPortMappingTableGet),
                                         sizeof(logicalPortMappingTable))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, isEqual,
                                      "get another logicalPortMappingTable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, logicalPortMappingTableGet.egressVlanTagStateEnable,
                                      "egressVlanTagStateEnable expected GT_FALSE, got %d",
                                      logicalPortMappingTableGet.egressVlanTagStateEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E, logicalPortMappingTableGet.egressVlanTagState,
                                      "egressVlanTagStateEnable expected CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E, got %d",
                                      logicalPortMappingTableGet.egressVlanTagState);
        }

        /*
            1.12. Same as 1.1 except logicalPortMappingTable.egressVlanTagStateEnable = GT_FALSE.
            Expected: GT_OK
         */

        logicalPortMappingTable.outputInterface.isTunnelStart = GT_TRUE;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr = 1;
        logicalPortMappingTable.egressVlanTagStateEnable = GT_FALSE;

        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, logicalPortNum);

        /*
            1.13. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params as 1.12.
            Expected: GT_OK, logicalPortMappingTable.egressVlanTagStateEnable [GT_FALSE] and
            logicalPortMappingTable.egressVlanTagState [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E].
         */

        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                               logicalPortNum, &logicalPortMappingTableGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, logicalDevNum);

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(logicalPortMappingTable),
                                         (GT_VOID*) &(logicalPortMappingTableGet),
                                         sizeof(logicalPortMappingTable))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, isEqual,
                                      "get another logicalPortMappingTable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, logicalPortMappingTableGet.egressVlanTagStateEnable,
                                      "egressVlanTagStateEnable expected GT_FALSE got %d",
                                      logicalPortMappingTableGet.egressVlanTagStateEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E, logicalPortMappingTableGet.egressVlanTagState,
                                      "egressVlanTagStateEnable expected CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E, got %d",
                                      logicalPortMappingTableGet.egressVlanTagState);
        }

        /*
            1.14. set full table ... then read full table .. check entries not override each other
            Expected: GT_OK.
        */

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(dev) == GT_TRUE)
            i = 0;
        else
            i = LTM_XCAT_MIN_LOGICAL_DEV_NUM_CNS;

        cpssOsBzero((GT_VOID*)&logicalPortMappingTable, sizeof(logicalPortMappingTable));
        cpssOsBzero((GT_VOID*)&logicalPortMappingTableGet, sizeof(logicalPortMappingTableGet));

        /* set full table ... then read full table .. check entries not override each other */
        for(; i<= LTM_MAX_LOGICAL_DEV_NUM_FOR_VPLS_CNS; i++)
        {
            for(j=0; j<= LTM_MAX_LOGICAL_PORT_NUM_FOR_VPLS_CNS; j++)
            {
                hwDev = (GT_U8)i;
                hwPort = (GT_U8)j;

                if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(i) && (i & 1))
                {
                    hwDev  &= ~1;/* clear LSBits --> make it even   */
                    hwPort += 64;/* jump to second hemisphere       */
                }

                logicalPortMappingTable.outputInterface.physicalInterface.devPort.hwDevNum    = hwDev;
                logicalPortMappingTable.outputInterface.physicalInterface.devPort.portNum   = hwPort;
                st = cpssDxChLogicalTargetMappingTableEntrySet(dev, hwDev, hwPort, &logicalPortMappingTable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, hwDev, hwPort);
            }
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(dev) == GT_TRUE)
            i = 0;
        else
            i = LTM_XCAT_MIN_LOGICAL_DEV_NUM_CNS;

        for(; i<= LTM_MAX_LOGICAL_DEV_NUM_FOR_VPLS_CNS; i++)
        {
            for(j=0; j<= LTM_MAX_LOGICAL_PORT_NUM_FOR_VPLS_CNS; j++)
            {
                hwDev = (GT_U8)i;
                hwPort = (GT_U8)j;

                if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(i) && (i & 1))
                {
                    hwDev  &= ~1;/* clear LSBits --> make it even   */
                    hwPort += 64;/* jump to second hemisphere       */
                }

                st = cpssDxChLogicalTargetMappingTableEntryGet(dev, hwDev, hwPort, &logicalPortMappingTable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, i, j);

                if(numErrors < 10)
                {
                    if(logicalPortMappingTable.outputInterface.physicalInterface.devPort.hwDevNum != hwDev ||
                       logicalPortMappingTable.outputInterface.physicalInterface.devPort.portNum  != hwPort)
                    {
                        PRV_UTF_LOG4_MAC("[UTF]: mismatch : error in index : {dev[%d],port[%d]} ---- read data from entry : {dev[%d],port[%d]} \n",
                                         hwDev,hwPort,
                                         logicalPortMappingTable.outputInterface.physicalInterface.devPort.hwDevNum,
                                         logicalPortMappingTable.outputInterface.physicalInterface.devPort.portNum);
                        numErrors ++;
                    }
                }
            }
        }

        if(numErrors)
        {
            PRV_UTF_LOG1_MAC("[UTF]: numErrors : %d \n",numErrors);
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, numErrors);
        }

        logicalPortNum = LTM_TESTED_PORT_NUM_CNS;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                              logicalPortNum, &logicalPortMappingTable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingTableEntryGet
(
    IN GT_U8                            devNum,
    IN GT_U8                            logicalDevNum,
    IN GT_U8                            logicalPortNum,
    OUT CPSS_DXCH_OUTPUT_INTERFACE_STC  *logicalPortMappingTablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingTableEntryGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                   logicalPortNum [LTM_TESTED_PORT_NUM_CNS]
               and logicalPortMappingTable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingTableEntryGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong logicalPortNum [LTM_INVALID_PORT_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong logicalPortMappingTable pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U8       logicalDevNum = LTM_LOGICAL_DEV_NUM_CNS;
    GT_U8       logicalPortNum = LTM_TESTED_PORT_NUM_CNS;
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  logicalPortMappingTable;

    cpssOsBzero((GT_VOID*)&logicalPortMappingTable, sizeof(logicalPortMappingTable));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with logicalDevNum [LTM_LOGICAL_DEV_NUM_CNS],
                           logicalPortNum [LTM_TESTED_PORT_NUM_CNS].
        */

        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                         logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, logicalPortNum);

        /*
            1.2. Call with wrong logicalDevNum [LTM_INVALID_LOGICAL_DEV_NUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        logicalDevNum  = LTM_INVALID_LOGICAL_DEV_NUM_CNS;

        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                                logicalPortNum, &logicalPortMappingTable);
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, logicalDevNum, logicalPortNum);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, logicalDevNum, logicalPortNum);
        }

        logicalDevNum  = LTM_LOGICAL_DEV_NUM_CNS;

        /*
            1.3. Call with wrong logicalPortNum [LTM_INVALID_PORT_NUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        logicalPortNum = LTM_INVALID_PORT_NUM_CNS;

        st = cpssDxChLogicalTargetMappingTableEntrySet(dev, logicalDevNum,
                                                  logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, logicalDevNum, logicalPortNum);

        logicalPortNum = LTM_TESTED_PORT_NUM_CNS;

        /*
            1.4. Call with wrong logicalPortMappingTable pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum, logicalPortNum, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, logicalDevNum, logicalPortNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                          logicalPortNum, &logicalPortMappingTable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingTableEntryGet(dev, logicalDevNum,
                                          logicalPortNum, &logicalPortMappingTable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet
(
    IN GT_U8                                                                  devNum,
    IN CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with mode[CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_4K_VID_32_LP_E,
                        CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_512_VID_256_LP_E,
                        CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_128_VID_1K_LP_E,
                        CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_64_VID_2K_LP_E].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range mode and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                                           st      = GT_OK;
    GT_U8                                                               dev     = 0;
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT mode    = 0;
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT modeRet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with mode[CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_4K_VID_32_LP_E,
                                    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_512_VID_256_LP_E,
                                    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_128_VID_1K_LP_E,
                                    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_64_VID_2K_LP_E].
                Expected: GT_OK.
            */

            /* call with mode[CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_4K_VID_32_LP_E] */
            mode = CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_4K_VID_32_LP_E;
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, &modeRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                             "got another mode: %d",
                                             modeRet);
            }

            /* call with mode[CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_512_VID_256_LP_E] */
            mode = CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_512_VID_256_LP_E;
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, &modeRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                             "got another mode: %d",
                                             modeRet);
            }

            /* call with mode[CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_128_VID_1K_LP_E] */
            mode = CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_128_VID_1K_LP_E;
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, &modeRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                             "got another mode: %d",
                                             modeRet);
            }

            /* call with mode[CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_64_VID_2K_LP_E] */
            mode = CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_64_VID_2K_LP_E;
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, &modeRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                             "got another mode: %d",
                                             modeRet);
            }

            /*
                1.3. Call with out of range mode and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet
                                (dev, mode),
                                mode);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet
(
    IN  GT_U8                                                                devNum,
    OUT CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with out of range modePtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                                           st      = GT_OK;
    GT_U8                                                               dev     = 0;
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT mode    = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with non-null modePtr.
                Expected: GT_OK.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, &mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet
(
    IN GT_U8    devNum,
    IN GT_U32   counter
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with counter[0, 0xAA55, 0xFFFFFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet.
    Expected: GT_OK and the same values as was set.

*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_U32                          counter     = 0;
    GT_U32                          counterRet  = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with counter[0, 0xAA55, 0xFFFFFFFF].
                Expected: GT_OK.
            */

            /* call with counter[0] */
            counter = 0;
            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet(dev, counter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet(dev, &counterRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(counter, counterRet,
                                             "got another counter: %d",
                                             counterRet);
            }

            /* call with counter[0xAA55] */
            counter = 0xAA55;
            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet(dev, counter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet(dev, &counterRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(counter, counterRet,
                                             "got another counter: %d",
                                             counterRet);
            }

            /* call with counter[0xFFFFFFFF] */
            counter = 0xFFFFFFFF;
            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet(dev, counter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet(dev, &counterRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(counter, counterRet,
                                             "got another counter: %d",
                                             counterRet);
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet(dev, counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet(dev, counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *counterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with non-null counterPtr.
    Expected: GT_OK.
    1.2. Call with out of range counterPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_U32                          counter     = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with non-null counterPtr.
                Expected: GT_OK.
            */

            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet(dev, &counter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call with out of range counterPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet(dev, &counter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet(dev, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet(dev, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressMappingEntrySet
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN GT_U32                       vlanIdIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressMappingEntrySet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with vlanId[0, 1024, 4095],
                   vlanIdIndex[0,1024, 4095].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingEgressMappingEntryGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range vlanId[4096] and other valid params.
    Expected: NON GT_BAD_PARAM.
    1.4. Call with out of range vlanIdIndex[4096] and other valid params.
    Expected: NON GT_BAD_PARAM.
*/

    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    GT_U16                              vlanId          = 0;
    GT_U32                              vlanIdIndex     = 0;
    GT_U32                              vlanIdIndexRet  = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with vlanId[0, 1024, 4095],
                               vlanIdIndex[0,1024, 4095].
                Expected: GT_OK.
            */

            /* call with vlanId[0] */
            vlanId = 0;
            vlanIdIndex = 0;
            st = cpssDxChLogicalTargetMappingEgressMappingEntrySet(dev, vlanId, vlanIdIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressMappingEntryGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndexRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(vlanIdIndex, vlanIdIndexRet,
                                             "got another vlanIdIndex: %d",
                                             vlanIdIndexRet);
            }

            /* call with vlanId[1024] */
            vlanId = 1024;
            vlanIdIndex = 1024;
            st = cpssDxChLogicalTargetMappingEgressMappingEntrySet(dev, vlanId, vlanIdIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressMappingEntryGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndexRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(vlanIdIndex, vlanIdIndexRet,
                                             "got another vlanIdIndex: %d",
                                             vlanIdIndexRet);
            }

            /* call with vlanId[4095] */
            vlanId = 4095;
            vlanIdIndex = 4095;
            st = cpssDxChLogicalTargetMappingEgressMappingEntrySet(dev, vlanId, vlanIdIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressMappingEntryGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndexRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(vlanIdIndex, vlanIdIndexRet,
                                             "got another vlanIdIndex: %d",
                                             vlanIdIndexRet);
            }

            /*
                1.3. Call with out of range vlanId[4096] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            vlanId = 4096;
            st = cpssDxChLogicalTargetMappingEgressMappingEntrySet(dev, vlanId, vlanIdIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                             "cpssDxChLogicalTargetMappingEgressMappingEntrySet: %d", dev);

            /* restore values */
            vlanId = 4095;

            /*
                1.4. Call with out of range vlanIdIndex[4096] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            vlanIdIndex = 4096;
            st = cpssDxChLogicalTargetMappingEgressMappingEntrySet(dev, vlanId, vlanIdIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                             "cpssDxChLogicalTargetMappingEgressMappingEntrySet: %d", dev);

            /* restore values */
            vlanIdIndex = 4095;

        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressMappingEntrySet(dev, vlanId, vlanIdIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEgressMappingEntrySet(dev, vlanId, vlanIdIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressMappingEntryGet
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    OUT GT_U32                      *vlanIdIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressMappingEntryGet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with vlanId[0, 1024, 4095], non-null vlanIndexPtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanId[4096] and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range vlanIdIndexPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    GT_U16                              vlanId          = 0;
    GT_U32                              vlanIdIndex     = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with vlanId[0, 1024, 4095], non-null vlanIndexPtr.
                Expected: GT_OK.
            */

            /* call with vlanId[0] */
            vlanId = 0;
            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with vlanId[1024] */
            vlanId = 1024;
            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with vlanId[4095] */
            vlanId = 4095;
            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /*
                1.2. Call with out of range vlanId[4096] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            vlanId = 4096;

            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            vlanId = 4095;

            /*
                1.3. Call with out of range vlanIdIndexPtr[NULL] and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEgressMappingEntryGet(dev, vlanId, &vlanIdIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressMemberSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       vlanIdIndex,
    IN GT_U8                        logicalDevNum,
    IN GT_U8                        logicalPortNum,
    IN GT_BOOL                      isMember
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressMemberSet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with vlanIdIndex[0, 2048, 4095],
                   logicalDevNum[0, 15, 31],
                   logicalPortNum[0, 31, 63],
                   isMember[GT_TRUE, GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChLogicalTargetMappingEgressMemberGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range vlanIdIndex[4096] and other valid params.
    Expected: NON GT_BAD_PARAM.
    1.4. Call with out of range logicalDevNum[32] and other valid params.
    Expected: NON GT_BAD_PARAM.
    1.5. Call with out of range logicalPortNum[64] and other valid params.
    Expected: NON GT_BAD_PARAM.
*/

    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    GT_U32                              vlanIdIndex     = 0;
    GT_U8                               logicalDevNum   = 0;
    GT_U8                               logicalPortNum  = 0;
    GT_BOOL                             isMember        = GT_FALSE;
    GT_BOOL                             isMemberRet     = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with vlanIdIndex[0, 2048, 4095],
                               logicalDevNum[0, 15, 31],
                               logicalPortNum[0, 31, 63],
                               isMember[GT_TRUE, GT_FALSE, GT_TRUE].
                Expected: GT_OK.
            */

            /* call with vlanIdIndex[0] */
            vlanIdIndex = 0;
            logicalDevNum = 0;
            logicalPortNum = 0;
            isMember = GT_TRUE;
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_4K_VID_32_LP_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                             logicalDevNum,
                                                             logicalPortNum,
                                                             isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressMemberGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMemberRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(isMember, isMemberRet,
                                             "got another isMember: %d",
                                             isMemberRet);
            }

            /* call with vlanIdIndex[2048] */
            vlanIdIndex = 2048;
            logicalDevNum = 15;
            logicalPortNum = 31;
            isMember = GT_FALSE;
            st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                             logicalDevNum,
                                                             logicalPortNum,
                                                             isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressMemberGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMemberRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(isMember, isMemberRet,
                                             "got another isMember: %d",
                                             isMemberRet);
            }

            /* call with vlanIdIndex[4095] */
            vlanIdIndex = 4095;
            logicalDevNum = 31;
            logicalPortNum = 63;
            isMember = GT_TRUE;
            st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                             logicalDevNum,
                                                             logicalPortNum,
                                                             isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChLogicalTargetMappingEgressMemberGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMemberRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(isMember, isMemberRet,
                                             "got another isMember: %d",
                                             isMemberRet);
            }

            /*
                1.3. Call with out of range vlanIdIndex[4096] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            vlanIdIndex = 4096;
            st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                             logicalDevNum,
                                                             logicalPortNum,
                                                             isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            vlanIdIndex = 4095;

            /*
                1.4. Call with out of range logicalDevNum[32] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            logicalDevNum = 32;
            st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                             logicalDevNum,
                                                             logicalPortNum,
                                                             isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            logicalDevNum = 31;

            /*
                1.5. Call with out of range logicalPortNum[64] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            logicalPortNum = 64;
            st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                             logicalDevNum,
                                                             logicalPortNum,
                                                             isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            logicalPortNum = 63;
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                         logicalDevNum,
                                                         logicalPortNum,
                                                         isMember);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLogicalTargetMappingEgressMemberSet(dev, vlanIdIndex,
                                                     logicalDevNum,
                                                     logicalPortNum,
                                                     isMember);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressMemberGet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       vlanIdIndex,
    IN  GT_U8                        logicalDevNum,
    IN  GT_U8                        logicalPortNum,
    OUT GT_BOOL                     *isMemberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressMemberGet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with vlanIdIndex[0, 2048, 4095],
                   logicalDevNum[0, 15, 31],
                   logicalPortNum[0, 31, 63],
                   non-null isMemberPtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanIdIndex[4096] and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range logicalDevNum[32] and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range logicalPortNum[64] and other valid params.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range isMemberPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    GT_U32                              vlanIdIndex     = 0;
    GT_U8                               logicalDevNum   = 0;
    GT_U8                               logicalPortNum  = 0;
    GT_BOOL                             isMember        = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with vlanIdIndex[0, 2048, 4095],
                               logicalDevNum[0, 15, 31],
                               logicalPortNum[0, 31, 63],
                               isMember[GT_TRUE, GT_FALSE, GT_TRUE].
                Expected: GT_OK.
            */

            /* call with vlanIdIndex[0] */
            vlanIdIndex = 0;
            logicalDevNum = 0;
            logicalPortNum = 0;
            st = cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet(dev, CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_4K_VID_32_LP_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with vlanIdIndex[2048] */
            vlanIdIndex = 2048;
            logicalDevNum = 15;
            logicalPortNum = 31;
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with vlanIdIndex[4095] */
            vlanIdIndex = 4095;
            logicalDevNum = 31;
            logicalPortNum = 63;
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /*
                1.2. Call with out of range vlanIdIndex[4096] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            vlanIdIndex = 4096;
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            vlanIdIndex = 4095;

            /*
                1.3. Call with out of range logicalDevNum[32] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            logicalDevNum = 32;
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            logicalDevNum = 31;

            /*
                1.4. Call with out of range logicalPortNum[64] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            logicalPortNum = 64;
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   &isMember);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            logicalPortNum = 63;

            /*
                1.5. Call with out of range isMemberPtr[NULL] and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                                   logicalDevNum,
                                                                   logicalPortNum,
                                                                   NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                               logicalDevNum,
                                                               logicalPortNum,
                                                               &isMember);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLogicalTargetMappingEgressMemberGet(dev, vlanIdIndex,
                                                           logicalDevNum,
                                                           logicalPortNum,
                                                           &isMember);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLogicalTargetMappingEgressMemberTableClear
(
    IN GT_U8                        devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChLogicalTargetMappingEgressMemberTableClear)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with valid parameters.
    Expected: GT_OK.
*/
    GT_STATUS               st      = GT_OK;
    GT_U8                   dev     = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with valid parameters.
                Expected: GT_OK.
            */
            st = cpssDxChLogicalTargetMappingEgressMemberTableClear(dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLogicalTargetMappingEgressMemberTableClear(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLogicalTargetMappingEgressMemberTableClear(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChLogicalTargetMapping suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChLogicalTargetMapping)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingDeviceEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingDeviceEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressMappingEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressMappingEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressMemberSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressMemberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLogicalTargetMappingEgressMemberTableClear)

UTF_SUIT_END_TESTS_MAC(cpssDxChLogicalTargetMapping)

