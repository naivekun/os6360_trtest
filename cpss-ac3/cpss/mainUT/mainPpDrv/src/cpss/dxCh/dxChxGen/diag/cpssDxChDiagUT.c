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
* @file cpssDxChDiagUT.c
*
* @brief Unit tests for cpssDxChDiagUT, that provides
* CPSS DXCH Diagnostic API
*
* @version   66
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/generic/diag/private/prvCpssGenDiag.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/* Valid port num value used for testing */
#define DIAG_VALID_PORTNUM_CNS       0

/* Invalid lane num used for testing */
#define DIAG_INVALID_LANENUM_CNS     4


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagAllMemTest
(
    IN GT_U8                     devNum,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *addrPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagAllMemTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with addrPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with readValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with writeValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     testStatus;
    GT_U32      addr;
    GT_U32      readVal;
    GT_U32      writeVal;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT3_E, "JIRA-6750");

    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, NULL, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with addrPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with readValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, NULL, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with writeValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        if(!prvUtfIsGmCompilation())
        {
            /* after messing with the memories ... synch shadow to HW state */
            prvCpssDxChDiagDataIntegrityShadowAndHwSynch(dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagAllRegTest
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *testStatusPtr,
    OUT GT_U32  *badRegPtr,
    OUT GT_U32  *readValPtr,
    OUT GT_U32  *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagAllRegTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with badRegPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with readValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with writeValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     testStatus;
    GT_U32      badReg   = 0;
    GT_U32      readVal  = 0;
    GT_U32      writeVal = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
        if((st != GT_OK) || (testStatus != GT_TRUE))
        {
            PRV_UTF_LOG3_MAC("cpssDxChDiagAllRegTest: FAILED on register address[0x%8.8x] with writeVal[0x%8.8x] and readVal[0x%8.8x] \n",
                badReg,readVal,writeVal);
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, testStatus, dev);

        /*
            1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, NULL, &badReg, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with badRegPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with readValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, NULL, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with writeValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemRead
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemRead)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                               CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                  offset [0], and non-NULL dataPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values memType  and
         other params same as in 1.1.
    Expected: not GT_OK.
    1.3. Call with dataPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
    GT_U32                     offset = 0;
    GT_U32                     data;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Call with memType = [  CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                        CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                          offset [0], and non-NULL dataPtr.
            Expected: GT_OK.
        */
        memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;

        st = cpssDxChDiagMemRead(dev, memType, offset, &data);
        if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily ||
            UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Lion2 does not have CPU access to buffer memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);


        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;

        st = cpssDxChDiagMemRead(dev, memType, offset, &data);
        if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
            (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
        {
            /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*
            1.2. Call with wrong enum values memType and other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemRead
                            (dev, memType, offset, &data),
                            memType);

        /*
            1.3. Call with dataPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagMemRead(dev, memType, offset, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemRead(dev, memType, offset, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemRead(dev, memType, offset, &data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemTest
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         startOffset,
    IN GT_U32                         size,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *addrPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with  memType [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                             CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                        startOffset [0],
                        size [_8K],
                        profile [CPSS_DIAG_TEST_RANDOM_E]
                    and non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with wrong enum values memType and other params same as in 1.1.
    Expected: not GT_OK.
    1.3. Call with wrong enum values profile and other params same as in 1.1.
    Expected: not GT_OK.
    1.4. Call with testStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with addrPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with readValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call with writeValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
    GT_U32                     startOffset = 0;
    GT_U32                     size = 0;
    CPSS_DIAG_TEST_PROFILE_ENT profile = CPSS_DIAG_TEST_RANDOM_E;
    GT_BOOL                    testStatus;
    GT_U32                     addr;
    GT_U32                     readVal;
    GT_U32                     writeVal;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);
        /*
            1.1. Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E,
                                       CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E /
                                       CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                          offset [0], and non-NULL dataPtr.
            Expected: GT_OK.
        */

        /*call with memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E*/
        memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
        size = _8K;

        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Lion2 does not have CPU access to buffer memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);


        /*call with memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E*/
        memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E;

        /* cheetah has entry size 12 bytes. Need to align tested size with 12 bytes */
        size = (CPSS_PP_FAMILY_CHEETAH_E == devFamily)? 4092 : _4K;

        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*call with memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E*/
        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
        size = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;

        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
            (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
        {
            /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*
            1.2. Call with wrong enum values memType and other params same as in 1.1.
            Expected: not GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemTest
                            (dev, memType, startOffset, size, profile,
                             &testStatus, &addr, &readVal, &writeVal),
                            memType);

        /*
            1.3. Call with wrong enum values profile and other params same as in 1.1.
            Expected: not GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemTest
                            (dev, memType, startOffset, size, profile,
                             &testStatus, &addr, &readVal, &writeVal),
                            profile);

        /*
            1.4. Call with testStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 NULL, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with addrPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.6. Call with readValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, NULL, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.7. Call with writeValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        if(!prvUtfIsGmCompilation())
        {
            /* after messing with the memories ... synch shadow to HW state */
            prvCpssDxChDiagDataIntegrityShadowAndHwSynch(dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                             &testStatus, &addr, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemWrite
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                               CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                  offset [0] and data [0].
    Expected: GT_OK.
    1.2. Call with wrong enum values memType  and
         other params same as in 1.1.
    Expected: not GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
    GT_U32                     offset = 0;
    GT_U32                     data = 0;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);
        /*
            1.1. Call with memType = [  CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                        CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                  offset [0] and data[0].
            Expected: GT_OK.
        */
        memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;

        st = cpssDxChDiagMemWrite(dev, memType, offset, data);
        if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Lion2 does not have CPU access to buffer memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;

        st = cpssDxChDiagMemWrite(dev, memType, offset, data);
        if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
            (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
        {
            /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*
            1.2. Call with wrong enum values memType  and other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemWrite
                            (dev, memType, offset, data),
                            memType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemWrite(dev, memType, offset, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemWrite(dev, memType, offset, data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortCheckEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortCheckEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsPortCheckEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }

            }

            /* iterate with enable - GT_FALSE */
            enable = GT_FALSE;
            st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortCheckEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortCheckEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);

                    /* iterate with enable - GT_TRUE */
                    enable = GT_TRUE;

                    st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortCheckEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortCheckEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortCheckEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortCheckEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
            if(prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {

                /* the feature supported for all Gig ports
                   and XG ports of CH3 and above devices */
                if( IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType)) )
                {
                    if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                        {
                            if(IS_PORT_XG_E(portType))
                            {
                                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                        }
                        else
                        {
                            if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                            {
                                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                        }
                    }
                    else
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call enablePtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortCheckReadyGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *isReadyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortCheckReadyGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL isReady.
    Expected: GT_OK.
    1.1.2. Call isReadyPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;
    GT_BOOL     isReady    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if( IS_PORT_GE_E(portType) )
            {
                /*
                    1.1.1. Call with not-NULL isReady.
                    Expected: GT_OK.
                */
                st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call isReadyPtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, isReadyPtr = NULL", dev, port);
                }
            }
            else
            {
                /* not supported not GE_E ports*/
                st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, isReady);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortGenerateEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsPortGenerateEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }

            /* iterate with enable - GT_FALSE */
            enable = GT_FALSE;
            st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortGenerateEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortGenerateEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);

                    /* iterate with enable - GT_TRUE */
                    enable = GT_TRUE;

                    st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortGenerateEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortGenerateEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);

                }

            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortGenerateEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }

            st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call enablePtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortStatusGet
(
    IN   GT_U8          devNum,
    IN   GT_U8          portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *checkerLockedPtr,
    OUT  GT_U32         *errorCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call checkerLockedPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call errorCntrPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     checkerLocked;
    GT_U32      errorCntr;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL pointers.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                     notSupported = GT_TRUE;
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }
            st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call checkerLockedPtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, NULL, &errorCntr);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, checkerLockedPtr = NULL", dev, port);

                    /*
                        1.1.2. Call errorCntr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, errorCntr = NULL", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, checkerLocked);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortTransmitModeSet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortTransmitModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsPortTransmitModeGet with not-NULL modePtr.
    Expected: GT_OK and the same mode as was set.
    1.1.3. Call with wrong enum values mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT modeGet = 1;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E].
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }

            mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E;

            st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortTransmitModeGet with not-NULL modePtr.
                        Expected: GT_OK and the same mode as was set.
                    */
                    st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &modeGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortTransmitModeGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                                   "got another mode then was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }

            /*
                1.1.3. Call with wrong enum values mode.
                Expected: GT_BAD_PARAM.
            */

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( IS_PORT_GE_E(portType) ||
                (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType)) )
            {
                if (GT_FALSE == notSupported)
                {
                    UTF_ENUMS_CHECK_MAC(cpssDxChDiagPrbsPortTransmitModeSet
                                        (dev, port, lane, mode),
                                        mode);
                }
            }
        }

        mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortTransmitModeGet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortTransmitModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL mode.
    Expected: GT_OK.
    1.1.2. Call modePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL mode.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }
            st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call modePtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, modePtr = NULL", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegTest
