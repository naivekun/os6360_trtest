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
* @file cpssDxChProtectionUT.c
*
* @brief Unit tests for cpssDxChProtection, that provides CPSS implementation for
* Protection Switching.
*
* @version   2
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/protection/cpssDxChProtection.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/* Maximal LOC table index */
#define PROTECTION_MAX_LOC_TABLE_INDEX_CNS(_dev) \
    (PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.parametericTables.numEntriesProtectionLoc - 1)

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChProtectionEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_U32                                  notAppFamilyBmp;
    GT_BOOL                                 enable      = GT_FALSE;
    GT_BOOL                                 enableRet   = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChProtectionEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChProtectionEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChProtectionEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChProtectionEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChProtectionEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChProtectionEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChProtectionEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                  notAppFamilyBmp;
    GT_BOOL                                 enable          = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChProtectionEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChProtectionEnableGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChProtectionEnableGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChProtectionEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionTxEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionTxEnableSet)
{
/*
    ITERATE_DEVICES_EPORTS (Bobcat2, Caelum, Bobcat3)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChProtectionTxEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_BOOL             enable;
    GT_BOOL             enableGet;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                Call with enable [GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

            /*
                Call cpssDxChProtectionTxEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChProtectionTxEnableGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", devNum);

            /*
                Call with enable [GT_FALSE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

            /*
                Call cpssDxChProtectionTxEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChProtectionTxEnableGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", devNum);
        }

        portNum = 0;
        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* Call function for each non-active port */
            st = cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionTxEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionTxEnableGet)
{
/*
    ITERATE_DEVICES_EPORTS (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_BOOL             enable;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1. Call function with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &enable);

            /*
                1.2. Call function with NULL enablePtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChProtectionTxEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                "%d, enablePtr = NULL", devNum);
        }

        portNum = 0;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionTxEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionPortToLocMappingSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      locTableIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionPortToLocMappingSet)
{
/*
    ITERATE_DEVICES_EPORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with locTableIndex [0 / 1000 / 2047].
    Expected: GT_OK.
    1.1.2. Call cpssDxChProtectionPortToLocMappingGet.
    Expected: GT_OK and the same locTableIndex.
    1.1.3 Call with out of range locTableIndex and other valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_U32              locTableIndex;
    GT_U32              locTableIndexGet;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with locTableIndex [0 / 1000 / 2047].
                Expected: GT_OK.
            */
            locTableIndex = 0;

            st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, locTableIndex);

            /*
                1.1.2. Call cpssDxChProtectionPortToLocMappingGet.
                Expected: GT_OK and the same locTableIndex.
            */
            st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChProtectionPortToLocMappingSet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(locTableIndex, locTableIndexGet,
                "get another locTableIndex than was set: %d", devNum);

            /* Call with locTableIndex [1000]. */
            locTableIndex = 1000;

            st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, locTableIndex);

            /*
                1.1.2. Call cpssDxChProtectionPortToLocMappingGet.
                Expected: GT_OK and the same locTableIndex.
            */
            st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChProtectionPortToLocMappingGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(locTableIndex, locTableIndexGet,
                "get another locTableIndex than was set: %d", devNum);

            /* Call with locTableIndex [max]. */
            locTableIndex = PROTECTION_MAX_LOC_TABLE_INDEX_CNS(devNum);

            st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, locTableIndex);

            /*
                1.1.2. Call cpssDxChProtectionPortToLocMappingGet.
                Expected: GT_OK and the same locTableIndex.
            */
            st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChProtectionPortToLocMappingGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(locTableIndex, locTableIndexGet,
                "get another locTableIndex than was set: %d", devNum);
            /*
                1.1.3 Call with out of range locTableIndex and other valid params.
                Expected: NOT GT_OK.
            */
            locTableIndex = PROTECTION_MAX_LOC_TABLE_INDEX_CNS(devNum) + 1;

            st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum, locTableIndex);
        }

        locTableIndex = 0;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        locTableIndex);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    locTableIndex = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionPortToLocMappingGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_U32      *locTableIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionPortToLocMappingGet)
{
/*
    ITERATE_DEVICES_EPORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL locTableIndexPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL locTableIndexPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_U32              locTableIndex;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1. Call function with not NULL locTableIndexPtr.
                Expected: GT_OK.
            */
            st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &locTableIndex);

            /*
                1.2. Call function with NULL locTableIndexPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, locTableIndexPtr = NULL", devNum, portNum);
        }

        portNum = 0;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        locTableIndex);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndex);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionPortToLocMappingGet(devNum, portNum, &locTableIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionLocStatusSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  index,
    IN  CPSS_DXCH_PROTECTION_LOC_STATUS_ENT     status
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionLocStatusSet)
{
/*
    ITERATE_DEVICES_EPORTS (Bobcat2, Caelum, Bobcat3)
    - Call with locTableIndex [0 / 1000 / 2047].
      Expected: GT_OK
    - Call cpssDxChProtectionLocStatusGet.
      Expected: GT_OK and the same locTableIndex.
    - Call with wrong locTableIndex and other valid params.
      Expected: GT_BAD_PARAM
    - Call with locStatus [CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E/ CPSS_DXCH_PROTECTION_LOC_DETECTED_E].
      Expected: GT_OK.
    - Call with wrong locStatus and other valid params.
      Expected: GT_BAD_PARAM
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_U32              locTableIndex;
    CPSS_DXCH_PROTECTION_LOC_STATUS_ENT locStatus;
    CPSS_DXCH_PROTECTION_LOC_STATUS_ENT locStatusGet;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Call with locTableIndex [0]. */
        locTableIndex = 0;
        locStatus = CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E;

        st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, locTableIndex, locStatus);

        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &locStatusGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChProtectionLocStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(locStatus, locStatusGet,
            "get another locStatus than was set: %d", devNum);

        /* Call with locTableIndex [1000]. */
        locTableIndex = 1000;

        st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, locTableIndex, locStatus);

        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &locStatusGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChProtectionLocStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(locStatus, locStatusGet,
            "get another locStatus than was set: %d", devNum);

        /* Call with locTableIndex [max]. */
        locTableIndex = PROTECTION_MAX_LOC_TABLE_INDEX_CNS(devNum);

        st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, locTableIndex, locStatus);

        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &locStatusGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChProtectionLocStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(locStatus, locStatusGet,
            "get another locStatus than was set: %d", devNum);

        /* Call with illegal locTableIndex. */
        locTableIndex = PROTECTION_MAX_LOC_TABLE_INDEX_CNS(devNum) + 1;

        st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, locTableIndex, locStatus);

        locTableIndex = 0;
        locStatus = CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E;
        st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, locTableIndex, locStatus);

        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &locStatusGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChProtectionLocStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(locStatus, locStatusGet,
            "get another locStatus than was set: %d", devNum);

        locStatus = CPSS_DXCH_PROTECTION_LOC_DETECTED_E;
        st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, locTableIndex, locStatus);

        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &locStatusGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChProtectionLocStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(locStatus, locStatusGet,
            "get another locStatus than was set: %d", devNum);

        /* check wrong enum values */
        UTF_ENUMS_CHECK_MAC(cpssDxChProtectionLocStatusSet
                            (devNum, locTableIndex, locStatus),
                            locStatus);
    }

    locTableIndex = 0;
    locStatus = CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E;

    /* 2. For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionLocStatusSet(devNum, locTableIndex, locStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionLocStatusGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  index,
    OUT CPSS_DXCH_PROTECTION_LOC_STATUS_ENT     *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionLocStatusGet)
{
/*
    ITERATE_DEVICES_EPORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL statusPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL statusPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_U32              locTableIndex = 0;
    CPSS_DXCH_PROTECTION_LOC_STATUS_ENT     status;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL status.
            Expected: GT_OK.
        */
        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, locTableIndex, &status);

        /*
            1.2. Call function with NULL status.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
            "%d, %d, status = NULL", devNum, locTableIndex);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionLocStatusGet(devNum, locTableIndex, &status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionRxExceptionPacketCommandSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PACKET_CMD_ENT     command
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionRxExceptionPacketCommandSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    - Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                          CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                          CPSS_PACKET_CMD_DROP_HARD_E /
                          CPSS_PACKET_CMD_DROP_SOFT_E].
      Expected: GT_OK.
    - Call cpssDxChProtectionRxExceptionPacketCommandGet with non NULL cmdPtr.
      Expected: GT_OK and the same cmd.
    - Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                        CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_E /
                                        CPSS_PACKET_CMD_NONE_E],
      Expected: GT_BAD_PARAM
    - Call with cmd [wrong enum values].
      Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       devNum  = 0;
    CPSS_PACKET_CMD_ENT  cmd    = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT  cmdGet = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                  CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                  CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                  CPSS_PACKET_CMD_DROP_HARD_E /
                                  CPSS_PACKET_CMD_DROP_SOFT_E].
            Expected: GT_OK.
        */
        for (cmd = CPSS_PACKET_CMD_FORWARD_E; cmd <= CPSS_PACKET_CMD_DROP_SOFT_E; cmd++)
        {
            st = cpssDxChProtectionRxExceptionPacketCommandSet(devNum, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, cmd);

            /*
                Call cpssDxChProtectionRxExceptionPacketCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChProtectionRxExceptionPacketCommandGet(devNum, &cmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChProtectionRxExceptionPacketCommandGet: %d", devNum);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d", devNum);
        }

        /*
            Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                                CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                                CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                                CPSS_PACKET_CMD_BRIDGE_E /
                                                CPSS_PACKET_CMD_NONE_E  /
                                                CPSS_PACKET_CMD_LOOPBACK_E /
                                                CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E],
            Expected: NON GT_OK.
        */
        for (cmd = CPSS_PACKET_CMD_ROUTE_E; cmd <= CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E; cmd++)
        {
            st = cpssDxChProtectionRxExceptionPacketCommandSet(devNum, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, cmd);
        }

        cmd = CPSS_PACKET_CMD_FORWARD_E;

        /*
            Call with cmd [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChProtectionRxExceptionPacketCommandSet(devNum, cmd),
                            cmd);
    }

    cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChProtectionRxExceptionPacketCommandSet(devNum, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionRxExceptionPacketCommandSet(devNum, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionRxExceptionPacketCommandGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionRxExceptionPacketCommandGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL commandPtr.
    Expected: GT_OK.
    1.1.2. Call with commandPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;

    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not NULL commandPtr.
            Expected: GT_OK.
        */
        st = cpssDxChProtectionRxExceptionPacketCommandGet(dev, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with commandPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChProtectionRxExceptionPacketCommandGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, commandPtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChProtectionRxExceptionPacketCommandGet(dev, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionRxExceptionPacketCommandGet(dev, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionRxExceptionCpuCodeSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionRxExceptionCpuCodeSet)
{
/*
    ITERATE_DEVICE
    1.1. Call cpssDxChProtectionRxExceptionCpuCodeSet with relevant
         cpuCode [CPSS_NET_CONTROL_E/
                  CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E/
                  CPSS_NET_LOCK_PORT_MIRROR_E].
    Expected: GT_OK
    1.2. Call cpssDxChProtectionRxExceptionCpuCodeGet.
    Expected: GT_OK and the same cpuCode
    1.4. Call with wrong enum value cpuCode.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_CONTROL_E;
    CPSS_NET_RX_CPU_CODE_ENT cpuCodeGet;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpuCode = CPSS_NET_CONTROL_E;

        st = cpssDxChProtectionRxExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;

        st = cpssDxChProtectionRxExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        cpuCode = CPSS_NET_LOCK_PORT_MIRROR_E;

        st = cpssDxChProtectionRxExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        cpuCode = CPSS_NET_LAST_USER_DEFINED_E;

        st = cpssDxChProtectionRxExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        UTF_ENUMS_CHECK_MAC(cpssDxChProtectionRxExceptionCpuCodeSet(dev, cpuCode),
                            cpuCode);
    }
    cpuCode = CPSS_NET_CONTROL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChProtectionRxExceptionCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionRxExceptionCpuCodeSet(dev, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChProtectionRxExceptionCpuCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChProtectionRxExceptionCpuCodeGet)
{
/*
    ITERATE_DEVICES
    - Call cpssDxChProtectionRxExceptionCpuCodeGet with not NULL cpuCodePtr.
      Expected: GT_OK.
    - Call with NULL cpuCodePtr.
      Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_CONTROL_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChProtectionRxExceptionCpuCodeGet(dev, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChProtection suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChProtection)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionTxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionTxEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionPortToLocMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionPortToLocMappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionLocStatusSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionLocStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionRxExceptionPacketCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionRxExceptionPacketCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionRxExceptionCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChProtectionRxExceptionCpuCodeGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChProtection)


