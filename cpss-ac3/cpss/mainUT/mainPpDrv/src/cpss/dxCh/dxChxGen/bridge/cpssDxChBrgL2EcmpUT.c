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
* @file cpssDxChBrgL2EcmpUT.c
*
* @brief Unit tests for cpssDxChBrgL2Ecmp, that provides
* L2 ECMP facility CPSS DxCh implementation.
*
* @version   8
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* Defines */

/* Default valid value for port id */
#define BRG_L2ECMP_VALID_PHY_PORT_CNS  0

/* number entries in the ECMP table*/ CPSS_TBD_BOOKMARK
#define    ECMP_TABLE_SIZE_CNS(_dev) (PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.l2EcmpNumMembers - \
    PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.l2EcmpNumMembers_usedByTrunk) /*actual is _8K but high part used for the trunk entries*/
#define    MAX_NUM_OF_PATH_CNS      (_4K+1)/* maximum number of paths */
#define    TESTED_NUM_OF_PATH_CNS   100/*number of paths */

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      ecmpIndexBaseEport
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpIndexBaseEportSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with ecmpIndexBaseEport [0 / 32 / maxEPortNum]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgL2EcmpIndexBaseEportGet with not NULL ecmpIndexBaseEportPtr.
    Expected: GT_OK and the same ecmpIndexBaseEport.
    1.3. Call with out of range ecmpIndexBaseEport [maxEPortNum + 1].
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_PORT_NUM ecmpIndexBaseEport    = 0;
    GT_PORT_NUM ecmpIndexBaseEportGet = 0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with ecmpIndexBaseEport [0 / 32 / maxEPortNum]
            Expected: GT_OK.
        */
        /* call with ecmpIndexBaseEport = 0 */
        ecmpIndexBaseEport = 0;

        st = cpssDxChBrgL2EcmpIndexBaseEportSet(dev, ecmpIndexBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecmpIndexBaseEport);

        /*
            1.2. Call cpssDxChBrgL2EcmpIndexBaseEportGet with not NULL ecmpIndexBaseEportPtr.
            Expected: GT_OK and the same ecmpIndexBaseEport.
        */
        st = cpssDxChBrgL2EcmpIndexBaseEportGet(dev, &ecmpIndexBaseEportGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpIndexBaseEportGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpIndexBaseEport, ecmpIndexBaseEportGet,
                       "get another ecmpIndexBaseEport than was set: %d", dev);

        /* call with ecmpIndexBaseEport = 32 */
        ecmpIndexBaseEport = 32;

        st = cpssDxChBrgL2EcmpIndexBaseEportSet(dev, ecmpIndexBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecmpIndexBaseEport);

        /*
            1.2. Call cpssDxChBrgL2EcmpIndexBaseEportGet with not NULL ecmpIndexBaseEportPtr.
            Expected: GT_OK and the same ecmpIndexBaseEport.
        */
        st = cpssDxChBrgL2EcmpIndexBaseEportGet(dev, &ecmpIndexBaseEportGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpIndexBaseEportGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpIndexBaseEport, ecmpIndexBaseEportGet,
                       "get another ecmpIndexBaseEport than was set: %d", dev);

        /* call with ecmpIndexBaseEport = maxEPortNum */
        ecmpIndexBaseEport = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev);

        st = cpssDxChBrgL2EcmpIndexBaseEportSet(dev, ecmpIndexBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecmpIndexBaseEport);

        /*
            1.2. Call cpssDxChBrgL2EcmpIndexBaseEportGet with not NULL ecmpIndexBaseEportPtr.
            Expected: GT_OK and the same ecmpIndexBaseEport.
        */
        st = cpssDxChBrgL2EcmpIndexBaseEportGet(dev, &ecmpIndexBaseEportGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpIndexBaseEportGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpIndexBaseEport, ecmpIndexBaseEportGet,
                       "get another ecmpIndexBaseEport than was set: %d", dev);

        /*
            1.3. Call with out of range ecmpIndexBaseEport [maxEPortNum + 1].
            Expected: NON GT_OK.
        */
        ecmpIndexBaseEport = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev) + 1;

        st = cpssDxChBrgL2EcmpIndexBaseEportSet(dev, ecmpIndexBaseEport);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecmpIndexBaseEport);
    }

    ecmpIndexBaseEport = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpIndexBaseEportSet(dev, ecmpIndexBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpIndexBaseEportSet(dev, ecmpIndexBaseEport);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpIndexBaseEportGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *ecmpIndexBaseEportPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpIndexBaseEportGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non null ecmpIndexBaseEportPtr.
    Expected: GT_OK.
    1.2. Call with ecmpIndexBaseEportPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32      ecmpIndexBaseEport = 0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null ecmpIndexBaseEportPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2EcmpIndexBaseEportGet(dev, &ecmpIndexBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with ecmpIndexBaseEportPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2EcmpIndexBaseEportGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ecmpIndexBaseEportPtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpIndexBaseEportGet(dev, &ecmpIndexBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpIndexBaseEportGet(dev, &ecmpIndexBaseEport);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpLttTableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index [0 / maxIndex],
                   ecmpLttInfoPtr->{ecmpStartIndex [0 / 0x3FFF],
                                    ecmpNumOfPaths [0 / 4K],
                                    ecmpEnable [GT_TRUE / GT_FALSE]}
    Expected: GT_OK.
    1.2. Call cpssDxChBrgL2EcmpLttTableGet with same index
                                                not NULL ecmpLttInfoPtr.
    Expected: GT_OK and the same ecmpLttInfoPtr.
    1.3. Call with out of range index [maxIndex + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range ecmpLttInfoPtr->ecmpStartIndex [0x4000],
                   and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range ecmpLttInfoPtr->ecmpNumOfPaths [4K+1],
                   and other valid params.
    Expected: NON GT_OK.
    1.6. Call with out of range ecmpLttInfoPtr->ecmpNumOfPaths [0],
                   and other valid params.
    Expected: NON GT_OK.
    1.7. Call with ecmpLttInfoPtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      index = 0;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC ecmpLttInfo    = {0};
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC ecmpLttInfoGet = {0};
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / maxIndex],
                           ecmpLttInfoPtr->{ecmpStartIndex [0 / 0x3FFF],
                                            ecmpNumOfPaths [0 / 0x3F],
                                            ecmpEnable [GT_TRUE / GT_FALSE]}
            Expected: GT_OK.
        */
        /* call with index = 0 */
        index = 0;

        ecmpLttInfo.ecmpStartIndex = 0;
        ecmpLttInfo.ecmpNumOfPaths = 1;
        ecmpLttInfo.ecmpEnable     = GT_TRUE;
                ecmpLttInfo.ecmpRandomPathEnable = GT_TRUE;

        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call cpssDxChBrgL2EcmpLttTableGet with same index
                                                        not NULL ecmpLttInfoPtr.
            Expected: GT_OK and the same ecmpLttInfoPtr.
        */
        st = cpssDxChBrgL2EcmpLttTableGet(dev, index, &ecmpLttInfoGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpLttTableGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpStartIndex, ecmpLttInfoGet.ecmpStartIndex,
                       "get another ->ecmpStartIndex than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpNumOfPaths, ecmpLttInfoGet.ecmpNumOfPaths,
                       "get another ->ecmpNumOfPaths than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpEnable, ecmpLttInfoGet.ecmpEnable,
                       "get another ->ecmpEnable than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpRandomPathEnable, ecmpLttInfoGet.ecmpRandomPathEnable,
                       "get another ->ecmpRandomPathEnable than was set: %d", dev);


        /* call with index = maxIndex */
        index = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev) - 1;

        ecmpLttInfo.ecmpStartIndex = ECMP_TABLE_SIZE_CNS(dev) / 2;
        ecmpLttInfo.ecmpNumOfPaths = (ECMP_TABLE_SIZE_CNS(dev) > (TESTED_NUM_OF_PATH_CNS + ecmpLttInfo.ecmpStartIndex)) ?
                                    TESTED_NUM_OF_PATH_CNS : (ECMP_TABLE_SIZE_CNS(dev) - ecmpLttInfo.ecmpStartIndex - 1);
        ecmpLttInfo.ecmpEnable     = GT_FALSE;
                ecmpLttInfo.ecmpRandomPathEnable = GT_TRUE;

        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call cpssDxChBrgL2EcmpLttTableGet with same index
                                                        not NULL ecmpLttInfoPtr.
            Expected: GT_OK and the same ecmpLttInfoPtr.
        */
        st = cpssDxChBrgL2EcmpLttTableGet(dev, index, &ecmpLttInfoGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpLttTableGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpStartIndex, ecmpLttInfoGet.ecmpStartIndex,
                       "get another ->ecmpStartIndex than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpNumOfPaths, ecmpLttInfoGet.ecmpNumOfPaths,
                       "get another ->ecmpNumOfPaths than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpEnable, ecmpLttInfoGet.ecmpEnable,
                       "get another ->ecmpEnable than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(ecmpLttInfo.ecmpRandomPathEnable, ecmpLttInfoGet.ecmpRandomPathEnable,
                                                           "get another ->ecmpRandomPathEnable than was set: %d", dev);

        /*
            1.3. Call with out of range index [maxIndex + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */

        index = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        index = 0;

        /*
            1.4. Call with out of range ecmpLttInfoPtr->ecmpStartIndex [0x4000],
                           and other valid params.
            Expected: NON GT_OK.
        */
        ecmpLttInfo.ecmpStartIndex = ECMP_TABLE_SIZE_CNS(dev);

        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        ecmpLttInfo.ecmpStartIndex = 0;

        /*
            1.5. Call with out of range ecmpLttInfoPtr->ecmpNumOfPaths [4K + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        ecmpLttInfo.ecmpNumOfPaths = MAX_NUM_OF_PATH_CNS;

        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.6. Call with out of range ecmpLttInfoPtr->ecmpNumOfPaths [0],
                           and other valid params.
            Expected: NON GT_OK.
        */
        ecmpLttInfo.ecmpNumOfPaths = 0;

        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        ecmpLttInfo.ecmpNumOfPaths = 1;
        /*
            1.7. Call with ecmpLttInfoPtr [NULL],
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ecmpLttInfoPtr = NULL", dev);
    }

    index = 0;

    ecmpLttInfo.ecmpStartIndex = 0;
    ecmpLttInfo.ecmpNumOfPaths = 1;
    ecmpLttInfo.ecmpEnable     = GT_TRUE;
    ecmpLttInfo.ecmpRandomPathEnable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpLttTableSet(dev, index, &ecmpLttInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpLttTableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index [0 / maxIndex],
                   non NULL ecmpLttInfoPtr.
    Expected: GT_OK.
    1.2. Call with out of range index [maxIndex + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.3. Call with ecmpLttInfoPtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      index = 0;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC ecmpLttInfo = {0};
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / maxIndex],
                           non NULL ecmpLttInfoPtr.
            Expected: GT_OK.
        */
        /* call with index = 0 */
        index = 0;

        st = cpssDxChBrgL2EcmpLttTableGet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* call with index = maxIndex */
        index = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev) - 1;

        st = cpssDxChBrgL2EcmpLttTableGet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with out of range index [maxIndex + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        index = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgL2EcmpLttTableGet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        index = 0;

        /*
            1.3. Call with ecmpLttInfoPtr [NULL],
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2EcmpLttTableGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ecmpLttInfoPtr = NULL", dev);
    }

    index = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpLttTableGet(dev, index, &ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpLttTableGet(dev, index, &ecmpLttInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN GT_PORT_NUM                          targetEport,
    IN GT_HW_DEV_NUM                        targetHwDevice
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpTableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index [0 / maxIndex],
                   targetEport [0 / maxEPortNum],
                   targetHwDevice [0 / maxHwDevNum].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgL2EcmpTableGet with same index
                                             not NULL targetEportPtr,
                                             not NULL targetHwDevicePtr.
    Expected: GT_OK and the same targetEport, targetHwDevice.
    1.3. Call with out of range index [maxIndex + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range targetEport [maxEPortNum + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range targetHwDevice [targetHwDevice + 1],
                   and other valid params.
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32          index           = 0;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC ecmpEntry = {0,0};
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC ecmpEntryGet = {0,0};
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / maxIndex],
                           targetEport [0 / maxEPortNum],
                           targetHwDevice [0 / maxHwDevNum].
            Expected: GT_OK.
        */
        /* call with index = 0 */
        index        = 0;
        ecmpEntry.targetEport = 0;
        ecmpEntry.targetHwDevice = 0;

        st = cpssDxChBrgL2EcmpTableSet(dev, index, &ecmpEntry);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, ecmpEntry.targetEport, ecmpEntry.targetHwDevice);

        /*
            1.2. Call cpssDxChBrgL2EcmpTableGet with same index
                                                     not NULL targetEportPtr,
                                                     not NULL targetHwDevicePtr.
            Expected: GT_OK and the same targetEport, targetHwDevice.
        */
        st = cpssDxChBrgL2EcmpTableGet(dev, index, &ecmpEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpTableGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.targetEport, ecmpEntryGet.targetEport,
                       "get another targetEport than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.targetHwDevice, ecmpEntryGet.targetHwDevice,
                       "get another targetHwDevice than was set: %d", dev);

        /* call with index = maxIndex */
        index        = ECMP_TABLE_SIZE_CNS(dev) - 1;
        ecmpEntry.targetEport  = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev);
        ecmpEntry.targetHwDevice = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev);

        st = cpssDxChBrgL2EcmpTableSet(dev, index, &ecmpEntry);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, ecmpEntry.targetEport, ecmpEntry.targetHwDevice);

        /*
            1.2. Call cpssDxChBrgL2EcmpTableGet with same index
                                                     not NULL targetEportPtr,
                                                     not NULL targetHwDevicePtr.
            Expected: GT_OK and the same targetEport, targetHwDevice.
        */
        st = cpssDxChBrgL2EcmpTableGet(dev, index, &ecmpEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpTableGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.targetEport, ecmpEntryGet.targetEport,
                       "get another targetEport than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.targetHwDevice, ecmpEntryGet.targetHwDevice,
                       "get another targetHwDevice than was set: %d", dev);

        /*
            1.3. Call with out of range index [maxIndex + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        index = ECMP_TABLE_SIZE_CNS(dev);

        st = cpssDxChBrgL2EcmpTableSet(dev, index, &ecmpEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        index = 0;

        /*
            1.4. Call with out of range targetEport [maxEPortNum + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        ecmpEntry.targetEport = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev) + 1;

        st = cpssDxChBrgL2EcmpTableSet(dev, index, &ecmpEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        ecmpEntry.targetEport = 0;

        /*
            1.5. Call with out of range targetHwDevice [targetHwDevice + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        ecmpEntry.targetHwDevice = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev) + 1;

        st = cpssDxChBrgL2EcmpTableSet(dev, index, &ecmpEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        ecmpEntry.targetHwDevice = 0;
    }

    index        = 0;
    ecmpEntry.targetEport  = 0;
    ecmpEntry.targetHwDevice = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpTableSet(dev, index, &ecmpEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpTableSet(dev, index, &ecmpEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT GT_PORT_NUM                          *targetEportPtr,
    OUT GT_HW_DEV_NUM                        *targetHwDevicePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpTableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index [0 / maxIndex],
                   non NULL targetEportPtr,
                   non NULL targetHwDevicePtr.
    Expected: GT_OK.
    1.2. Call with out of range index [maxIndex + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.3. Call with targetEportPtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with targetHwDevicePtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32                          index        = 0;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC ecmpEntry = {0,0};
    GT_U32                          notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / maxIndex],
                           non NULL targetEportPtr,
                           non NULL targetHwDevicePtr.
            Expected: GT_OK.
        */
        /* call with index = 0 */
        index = 0;

        st = cpssDxChBrgL2EcmpTableGet(dev, index, &ecmpEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* call with index = maxIndex */
        index = ECMP_TABLE_SIZE_CNS(dev) - 1;

        st = cpssDxChBrgL2EcmpTableGet(dev, index, &ecmpEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with out of range index [maxIndex + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        index = ECMP_TABLE_SIZE_CNS(dev);

        st = cpssDxChBrgL2EcmpTableGet(dev, index, &ecmpEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        index = 0;

        /*
            1.3. Call with ecmpEntry [NULL],
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2EcmpTableGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ecmpEntry = NULL", dev);

    }

    index = 0;
    ecmpEntry.targetEport = 0;
    ecmpEntry.targetHwDevice = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpTableGet(dev, index, &ecmpEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpTableGet(dev, index, &ecmpEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpMemberSelectionModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT      selectionMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpMemberSelectionModeSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with selectionMode [CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E /
                                  CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E /
                                  CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgL2EcmpMemberSelectionModeGet with not NULL selectionModePtr.
    Expected: GT_OK and the same selectionMode.
    1.3. Call with selectionMode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT selectionMode    = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;
    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT selectionModeGet = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with selectionMode [CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E /
                                          CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E /
                                          CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E].
            Expected: GT_OK.
        */
        /* call with selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E */
        selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;

        st = cpssDxChBrgL2EcmpMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, selectionMode);

        /*
            1.2. Call cpssDxChBrgL2EcmpMemberSelectionModeGet with not NULL selectionModePtr.
            Expected: GT_OK and the same selectionMode.
        */
        st = cpssDxChBrgL2EcmpMemberSelectionModeGet(dev, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpMemberSelectionModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
                       "get another selectionMode than was set: %d", dev);

        /* call with selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E */
        selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E;

        st = cpssDxChBrgL2EcmpMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, selectionMode);

        /*
            1.2. Call cpssDxChBrgL2EcmpMemberSelectionModeGet with not NULL selectionModePtr.
            Expected: GT_OK and the same selectionMode.
        */
        st = cpssDxChBrgL2EcmpMemberSelectionModeGet(dev, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpMemberSelectionModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
                       "get another selectionMode than was set: %d", dev);

        /* call with selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E */
        selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E;

        st = cpssDxChBrgL2EcmpMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, selectionMode);

        /*
            1.2. Call cpssDxChBrgL2EcmpMemberSelectionModeGet with not NULL selectionModePtr.
            Expected: GT_OK and the same selectionMode.
        */
        st = cpssDxChBrgL2EcmpMemberSelectionModeGet(dev, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2EcmpMemberSelectionModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
                       "get another selectionMode than was set: %d", dev);

        /*
            1.3. Call with selectionMode [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgL2EcmpMemberSelectionModeSet
                            (dev, selectionMode),
                            selectionMode);
    }

    selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpMemberSelectionModeSet(dev, selectionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpMemberSelectionModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT     *selectionModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpMemberSelectionModeGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non null selectionModePtr.
    Expected: GT_OK.
    1.2. Call with selectionModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null selectionModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2EcmpMemberSelectionModeGet(dev, &selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with selectionModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2EcmpMemberSelectionModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, selectionModePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpMemberSelectionModeGet(dev, &selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpMemberSelectionModeGet(dev, &selectionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non null enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st              = GT_OK;
    GT_U8       dev             = 0;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable          = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2EcmpEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2EcmpEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2EcmpEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2EcmpEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2EcmpEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2EcmpEnableSet)
{
    /*
        ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
        1.1. Call with enable [GT_TRUE/GT_FALSE].
        Expected: GT_OK.
        1.2. Call cpssDxChBrgL2EcmpEnableGet.
        Expected: GT_OK and the same values that was set.
    */

        GT_STATUS   st                  = GT_OK;
        GT_U8       dev                 = 0;
        GT_U32      notAppFamilyBmp;
        GT_BOOL     enable              = GT_FALSE;
        GT_BOOL     enableRet           = GT_FALSE;

        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            /*
                1.1. Call with enable [GT_TRUE/GT_FALSE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgL2EcmpEnableSet(dev, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChBrgL2EcmpEnableGet.
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgL2EcmpEnableGet(dev, &enableRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " %d", dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "get another values than was set: %d",
                                         dev);

            /*
                1.1. Call with enable [GT_TRUE/GT_FALSE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChBrgL2EcmpEnableSet(dev, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChBrgL2EcmpEnableGet.
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgL2EcmpEnableGet(dev, &enableRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " %d", dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "get another values than was set: %d",
                                         dev);
        }

        /* 2. For not active devices check that function returns non GT_OK.*/
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

        /* Go over all non active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssDxChBrgL2EcmpEnableSet(dev, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        /* 3.Call function with out of bound value for device id */
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChBrgL2EcmpEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgL2Ecmp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgL2Ecmp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpIndexBaseEportSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpIndexBaseEportGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpLttTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpLttTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpMemberSelectionModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpMemberSelectionModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2EcmpEnableSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgL2Ecmp)