(
    IN GT_U8                          devNum,
    IN GT_U32                         regAddr,
    IN GT_U32                         regMask,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call wrong enum values profile.
    Expected: GT_BAD_PARAM.
    1.1.3. Call testStatusPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call readValPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call writeValPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    GT_U32                         regAddr = 0;
    GT_U32                         regMask = 0;
    CPSS_DIAG_TEST_PROFILE_ENT     profile = CPSS_DIAG_TEST_RANDOM_E;
    GT_BOOL                        testStatus;
    GT_U32                         readVal = 0;
    GT_U32                         writeVal = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* register '0' not exists (make it 0 from MG unit) */
            regAddr = 0 + prvCpssDxChHwUnitBaseAddrGet(dev,PRV_CPSS_DXCH_UNIT_MG_E,NULL);
        }
        else
        {
            regAddr = 0;
        }

        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call wrong enum values profile.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagRegTest
                            (dev, regAddr, regMask, profile, &testStatus, &readVal, &writeVal),
                            profile);

        /*
            1.1.3. Call testStatusPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4. Call readValPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, NULL, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.5. Call writeValPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                             &testStatus, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegsDump)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call regAddrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call regDataPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    GT_U32    regsNum = 0;
    GT_U32    offset = 0;
    GT_U32    regAddr= 0;
    GT_U32    regData[1] = { 0 };


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsDump(dev, NULL, offset, &regAddr, regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3. Call regAddr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, NULL, regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4. Call regData[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, regData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, regData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegsNumGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_U32      regsNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsNumGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsCyclicDataSet
(
    IN   GT_U8          devNum,
    IN   GT_U8          portNum,
    IN   GT_U32         laneNum,
    IN   GT_U32         cyclicDataArr[4]
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsCyclicDataSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh3 and above)
    1.1.1. Call with correct params.
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsCyclicDataGet with the same params.
    Expected: GT_OK and the same cyclicDataArr as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_U8       i = 0;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    GT_U32      cyclicDataArr[4] = {0,1,2,3};
    GT_U32      cyclicDataArrGet[4] = {1,2,3,4};
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with correct params.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if (prvUtfPortMacModuloCalc(dev, port, 2))
                {
                     notSupported = GT_TRUE;
                }
            }


            st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);

            if( IS_PORT_XG_E(portType) && (GT_FALSE == notSupported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call cpssDxChDiagPrbsCyclicDataGet.
                    Expected: GT_OK and the same cyclicDataArr as was set.
                */
                st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArrGet);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChDiagPrbsCyclicDataGet: %d, %d", dev, port);

                for(i = 0; i < 4; i++)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(cyclicDataArr[i], cyclicDataArrGet[i],
                          "got another cyclicDataArr then was set: %d, %d", dev, port);
                }
            }
            else
            {
                /* not supported not XG_E ports*/
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsCyclicDataGet
(
    IN   GT_U8          devNum,
    IN   GT_U8          portNum,
    IN   GT_U32         laneNum,
    OUT  GT_U32         cyclicDataArr[4]
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsCyclicDataGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh3 and above)
    1.1.1. Call with correct params.
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;
    GT_U32      lane = 0;
    GT_U32      cyclicDataArr[4] = {0,1,2,3};
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6066);
        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with correct params.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if (prvUtfPortMacModuloCalc(dev, port, 2))
                {
                     notSupported = GT_TRUE;
                }
            }

            st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);

            if( IS_PORT_XG_E(portType) && (GT_FALSE == notSupported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                /* not supported other ports*/
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPhyRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPhyRegRead)
{
/*
    1.1.1. Call with data=NULL
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U32                          baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0;
    GT_U32                          smiRegOffset = 0;
    GT_U32                          phyAddr = 0;
    GT_U32                          offset = 0;
    GT_BOOL                         doByteSwap = GT_TRUE;

    st = cpssDxChDiagPhyRegRead(baseAddr, ifChannel, smiRegOffset,
                                  phyAddr, offset, NULL, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPhyRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPhyRegWrite)
{
/*
    1.1.1. Call with phyAddr = 33
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_BAD_PARAM;

    GT_U32                          baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0;
    GT_U32                          smiRegOffset = 0;
    GT_U32                          phyAddr = 33;
    GT_U32                          offset = 0;
    GT_U32                          data = 0;
    GT_BOOL                         doByteSwap = GT_TRUE;


    st = cpssDxChDiagPhyRegWrite(baseAddr, ifChannel, smiRegOffset,
                                  phyAddr, offset, data, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
*/
static void ut_cpssDxChDiagRegRead(IN GT_U8 dev)
{
/*
    1.1.1. Call with dataPtr == NULL
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_UINTPTR                      baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannelArr[] = {
            CPSS_CHANNEL_PCI_E,
            CPSS_CHANNEL_PEX_E,
            CPSS_CHANNEL_PEX_MBUS_E,

            CPSS_CHANNEL_LAST_E
            };
    CPSS_DIAG_PP_REG_TYPE_ENT       regType = 0;
    GT_U32                          offset = 0;
    GT_U32                          regsArray[] = {0,0,0,0xFFFFFFFF};
    GT_BOOL                         doByteSwap = GT_TRUE;
    GT_U32                          portGroupId = 0;
    GT_U32                          orig_regValue,regValueRead,newRegValueWrite;
    GT_U32                          ii,jj;
    GT_U32  index = 0;
    APP_DEMO_PP_CONFIG              *appDemoPtr = NULL;

    st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                             NULL, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    if (prvCpssDrvPpConfig[dev]->mngInterfaceType == CPSS_CHANNEL_PEX_KERNEL_E)
    {
        PRV_UTF_LOG1_MAC("PEX_KERNEL channel connects device %d to CPU\n", dev);
        PRV_UTF_LOG0_MAC("Test skipped \n");
        SKIP_TEST_MAC;
    }

    index = 0;
    appDemoPtr = &appDemoPpConfigList[0];
    /* find the AppDemo that match the dev */
    while(index < APP_DEMO_PP_CONFIG_SIZE_CNS)
    {
        if(appDemoPtr->devNum == dev)
        {
            break;
        }
        index++;
        appDemoPtr++;
    }

    if(index == APP_DEMO_PP_CONFIG_SIZE_CNS)
    {
        /* not found our device ?! */
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(APP_DEMO_PP_CONFIG_SIZE_CNS, index);
        return;
    }

    appDemoPtr = &appDemoPpConfigList[index];

    if(appDemoPtr->valid == GT_TRUE &&
       (CPSS_CHANNEL_PCI_E == appDemoPtr->channel ||
        CPSS_CHANNEL_PEX_E == appDemoPtr->channel ||
        CPSS_CHANNEL_PEX_MBUS_E == appDemoPtr->channel) )
    {
        CPSS_HW_INFO_STC *hwInfoPtr;

        PRV_UTF_LOG1_MAC("baseAddr [0x%8.8x] of device \n", baseAddr);

        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(CPSS_PARAM_NOT_USED_CNS, baseAddr);

        hwInfoPtr = cpssDrvHwPpHwInfoStcPtrGet(dev, portGroupId);
        if (!hwInfoPtr)
        {
            return;
        }

        baseAddr = hwInfoPtr->resource.switching.start;
        if(baseAddr == CPSS_PARAM_NOT_USED_CNS || baseAddr == 0)
        {
            return;
        }

        regType = CPSS_DIAG_PP_REG_INTERNAL_E;

        /* test PCI mode */
        ifChannel = CPSS_CHANNEL_PCI_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /* appDemo provides 4M window for SIP5 devices. Therefore only new address completion
               should be used for SIP5 and above devices - PEX_MBUS. */
            ifChannel = CPSS_CHANNEL_PEX_MBUS_E;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* register '0x50' not exists (make it 0 from MG unit) */
            offset = 0x50 + prvCpssDxChHwUnitBaseAddrGet(dev,PRV_CPSS_DXCH_UNIT_MG_E,NULL);
        }
        else
        {
            offset = 0x50;
        }

        PRV_UTF_LOG1_MAC("ifChannel = [0x%8.8x] \n", ifChannel);

        /* read register that hold the 0x11AB to understand the 'swap' value */
        doByteSwap = GT_FALSE;
        st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                 &regValueRead, doByteSwap);

        PRV_UTF_LOG1_MAC("regAddr = [0x%8.8x] \n", offset);
        PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);
        PRV_UTF_LOG1_MAC("regValueRead = [0x%8.8x] \n", regValueRead);
        PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
        if(regValueRead == 0x11AB)
        {
        }
        else
        if(regValueRead == 0xAB110000)
        {
            doByteSwap = GT_TRUE;
            st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                     &regValueRead, doByteSwap);
            PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);
            PRV_UTF_LOG1_MAC("regValueRead = [0x%8.8x] \n", regValueRead);
            PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
        }

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(0x11AB , regValueRead);

        /* registers taken from cpssDxChDiagAllRegTest */
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* the device not supports those SIP5 TXQ features */
            regsArray[0] = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(dev).deviceMapConfigs.localTrgPortMapOwnDevEn[0];
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->TXQ.sdq[0].global_config;
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->IPvX.IPv4GlobalCtrl.IPv4CtrlReg0;
        }
        else
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            regsArray[0] = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(dev).deviceMapConfigs.localTrgPortMapOwnDevEn[0];
            regsArray[1] = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(dev).scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg0[0];
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->IPvX.IPv4GlobalCtrl.IPv4CtrlReg0;
        }
        else
        if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
        {
            regsArray[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->txqVer1.egr.global.egressInterruptMask;
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->txqVer1.dq.shaper.tokenBucketUpdateRate;
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->ipRegs.routerGlobalReg;
        }
        else
        {
            regsArray[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->interrupts.txqIntMask;
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->bridgeRegs.egressFilter;
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->egrTxQConf.txPortRegs[0].wrrWeights0;
        }

        for(jj = 0 ; regsArray[jj] != 0xFFFFFFFF ; jj++)
        {
            offset = regsArray[jj];
            PRV_UTF_LOG1_MAC("offset = [0x%8.8x] \n", offset);

            /* read register by 'regular driver' API */
            cpssDrvPpHwRegisterRead(dev,portGroupId,offset, &orig_regValue);
            PRV_UTF_LOG1_MAC("orig_regValue = [0x%8.8x] \n", orig_regValue);

            newRegValueWrite = 0xAABBCCDD + offset;
            PRV_UTF_LOG1_MAC("(by driver) newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
            /* write register by 'regular driver' API */
            cpssDrvPpHwRegisterWrite(dev,portGroupId,offset, newRegValueWrite);

            PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);

            for(ii = 0 ; ifChannelArr[ii] != CPSS_CHANNEL_LAST_E ; ii++)
            {
                ifChannel = ifChannelArr[ii];

                if(ifChannel == CPSS_CHANNEL_PEX_MBUS_E &&
                    !PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    continue;
                }

                if ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
                {
                    /* appDemo provides 4M window for SIP5 devices. Therefore only new address completion
                       should be used for SIP5 and above devices - PEX_MBUS. */
                    if(ifChannel != CPSS_CHANNEL_PEX_MBUS_E)
                    {
                        continue;
                    }
                }

                PRV_UTF_LOG1_MAC("ifChannel = [0x%8.8x] \n", ifChannel);

                regValueRead = 0;
                st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                         &regValueRead, doByteSwap);
                PRV_UTF_LOG1_MAC("(first)regValueRead = [0x%8.8x] \n", regValueRead);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(newRegValueWrite , regValueRead);

                newRegValueWrite = 0xF55F11FF + offset + (ifChannel << 10);
                PRV_UTF_LOG1_MAC("(by diag)newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
                st = cpssDxChDiagRegWrite(baseAddr, ifChannel, regType, offset,
                                         newRegValueWrite, doByteSwap);
                PRV_UTF_LOG1_MAC("newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);

                regValueRead = 0;
                st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                         &regValueRead, doByteSwap);
                PRV_UTF_LOG1_MAC("(second)regValueRead = [0x%8.8x] \n", regValueRead);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(newRegValueWrite , regValueRead);
            }

            /* restore register by 'regular driver' API */
            cpssDrvPpHwRegisterWrite(dev,portGroupId,offset, orig_regValue);
        }

    }


}

UTF_TEST_CASE_MAC(cpssDxChDiagRegRead)
{
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        ut_cpssDxChDiagRegRead(dev);
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegWrite)
{
/*
    1.1.1. Call with ifChannel not valid
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_BAD_PARAM;

    GT_U32                          baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0x55555;
    CPSS_DIAG_PP_REG_TYPE_ENT       regType = 0;
    GT_U32                          offset = 0;
    GT_U32                          data = 0;
    GT_BOOL                         doByteSwap = GT_TRUE;


    st = cpssDxChDiagRegWrite(baseAddr, ifChannel, regType, offset, data, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

static void prvUtfDxChPortSerdesNumGet(GT_U8 dev, GT_PHYSICAL_PORT_NUM port, GT_U32 *serdesNum)
{
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    GT_STATUS st;
    GT_U32      startSerdes;
    GT_U32      maxLaneNum;

    st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                 dev, port);

    if(PRV_CPSS_SIP_5_CHECK_MAC(dev) ||
       PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E )
    {
        st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode, &startSerdes, &maxLaneNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet: %d, %d", dev, port);

        *serdesNum = maxLaneNum;

        return;
    }

    switch (ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
            if(((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)) &&
               (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(dev,port) == GT_TRUE))
            {   /* xCat Flex ports have 4 SERDESes even part of them are not used */
                *serdesNum = 4;
            }
            else
                *serdesNum = 1;
            break;
        case CPSS_PORT_INTERFACE_MODE_HX_E:
        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            *serdesNum = 2;
            break;
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
            *serdesNum = 4;
            break;
        case CPSS_PORT_INTERFACE_MODE_XLG_E:
            *serdesNum = 8;
            break;
        default:
            *serdesNum = 0;
            break;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableSet
    +
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeSet
    +
GT_STATUS cpssDxChDiagPrbsSerdesStatusGet
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesFunctionalityTest)
{
/*
    ITERATE_DEVICES_PHY_PORT (BobCat2)
    1.1.1. Configure SERDES loopback
    1.1.2. Configure SERDES PRBS transmit mode to MODE_PRBS7_E
    1.1.3. Enable SERDES PRBS feature
    1.1.4. Check status of RPBS
    Expected:   lock status = GT_TRUE
                pattern counter = not equal to 0
*/

    GT_U8       dev;

#ifndef ASIC_SIMULATION
    GT_STATUS   st   = GT_OK;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_U32      serdesNum;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL     checkerLocked = GT_FALSE;
    GT_U32      errorCntr = 0;
    GT_U64      patternCntr;

    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
    cpssOsMemSet(&patternCntr, 0, sizeof(patternCntr));
#endif

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;


#ifndef ASIC_SIMULATION
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                             dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d", dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            for (lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChPortSerdesLoopbackModeSet(dev, port, (1<<lane), CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);

                /* Transmit mode should be set before enabling test mode */
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);

                if (portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else /* FE port */
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                                     dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                    }
                }

                /* Enable PRBS feature */
                enable = GT_TRUE;
                st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);

                if (portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else /* FE port */
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }

                /* Check that status is "LOCKED" and pattern counter was incremented */
                st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                        &checkerLocked, &errorCntr, &patternCntr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                if(st == GT_OK)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, checkerLocked, dev, port);
                    if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
                    {
                        /* BobK does not support counter. It always 0. */
                        UTF_VERIFY_EQUAL2_PARAM_MAC(0, patternCntr.l[0], dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(0, patternCntr.l[0], dev, port);
                    }
                }

                /* Disable PRBS feature */
                enable = GT_FALSE;
                st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);

                if (portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else /* FE port */
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }

                /* Disable SERDES Loopback */
                st = cpssDxChPortSerdesLoopbackModeSet(dev, port, (1<<lane), CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);

            }
        }
    }
#endif
}


extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesStatusGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    IN   GT_U32    laneNum,
    OUT  GT_BOOL  *lockedPtr,
    OUT  GT_U32   *errorCntrPtr,
    OUT  GT_U64   *patternCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat and above)
    1.1.1. Call with laneNum [0 - 3] and not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK for flexLink ports and GT_OK for other.
    1.1.3. Call  witn wrong checkerLockedPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call witn wrong errorCntrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call witn wrong patternCntrPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;
    GT_U32      lane      = 0;
    GT_BOOL     checkerLocked = GT_FALSE;
    GT_U32      errorCntr = 0;
    GT_U64      patternCntr;
    GT_U32      serdesNum;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    PRV_CPSS_PORT_TYPE_ENT          portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL                         isFlex;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
    GT_U32                                      boardIdx;
    GT_U32                                      boardRevId;
    GT_U32                                      reloadEeprom;

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_XCAT3_E, "JIRA-6749");

    CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    cpssOsMemSet(&patternCntr, 0, sizeof(patternCntr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;
        if(prvUtfIsGmCompilation())
        {
            break;
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlex);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortIsFlexLinkGet: %d, %d",
                                         dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);

            /*
                1.1.1. Call with laneNum [0 - 3] and not-NULL pointers.
                Expected: GT_OK.
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                        &checkerLocked, &errorCntr, &patternCntr);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_INITIALIZED, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }

            }

            /*
                1.1.2. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
                Expected: NOT GT_OK for flexLink ports and GT_OK for other.
            */
            lane = serdesNum;
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);

            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else /*FE port */
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, lane);
            }

            lane = 0;

            /*
                1.1.3. Call  witn wrong checkerLockedPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                                 NULL, &errorCntr, &patternCntr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, checkerLockedPtr = NULL", dev, port);

            /*
                1.1.4. Call witn wrong errorCntrPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                                 &checkerLocked, NULL, &patternCntr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, errorCntr = NULL", dev, port);

            /*
                1.1.5. Call witn wrong patternCntr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                                 &checkerLocked, &errorCntr, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, patternCntr = NULL", dev, port);
        }

        lane = 0;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                        &checkerLocked, &errorCntr, &patternCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
    st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
    st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                   &checkerLocked, &errorCntr, &patternCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTestEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat and above)
    1.1.1. Call with laneNum [0 - 3] enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK for flexLink ports and GT_OK for other.
    1.1.3. Call cpssDxChDiagPrbsSerdesTestEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;
    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_U32      serdesNum;
    GT_BOOL     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT          portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;

    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        if(prvUtfIsGmCompilation())
        {
            break;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                /* Transmit mode should be set before enabling test mode */
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                }
                else /* FE port */
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }

                for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
                {
                    st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        if(portType != PRV_CPSS_PORT_FE_E)
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                        }
                        else /* FE port */
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                        }

                        /*
                            1.1.2. Call cpssDxChDiagPrbsSerdesTestEnableGet with not-NULL enablePtr.
                            Expected: GT_OK and the same enable as was set.
                        */
                        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enableGet);

                        if(portType != PRV_CPSS_PORT_FE_E)
                        {
                            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                        "cpssDxChDiagPrbsSerdesTestEnableGet: %d, %d", dev, port);
#ifndef ASIC_SIMULATION
                            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                           "got another enable then was set: %d, %d", dev, port);
#endif
                        }
                        else /* FE port */
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                        }
                    }
                }
            }
        }

        lane = 0;
        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    lane = 0;
    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTestEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat and above)
    1.1.1. Call with lane [0 - 3] and not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_U32      serdesNum;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;

CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        if(prvUtfIsGmCompilation())
        {
            break;
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with lane [0 - 3] and not-NULL enable.
                Expected: GT_OK.
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);

                /* the feature supported for all Gig ports
                   and XG ports of CH3 and above devices */
                if( IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType)) )
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                        /*
                            1.1.2. Call enablePtr[NULL].
                            Expected: GT_BAD_PTR.
                        */
                        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, NULL);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        lane = 0;
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeSet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTransmitModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with laneNum [0 - 3] and mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsSerdesTransmitModeGet with not-NULL modePtr.
    Expected: GT_OK and the same mode as was set.
    1.1.3. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK.
    1.1.4. Call with wrong mode.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with wrong mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT modeGet = 1;
    GT_U32      serdesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL                         isFlex;

CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        if(prvUtfIsGmCompilation())
        {
            break;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlex);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortIsFlexLinkGet: %d, %d",
                                         dev, port);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E].
                Expected: GT_OK.
            */
            mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;

            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);

                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                        /*
                            1.1.2. Call cpssDxChDiagPrbsSerdesTransmitModeGet with not-NULL modePtr.
                            Expected: GT_OK and the same mode as was set.
                        */
                        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &modeGet);

                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                    "cpssDxChDiagPrbsSerdesTransmitModeGet: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                                       "got another mode then was set: %d, %d", dev, port);
                    }
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

            }

            /*
                1.1.3. Call with wrong laneNum[DIAG_INVALID_LANENUM_CNS].
                Expected: NOT GT_OK for flexLink ports and GT_OK for others.
            */

            lane = serdesNum;

            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);

            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else /* FE Port */
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            lane = 0;


            /*
                1.1.5. Call with wrong mode.
                Expected: GT_NOT_SUPPORTED.
            */

            for(mode = 0; mode < CPSS_DXCH_DIAG_TRANSMIT_MODE_MAX_E; mode++)
            {
                if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                   (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                   (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    if(mode < CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E)
                    {
                        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, mode);
                    }
                }
                else
                {
                    if((mode < CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E)
                        || (mode > CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E))
                    {
                        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, mode);
                    }
                }

            }

            mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
        }

        lane = 0;
        mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeGet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTransmitModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL mode.
    Expected: GT_OK.
    1.1.2. Call with wrong laneNum[DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK for flexLink ports and GT_OK for others.
    1.1.3. Call modePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;
    GT_U32      serdesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL                         isFlex;

CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        if(prvUtfIsGmCompilation())
        {
            break;
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlex);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortIsFlexLinkGet: %d, %d",
                                         dev, port);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with not-NULL mode.
                Expected: GT_OK
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                }
                else /* FE Port */
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

                st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else /* FE Port */
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

            }

            /*
                1.1.2. Call with wrong laneNum[DIAG_INVALID_LANENUM_CNS].
                Expected: NOT GT_OK for flexLink ports and GT_OK for others.
            */

            lane = serdesNum;
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E);
            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);

            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            lane = 0;

            /*
                1.1.2. Call with wrong modePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, modePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemoryBistBlockStatusGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType,
    IN  GT_U32                          blockIndex,
    OUT GT_BOOL                         *blockFixedPtr,
    OUT GT_U32                          *replacedIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemoryBistBlockStatusGet)
{
/*
    ITERATE_DEVICES (DxCh3)
    1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                and blockIndex [0].
    Expected: GT_OK.
    1.2. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                and blockIndex [13].
    Expected: GT_OK.
    1.3. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                and blockIndex [19].
    Expected: GT_OK.
    1.4. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                and  out of range blockIndex [14].
    Expected: NOT GT_OK.
    1.5. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                and  out of range blockIndex [20].
    Expected: NOT GT_OK.
    1.6. Call with wrong enum values memBistType.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong blockFixedPtr [NULL].
    Expected: GT_BAD_PTR.
    1.8. Call with wrong replacedIndex [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
    GT_U32                          blockIndex = 0;
    GT_BOOL                         blockFixed;
    GT_U32                          replacedIndex;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                        CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                        and blockIndex [0].
            Expected: GT_OK
        */
        blockIndex  = 0;

        /*call with memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        /*call with memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        /*
            1.2. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                        and blockIndex [13].
            Expected: GT_OK.
        */
        blockIndex  = 13;
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.3. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                        and blockIndex [19].
            Expected: GT_OK.
        */
        blockIndex  = 19;
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.4. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                        and  out of range blockIndex [14].
            Expected: NOT GT_OK.
        */
        blockIndex  = 14;
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.5. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                        and  out of range blockIndex [20].
            Expected: NOT GT_OK.
        */
        blockIndex  = 20;
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.6. Call with wrong enum values memBistType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemoryBistBlockStatusGet
                            (dev, memBistType, blockIndex, &blockFixed, &replacedIndex),
                            memBistType);

        /*
            1.7. Call with wrong blockFixedPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, NULL, &replacedIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.8. Call with wrong replacedIndex [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                blockIndex, &blockFixed, &replacedIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemoryBistsRun
(
    IN  GT_U8                           devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType,
    IN  GT_U32                          timeOut,
    IN  GT_BOOL                         clearMemoryAfterTest
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemoryBistsRun)
{
/*
    ITERATE_DEVICES (DxCh3)
    1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                       timeOut [0 / 10]
                   and clearMemoryAfterTest[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call with wrong enum values memBistType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
    CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT bistTestType = CPSS_DIAG_PP_MEM_BIST_PURE_MEMORY_TEST_E;
    GT_U32                          timeOut = 0;
    GT_BOOL                         clearMemoryAfterTest = GT_TRUE;
    GT_U32                               testsToRunBmpPtr = 0;
    GT_U32                               testsResultBmpPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                        CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                               timeOut [0 / 10]
                               and clearMemoryAfterTest[GT_FALSE / GT_TRUE].
            Expected: GT_OK
        */

        /* call with memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
        timeOut = 0;
        clearMemoryAfterTest = GT_FALSE;

        st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
        st = (GT_TIMEOUT == st) ? GT_OK : st;
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        /* call with memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;
        timeOut = 10;
        clearMemoryAfterTest = GT_TRUE;

        st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
        st = (GT_TIMEOUT == st) ? GT_OK : st;
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        /*
            1.2. Call with wrong enum values memBistType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemoryBistsRun
                            (dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr),
                            memBistType);
    }

    memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
    timeOut     = 0;
    clearMemoryAfterTest = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT2_E);

    timeOut = 10;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupMemWrite
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupMemWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Write / Read memory word for each port group,
         Call with memType = [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                              CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                              CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                  offset [0], and non-NULL dataPtr.
    Expected: GT_OK.
    1.2. Write memory word with CPSS_PORT_GROUP_UNAWARE_MODE_CNS
         and read for each port group.
    Expected: GT_OK and the same data.
    1.3. Write different memory words for each port groups. Read memory word with
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, it should be equal to one from
        port group 0.
    Expected: GT_OK and the same data.
    1.4. Write to specific port group and check that other port groups are not
    influenced.
    Expected: GT_OK and the same data.
    1.5. Call with wrong enum values memType and other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with dataPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
    GT_U32                     offset = 0;
    GT_U32                     data = 0;
    GT_U32                     readData = 0;
    GT_PORT_GROUPS_BMP         portGroupsBmp = 1;
    CPSS_DIAG_PP_MEM_TYPE_ENT  memTypesArray[3];
    GT_U32                     portGroupId;
    GT_U32                     memTypeId;
    GT_U32                     memTypeLen = 3;
    GT_U32                     i;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily = CPSS_PP_FAMILY_CHEETAH_E;
    GT_U32                     lastSetValue = 0;

    memTypesArray[0] = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
    memTypesArray[1] = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E;
    memTypesArray[2] = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Write / Read memory word for each port group,
                 Call with memType = [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                      CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                                      CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                          offset [0], and non-NULL dataPtr.
            Expected: GT_OK.
        */

#ifdef ASIC_SIMULATION
        if ((devFamily <= CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
            (devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            /* on those devices the 'buffer memory' memories not simulated although the device had it . */
            /* do not test CPSS_DIAG_PP_MEM_BUFFER_DRAM_E */
            memTypeLen = 2;
        }
#endif /* ASIC_SIMULATION*/

        for (memTypeId = 0; memTypeId < memTypeLen; memTypeId++)
        {
            memType = memTypesArray[memTypeId];

           PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
           {
               portGroupsBmp = (1 << portGroupId);

               if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
               {
                    if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                       (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                       /* In Lion Egress VLAN exist only in two port groups */
                        if ((0x3 & portGroupsBmp) == 0x0)
                        {
                            break;
                        }
                    }
                    else
                    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                    {
                        /* the EGF_SHT that hold (part of) the egress vlan table
                           is on 'even' port groups */
                        if ((0x55555555 & portGroupsBmp) == 0x0)
                        {
                            /* jump to next port group */
                            continue;
                        }
                    }
               }

                data = 0xAAAA5555;
                offset = 0x0;

                st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp,
                                                   memType, offset, data);
                if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                    (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                    (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
                {
                    /* Lion2 does not have CPU access to buffer memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                       memType, offset, &readData);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                       "The read data differs from written data: offset = %d, portGroupId = %d",
                                                 offset, portGroupId);
                }
                data = 0xFFFFFFFF;
                offset = 0x40;

                st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp,
                                                   memType, offset, data);
                if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                    (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                    (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
                {
                    /* Lion2 does not have CPU access to buffer memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                       memType, offset, &readData);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                      "The read data differs from written data: offset = %d, portGroupId = %d",
                                                 offset, portGroupId);
                }
           }
           PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        }

        /*
            1.2. Write memory word with CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                 and read for each port group.
            Expected: GT_OK and the same data.
        */

        for (memTypeId = 0; memTypeId < memTypeLen; memTypeId++)
        {
            memType = memTypesArray[memTypeId];
            data = 0xAAAA5555;
            offset = 0x80;

            st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               memType, offset, data);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
            {
                /* Lion2 does not have CPU access to buffer memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                /* read for each port group */
               PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
               {
                    /* set next port */
                    portGroupsBmp = (1 << portGroupId);

                   if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
                   {
                        if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                           (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                        {
                           /* In Lion Egress VLAN exist only in two port groups */
                            if ((0x3 & portGroupsBmp) == 0x0)
                            {
                                break;
                            }
                        }
                        else
                        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                        {
                            /* the EGF_SHT that hold (part of) the egress vlan table
                               is on 'even' port groups */
                            if ((0x55555555 & portGroupsBmp) == 0x0)
                            {
                                /* jump to next port group */
                                continue;
                            }
                        }
                   }

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                      memType, offset, &readData);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                     "The read data differs from written data: offset = %d, portGroupId = %d",
                                                 offset, portGroupId);
               }
               PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
            }
        }


        /*
            1.3. Write different memory words for each port groups. Read memory word with
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS, it should be equal to one from
                port group 0.
                Note: In Lion B0, the Egress VLAN table is duplicated in core 0 and 1.
                Expected: GT_OK and the same data.
        */

        for (memTypeId = 0; memTypeId < memTypeLen; memTypeId++)
        {
            memType = memTypesArray[memTypeId];
            data = 0xAAAAAAAA;
            offset = 0x40;

            /* write to each port group */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
            {
                /* set next port */
                portGroupsBmp = (1 << portGroupId);

               if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
               {
                    if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                       (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                       /* In Lion Egress VLAN exist only in two port groups */
                        if ((0x3 & portGroupsBmp) == 0x0)
                        {
                            break;
                        }
                    }
                    else
                    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                    {
                        /* the EGF_SHT that hold (part of) the egress vlan table
                           is on 'even' port groups */
                        if ((0x55555555 & portGroupsBmp) == 0x0)
                        {
                            /* jump to next port group */
                            continue;
                        }
                    }
               }

                st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp, memType, offset, data);
                lastSetValue = data;
                if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                    (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                    (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
                {
                    /* Lion2 does not have CPU access to buffer memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);
                }

                data |= (0x55 << (portGroupId * 4));
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

            /* Verify that the read data equals to the read data from the port group 0 */
            st = cpssDxChDiagPortGroupMemRead(dev,
                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, memType, offset, &readData);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
            {
                /* Lion2 does not have CPU access to buffer memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                data = 0xAAAAAAAA;
                if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
                {
                    if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                       (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                    {
                        /* This will be the data written to portGroup 0 and 1*/
                        data = 0xAAAAAAFF;
                    }
                    else
                    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                    {
                        /* the EGF_SHT that hold FULL egress vlan table (supports 256 physical ports)
                           is on 'even' port groups */
                        /* This will be the data written to portGroup 0,2,4,6 */
                        /* the CPSS ignores the 'specific port group' that we gave
                           in cpssDxChDiagPortGroupMemWrite and write it to all needing port groups !!
                           because values must be synched in all those port groups.
                           so last write is actually the value that we will read from any port group.
                           */
                        data = lastSetValue;
                    }
                }

                UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                 "The read data differs from written data: offset = %d, portGroupId = %d",
                                             offset, portGroupId);
            }
        }

        /*
            1.4. Write to specific port group and check that other port groups are not
            influenced.
            Expected: GT_OK and the same data.
        */
        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
        offset  = 0x0;
        data    = 0x0;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        {
            data = 0x0;

            /* write 0 to all port groups */
            st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               memType, offset, data);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* write memory word to specific port group id */

            portGroupsBmp = (1 << portGroupId);

            data = 0x5555AAAA;

            st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp,
                                               memType, offset, data);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* read data from other port groups and check that it was not changed */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,i)
            {
                if (portGroupId != i)
                {
                    portGroupsBmp = (1 << i);

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                      memType, offset, &data);
                    if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                        (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                    {
                        /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                        UTF_VERIFY_EQUAL2_STRING_MAC(0x0, data,
                         "The read data differs from written data: offset = %d, portGroupId = %d",
                                                     offset, i);
                    }
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,i)
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /*
            1.5. Call with wrong enum values memType and other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        data = 0xAAAAAAAA;
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemWrite
                            (dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, memType, offset, data),
                            memType);

        /*
            1.6. Call with dataPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagPortGroupMemRead(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          memType, offset, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          memType, offset, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                      memType, offset, data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupMemRead
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupMemRead)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1.1.  Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                                CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                  offset [0 / 100 / 1000], and non-NULL dataPtr.
    Expected: GT_OK.
    1.1.2. Call with wrong enum values memType and other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with dataPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
    GT_U32                     offset = 0;
    GT_U32                     readData;
    GT_PORT_GROUPS_BMP         portGroupsBmp = 1;
    GT_U32                     portGroupId;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily = CPSS_PP_FAMILY_CHEETAH_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1.  Call with memType = [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                             CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                                             CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                              offset [0 / 100 / 1000], and non-NULL dataPtr.
                Expected: GT_OK.
            */
            /* call with memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E */
            memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
            offset = 0;

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);
            if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* Lion2 does not have CPU access to buffer memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /*
                1.1.2. Call with wrong enum values memType and other params same as in 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemRead
                                (dev, portGroupsBmp, memType, offset, &readData),
                                memType);

            /* call with memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E */
            memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
            offset = 100;

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /*
                1.1.2. Call with wrong enum values memType and other params same as in 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemRead
                                (dev, portGroupsBmp, memType, offset, &readData),
                                memType);

            /* call with memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E */
            memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E;
            /* The offset inside the entry should be within 297 bits - 9 words
               For example offset 980 bytes is entry 15, with offset 20 bytes
               inside the entry */
            offset = 980;

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* NOTE: bobk not hold 9 words but only 5 !!! */
                offset = 64*15 + 16;/* 976 = entry 15 word 4 */
            }


            /* In Lion Egress VLAN exist only in two port groups */
           if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
           {
                if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                   (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                   /* In Lion Egress VLAN exist only in two port groups */
                    if ((0x3 & portGroupsBmp) == 0x0)
                    {
                        continue;
                    }
                }
                else
                if ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* the EGF_SHT that hold (part of) the egress vlan table
                       is on 'even' port groups */
                    if ((0x55555555 & portGroupsBmp) == 0x0)
                    {
                        /* jump to next port group */
                        continue;
                    }
                }
           }

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /*
                1.1.2. Call with wrong enum values memType and other params same as in 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemRead
                                (dev, portGroupsBmp, memType, offset, &readData),
                                memType);

            /*
                1.1.3. Call with dataPtr [NULL], other params same as in 1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                              memType, offset, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPortGroupMemRead(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          memType, offset, &readData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPortGroupMemRead(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                      memType, offset, &readData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupRegsNumGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *regsNumPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupRegsNumGet) */
GT_VOID cpssDxChDiagPortGroupRegsNumGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1  Call for each port group cpssDxChDiagPortGroupRegsNumGet and then
           cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
    Expected: GT_OK.
    1.1.2 Call for CPSS_PORT_GROUP_UNAWARE_MODE_CNS cpssDxChDiagPortGroupRegsNumGet
          and then cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
    Expected: GT_OK.
    1.1.3. Call cpssDxChDiagPortGroupRegsNumGet for CPSS_PORT_GROUP_UNAWARE_MODE_CNS
           with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st   = GT_OK;
    GT_U8                       dev;
    GT_U32                      regsNum = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp = 1;
    GT_U32                      *regAddrPtr;
    GT_U32                      *regDataPtr;
    GT_U32                      portGroupId;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            SKIP_TEST_MAC;
        }

        if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(dev) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E))
        {
            /* the device crash the WM/HW */
            return;
        }

        /*
            1.1.1  Call for each port group cpssDxChDiagPortGroupRegsNumGet and then
            cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
            Expected: GT_OK.
        */
        /* read for each port group */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next port */
            portGroupsBmp = (1 << portGroupId);

            /* read the registers number */
            st = cpssDxChDiagPortGroupRegsNumGet(dev, portGroupsBmp, &regsNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /*cpssOsPrintf("cpssDxChDiagPortGroupRegsNumGet : regsNum = %d \n",regsNum);*/

            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                        "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                        "cpssOsMalloc: Memory allocation error.");

            /* dump registers */
            st = cpssDxChDiagPortGroupRegsDump(dev,portGroupsBmp, &regsNum, 0,
                                               regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /*
            1.1.2 Call for CPSS_PORT_GROUP_UNAWARE_MODE_CNS cpssDxChDiagPortGroupRegsNumGet
            and then cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagPortGroupRegsNumGet(dev,
                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* allocate space for regAddress and regData */
        regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
        regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                        "cpssOsMalloc: Memory allocation error.");
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                        "cpssOsMalloc: Memory allocation error.");

        st = cpssDxChDiagPortGroupRegsDump(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if (regAddrPtr != NULL)
        {
            /* free memory if allocated */
            cpssOsFree(regAddrPtr);
            regAddrPtr = (GT_U32*)NULL;
        }

        if (regDataPtr != NULL)
        {
            /* free memory if allocated */
            cpssOsFree(regDataPtr);
            regDataPtr = (GT_U32*)NULL;
        }

        /*
            1.1.3. Call cpssDxChDiagPortGroupRegsNumGet for CPSS_PORT_GROUP_UNAWARE_MODE_CNS
            with regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagPortGroupRegsNumGet(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                    "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                    "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPortGroupRegsNumGet(dev,
                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagResetAndInitControllerRegsNumGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *regsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagResetAndInitControllerRegsNumGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1 Call for cpssDxChDiagResetAndInitControllerRegsNumGet and then
        cpssDxChDiagResetAndInitControllerRegsDump with not-NULL pointers.
    Expected: GT_OK.
    1.2 Call cpssDxChDiagResetAndInitControllerRegsNumGet with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st   = GT_OK;
    GT_U8                       dev;
    GT_U32                      regsNum = 0;
    GT_U32                      *regAddrPtr;
    GT_U32                      *regDataPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1 Call for cpssDxChDiagResetAndInitControllerRegsNumGet and then
                cpssDxChDiagResetAndInitControllerRegsDump with not-NULL pointers.
            Expected: GT_OK.
        */

        /* read the registers number */
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if( (GT_FALSE == prvUtfIsGmCompilation()) || (regsNum > 0) )
        {
            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                             "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                             "cpssOsMalloc: Memory allocation error.");

            /* dump registers */
            st = cpssDxChDiagResetAndInitControllerRegsDump(dev, &regsNum, 0,
                                                            regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }

        /*
            1.2 Call cpssDxChDiagResetAndInitControllerRegsNumGet with regsNumPtr[NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                    "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                    "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupRegsDump
(
    IN    GT_U8                 devNum,
    IN    GT_PORT_GROUPS_BMP    portGroupsBmp,
    INOUT GT_U32                *regsNumPtr,
    IN    GT_U32                offset,
    OUT   GT_U32                *regAddrPtr,
    OUT   GT_U32                *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupRegsDump)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1.1. Call with regAddrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.2. Call with regDataPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4 Call for each port group cpssDxChDiagPortGroupRegsDump
          with regsNumPtr bigger then the real registers number.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              regsNum = 0;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;
    GT_U32              *regAddrPtr = 0;
    GT_U32              *regDataPtr = 0;
    GT_U32              portGroupId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with regAddrPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               &regsNum, 0, NULL, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            /*
                1.1.2. Call with regDataPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               &regsNum, 0, regAddrPtr, NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            /*
                1.1.3. Call with regsNumPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               NULL, 0, regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            /*
                1.1.4 Call for each port group cpssDxChDiagPortGroupRegsDump
                with regsNumPtr bigger then the real registers number.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsNumGet(dev, portGroupsBmp,
                                                 &regsNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc((regsNum+1) * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc((regsNum+1) * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                    "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                    "cpssOsMalloc: Memory allocation error.");

            /* clean buffers to avoid trash in log */
            cpssOsMemSet(regAddrPtr, 0, (regsNum+1) * sizeof(GT_U32));
            cpssOsMemSet(regDataPtr, 0, (regsNum+1) * sizeof(GT_U32));
            regsNum += 1;

            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               &regsNum, 0, regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                           &regsNum, 0, NULL, regDataPtr);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                            "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                            "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        regsNum = 1;
        st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                           &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    regsNum = 1;
    st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                       &regsNum, 0, regAddrPtr, regDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagResetAndInitControllerRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagResetAndInitControllerRegsDump)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with regAddrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.2. Call with regDataPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4 Call with regsNumPtr bigger then the real registers number.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              regsNum = 0;
    GT_U32              *regAddrPtr = 0;
    GT_U32              *regDataPtr = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with regAddrPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                            dev, &regsNum, 0, NULL, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.2. Call with regDataPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                            dev, &regsNum, 0, regAddrPtr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3. Call with regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                          dev, NULL, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4 Call with regsNumPtr bigger then the real registers number.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if( (GT_FALSE == prvUtfIsGmCompilation()) || (regsNum > 0) )
        {
            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                             "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                             "cpssOsMalloc: Memory allocation error.");

            regsNum += 1;

            st = cpssDxChDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                            "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                            "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        regsNum = 1;
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    regsNum = 1;
    st = cpssDxChDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}

UTF_TEST_CASE_MAC(cpssDxChDiagDeviceVoltageGet)
{
    GT_STATUS st = GT_OK;
    GT_U8  dev = 0;
    GT_U32 voltage_milivolts;
    GT_U32 sensorNum;
    GT_U32      notAppFamilyBmp = 0;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    GM_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* sometimes test result in TEMP_INT interrupt and shutdown of the board */
    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_ALDRIN_E, "JIRA:CPSS-7658");
#endif

    /*PRV_UTF_LOG0_MAC("starting Voltage sensor test\n");*/
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*PRV_UTF_LOG1_MAC("dev=%d\n", dev);*/
        for (sensorNum = 0 ; sensorNum < 4 ; sensorNum++)
        {

            st = cpssDxChDiagDeviceVoltageGet(dev, sensorNum, &voltage_milivolts );
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            PRV_UTF_LOG3_MAC("dev=%d sensorNum=%d mV=%d\n", dev, sensorNum, voltage_milivolts);
#ifdef ASIC_SIMULATION
            /* in WM for hwValue = 0 and divider_en =0 divider_cfg =1 calculated value should be 1208mV */
            if (GT_OK == st && voltage_milivolts != 1208)
#else
            if (GT_OK == st && (voltage_milivolts <= 900 || voltage_milivolts >= 1150 ))
#endif
            {
                st = GT_OUT_OF_RANGE;
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        /* 4. sensorNumber out-of-range */
        st = cpssDxChDiagDeviceVoltageGet(dev, 4, &voltage_milivolts );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 5. NULL OUT ptr */
        st = cpssDxChDiagDeviceVoltageGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDeviceVoltageGet(dev, 0, &voltage_milivolts );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceVoltageGet(dev, 0, &voltage_milivolts );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDeviceTemperatureGet
(
    IN  GT_U8  dev,
    OUT GT_32  *temperaturePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureGet)
{
/*
    ITERATE_DEVICES(Lion and above)
    1.1. Call with not null temperaturePtr.
    Expected: GT_OK.
    1.2. Call api with wrong temperaturePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8  dev;
    GT_32  temperature;
    GT_BOOL didAnySensore;
    GT_U32  ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        didAnySensore = GT_FALSE;
        for(ii = 0 ; ii < (CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_MAX_E+1); ii ++)
        {
            st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev,ii);
            if(ii == 0 && st == GT_NOT_APPLICABLE_DEVICE)
            {
                /* the device not supports this API */
                /* so just get it's temperature */
                ii = 100;/*cause loop to end after this iteration */
            }
            else
            if(st == GT_BAD_PARAM)
            {
                /* this sensor is not supported by the device */
                continue;
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            didAnySensore = GT_TRUE;

            /*
                1.1. Call with not null temperaturePtr.
                Expected: GT_OK.
            */
            st = cpssDxChDiagDeviceTemperatureGet(dev, &temperature);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

#ifndef ASIC_SIMULATION
            /* make sure we get acceptable value that is not under 0 Celsius  and not above 200 */
            UTF_VERIFY_EQUAL2_STRING_MAC(0, (temperature <= 0 || temperature >= 200) ? 1 : 0,
                                       "(sensor[%d] --> temperature[%d] lower then 0 or higher then 200 degrees Celsius)",
                                       ii, temperature);
#endif
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, didAnySensore,
                                   "didAnySensore = %d", didAnySensore);

        /*
            1.2. Call api with wrong temperaturePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagDeviceTemperatureGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, temperaturePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDeviceTemperatureGet(dev, &temperature);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureGet(dev, &temperature);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningRxTune
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC    *portLaneArrPtr,
    IN  GT_U32                                      portLaneArrLength,
    IN  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT            prbsType,
    IN  GT_U32                                      prbsTime,
    IN  CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC              *optResultArrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningRxTune)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat, Lion)
    1.1.1. Call cpssDxChDiagSerdesTuningRxTune with
                    prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E,
                             CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E,
                             CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E],
                    prbsTime[0, 50, 100],
                    optMode[CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E],
                    portLaneArrLength[1],
                    valid portLaneArr.laneNum;
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum.
    Expected: NOT GT_OK.
    1.1.3. Call with out of range prbsType and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range optMode and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with out of range optResultArrPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/

#ifndef ASIC_SIMULATION
    GT_STATUS                                   st = GT_OK;
    GT_PHYSICAL_PORT_NUM                        port = DIAG_VALID_PORTNUM_CNS;
    GT_U8                                       dev = 0;
    CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC    portLaneArr         = {0};
    GT_U32                                      portLaneArrLength   = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT            prbsType            = 0;
    GT_U32                                      prbsTime            = 0;
    CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode             = 0;
    CPSS_DXCH_PORT_SERDES_TUNE_STC              optResultArr        = {0};

    GT_U32                                      serdesNum           = 0;
    GT_U32                                      lane                = 0;
    GT_BOOL                                     defaultTimeTested   = GT_FALSE;
#endif
    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* skip to reduce size of CPSS API log */
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Initialize Serdes optimization system */
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get max lane num */
            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            if(0 == serdesNum)
            {/* could be port which serdeses occupied by neighbor */
                continue;
            }

            /*
                1.1.1. Call cpssDxChDiagSerdesTuningRxTune with
                        prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E,
                                 CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E,
                                 CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E],
                        prbsTime[0, 50, 100],
                        optMode[CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E];
                Expected: GT_OK.
            */
            for (lane = 0; lane < serdesNum; lane++)
            {
                if(!defaultTimeTested)
                {/* test 200 ms just once, otherwise this test will take too long */

                    /* call with prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E] */
                    prbsType = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
                    prbsTime = 0;
                    optMode = CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E;

                    portLaneArr.laneNum = lane;
                    portLaneArr.portNum = port;
                    portLaneArrLength = 1;

                    st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                        prbsType, prbsTime, optMode, &optResultArr);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "dev %d, port %d", dev, port);

                    defaultTimeTested = GT_TRUE;
                }

                /* call with prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E] */
                prbsType = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E;
                prbsTime = 2;
                optMode = CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E;

                portLaneArr.laneNum = lane;
                portLaneArr.portNum = port;
                portLaneArrLength = 1;

                st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                    prbsType, prbsTime, optMode, &optResultArr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "dev %d, port %d", dev, port);


                /* call with prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E] */
                prbsType = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E;
                prbsTime = 4;
                optMode = CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E;

                portLaneArr.laneNum = lane;
                portLaneArr.portNum = port;
                portLaneArrLength = 1;

                st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                    prbsType, prbsTime, optMode, &optResultArr);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "dev %d, port %d", dev, port);
            }

            /*
                1.1.2. Call with out of range laneNum.
                Expected: NOT GT_OK.
            */
            portLaneArr.laneNum = serdesNum;

            st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                prbsType, prbsTime, optMode, &optResultArr);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                             "dev %d, port %d", dev, port);

            /* restore value */
            portLaneArr.laneNum = 0;

            /*
                1.1.2. Call with out of range prbsType and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagSerdesTuningRxTune
                                (dev, &portLaneArr, portLaneArrLength,
                                prbsType, prbsTime, optMode, &optResultArr),
                                prbsType);

            /*
                1.1.4. Call with out of range optMode and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagSerdesTuningRxTune
                                (dev, &portLaneArr, portLaneArrLength,
                                prbsType, prbsTime, optMode, &optResultArr),
                                optMode);

            /*
                1.1.5. Call with out of range optResultArrPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                prbsType, prbsTime, optMode, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "dev %d, port %d", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            portLaneArr.portNum = port;
            st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                prbsType, prbsTime, optMode, &optResultArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portLaneArr.portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                            prbsType, prbsTime, optMode, &optResultArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portLaneArr.portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                            prbsType, prbsTime, optMode, &optResultArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* free all resources allocated by tuning algorithm */
        st = cpssDxChDiagSerdesTuningSystemClose(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    portLaneArr.portNum = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                            prbsType, prbsTime, optMode, &optResultArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                        prbsType, prbsTime, optMode, &optResultArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
#endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningSystemClose
(
    IN    GT_U8  devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningSystemClose)
{
/*
    ITERATE_DEVICES (xCat, Lion)
    Old - 1.1. Call cpssDxChDiagSerdesTuningSystemClose (before init).
    Expected: NON_GT_OK. - not applicable - changed and won't fail any more.
    1.1. Call cpssDxChDiagSerdesTuningSystemInit.
    Expected: GT_OK.
    1.2. Call cpssDxChDiagSerdesTuningSystemClose (after init).
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChDiagSerdesTuningSystemInit.
            Expected: GT_OK.
        */
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call cpssDxChDiagSerdesTuningSystemClose (after init).
            Expected: GT_OK.
        */
        st = cpssDxChDiagSerdesTuningSystemClose(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningSystemClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningSystemClose(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningSystemInit
(
    IN    GT_U8  devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningSystemInit)
{
/*
    ITERATE_DEVICES (xCat, Lion)
    1.1. Call cpssDxChDiagSerdesTuningSystemInit.
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChDiagSerdesTuningSystemInit.
            Expected: GT_OK.
        */
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningSystemInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningTracePrintEnable
(
    IN    GT_U8  devNum,
    IN    GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningTracePrintEnable)
{
/*
    ITERATE_DEVICES (xCat, Lion)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 enable      = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with sensorType[CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_2_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_4_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_AVERAGE_E].
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range sensorType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      tsenNum = 5; /* number of temperature sensors */

    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorType    = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorTypeGet = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorTypeSave;

    /* there is no DFX in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        switch (PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                if (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
                {
                    /* BC2 */
                    tsenNum = 2;
                }
                else
                {
                    /* Cetus and Caelum */
                    tsenNum = 1;
                }
                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            case CPSS_PP_FAMILY_DXCH_AC3X_E:
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                tsenNum = 1;
                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                tsenNum = 4;
                break;
            default:
                tsenNum = 5;
                break;
        }

        /* save sensor selection */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

        /* 1.2. */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                        "get another sensorType than was set: %d", dev);

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 2)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_2_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 3)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 4)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_4_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 5)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_AVERAGE_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

        /* 1.2. */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                        "get another sensorType than was set: %d", dev);

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_MAX_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(!UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) &&
           PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.3. */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectSet
                                    (dev, sensorType),
                                    sensorType);

        /* restore sensor type */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorTypeSave);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorTypeSave);
    }

    sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL sensorTypePtr.
    Expected: GT_OK.
    1.2. Call with sensorTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, sensorTypePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureThresholdSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with thresholdValue for Lion2 [-142 \ 0 \ 228],
                                  for Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 [-277 \ 0 \ 198],
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range thresholdValue[-143 \ 229].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_32       thresholdValue    = 0;
    GT_32       thresholdValueGet = 0;
    GT_32       thresholdSave;

    /* there is no DFX in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save threshould */
                st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdSave);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* test low threshoulds on simulation only because it may shutdown board */
#ifdef ASIC_SIMULATION
            /* 1.1. */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
           PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            thresholdValue = -40;
        }
        else
        {
            thresholdValue = -142;
        }

        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.2. */
                st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* the value is rounded down by Set function */
                if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                   PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {

                    thresholdValue = -40;
                }
                else
                {
                    thresholdValue = -141;
                }

                UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                                "get another thresholdValue than was set: %d", dev);

                /* 1.1. */
                thresholdValue = 0;

                st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

                /* 1.2. */
                st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                                "get another thresholdValue than was set: %d", dev);
#endif
                /* 1.1. */
                if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                   PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    thresholdValue = 150;
                }
                else
                {
                    thresholdValue = 228;
                }

                st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

                /* 1.2. */

                if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                   PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    /* the value is rounded down by Set function */
                    thresholdValue = 149;
                }
                st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                                "get another thresholdValue than was set: %d", dev);

                /* 1.3. */
                if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                   PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    thresholdValue = -41;
                }
                else
                {
                    thresholdValue = -143;
                }

                st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

                /* 1.3. */
                if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                   PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                {
                    thresholdValue = 151;
                }
                else
                {
                    thresholdValue = 229;
                }

                st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

                thresholdValue = 0;

        /* restore thresholdValue */
        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdSave);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdSave);
    }

    thresholdValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureThresholdGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL thresholdValuePtr.
    Expected: GT_OK.
    1.2. Call with thresholdValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_32       thresholdValue = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. */
        st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, thresholdValuePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*static GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
)*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet)
{
    GT_STATUS               st  = GT_OK;                    /* function return value */
    GT_U8                   dev  = 0;                       /* device number */
    GT_PHYSICAL_PORT_NUM    port = DIAG_VALID_PORTNUM_CNS;  /* port number */
    GT_U32                  lane;                           /* lane number per port */
    GT_BOOL                 clearOnReadEnable;              /* clear on read enable flag */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
         UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            for(lane = 0; lane < PRV_CPSS_MAX_PORT_LANES_CNS; lane++)
            {
                /* 1.1.1 Set GT_FALSE */
                st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, lane, GT_FALSE);
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, &clearOnReadEnable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_FALSE, clearOnReadEnable," Set GT_FALSE but Get GT_TRUE : %d %d %d", dev, port, lane);
                    /* 1.1.2 Set GT_TRUE */
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, lane, GT_TRUE);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, &clearOnReadEnable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, clearOnReadEnable," Set GT_TRUE but Get GT_FALSE : %d %d %d", dev, port, lane);
                }
            }
            /*
                1.1.3. Call with lane number out of range.
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, lane, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "lane number out of range: %d %d", dev, port);
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. For all active devices go over all non available physical ports.
           Expected: GT_BAD_PARAM
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
          check that function returns GT_BAD_PARAM.                        */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*static GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet)
{
    GT_STATUS               st  = GT_OK;                    /* function return value */
    GT_U8                   dev  = 0;                       /* device number */
    GT_PHYSICAL_PORT_NUM    port = DIAG_VALID_PORTNUM_CNS;  /* port number */
    GT_U32                  lane;                           /* lane number per port */
    GT_BOOL                 clearOnReadEnable;              /* clear on read enable flag */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
         UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            for(lane = 0; lane < PRV_CPSS_MAX_PORT_LANES_CNS; lane++)
            {
                /*
                    1.1.1. Call with not-NULL clearOnReadEnable.
                    Expected: GT_OK.
                */
                st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, &clearOnReadEnable);
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    /*
                        1.1.2. Call with NULL clearOnReadEnable.
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "clearOnReadEnable pointer = NULL: %d %d", dev, port);
                }
            }
            /*
                1.1.3. Call with lane number out of range.
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, PRV_CPSS_MAX_PORT_LANES_CNS, &clearOnReadEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "lane number out of range: %d %d", dev, port);
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. For all active devices go over all non available physical ports.
           Expected: GT_BAD_PARAM
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
          check that function returns GT_BAD_PARAM.                        */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChDiag suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChDiag)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagAllMemTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagAllRegTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortCheckEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortCheckReadyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortGenerateEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortGenerateEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortTransmitModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortTransmitModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagResetAndInitControllerRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagResetAndInitControllerRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsCyclicDataSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsCyclicDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPhyRegRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPhyRegWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTestEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTestEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTransmitModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTransmitModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemoryBistBlockStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemoryBistsRun)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupMemWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupMemRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceVoltageGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningSystemClose)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningRxTune)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningSystemInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningTracePrintEnable)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesFunctionalityTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet)

UTF_SUIT_END_TESTS_MAC(cpssDxChDiag)

