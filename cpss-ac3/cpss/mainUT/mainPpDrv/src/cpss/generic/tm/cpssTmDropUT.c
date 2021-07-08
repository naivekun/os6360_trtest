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
* @file cpssTmDropUT.c
*
* @brief Unit tests for cpssTmDropUT.
*
* @version   4
********************************************************************************
*/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*******************************************************************************
GT_STATUS cpssTmDropWredCurveCreate
(
    IN  GT_U8               devNum,
    IN  CPSS_TM_LEVEL_ENT   level,
    IN  GT_U32              cos,
    IN  GT_U32              probabilityArr[CPSS_TM_DROP_PROB_ARR_SIZE_CNS],
    OUT GT_U32              *curvePtr
)
*/
UTF_TEST_CASE_MAC(cpssTmDropWredCurveCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    GT_U32                  cos;
    GT_U32                  probabilityArr[CPSS_TM_DROP_PROB_ARR_SIZE_CNS] = { 0 };
    GT_U32                  curvePtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */
        cpssOsMemSet(probabilityArr,0,sizeof(probabilityArr));

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters
                Expected: GT_OK */
        level= CPSS_TM_LEVEL_A_E;
        cos = 0;

        st = cpssTmDropWredCurveCreate(dev, level, cos, probabilityArr, &curvePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropWredCurveCreate
                            (dev, level, cos, probabilityArr, &curvePtr) ,level);

        /*
            1.3 check out of range cos.
            Call with cos [8], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        level= CPSS_TM_LEVEL_C_E;


        cos = 20;

        st = cpssTmDropWredCurveCreate(dev, level, cos, probabilityArr, &curvePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);

        /*
            1.4 check out of range probabilityArr.
            Call with probabilityArr [102], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cos = 0;
        probabilityArr[0]=102;

        st = cpssTmDropWredCurveCreate(dev, level, cos, probabilityArr, &curvePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, probabilityArr[0]);

        /*
            1.5 Call with curvePtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

       st = cpssTmDropWredCurveCreate(dev, level, cos, probabilityArr, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, curvePtr = NULL", dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.6 Check when TM LIB not initialized.
          Expected: GT_NOT_INITIALIZED */

        /* TM LIB not initialized */
        st = cpssTmDropWredCurveCreate(dev, level, cos, probabilityArr, &curvePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;
    cos = 0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropWredCurveCreate(dev, level, cos, probabilityArr, &curvePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropWredCurveCreate(dev, level, cos, probabilityArr, &curvePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*******************************************************************************
GT_STATUS cpssTmDropWredTraditionalCurveCreate
(
    IN  GT_U8               devNum,
    IN  CPSS_TM_LEVEL_ENT   level,
    IN  GT_U32              cos,
    IN  GT_U32              maxProbability,
    OUT GT_U32              *curvePtr
)
*/
UTF_TEST_CASE_MAC(cpssTmDropWredTraditionalCurveCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    GT_U32                  cos;
    GT_U32                  maxProbability=50;
    GT_U32                  curvePtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */
        level= CPSS_TM_LEVEL_A_E;
        cos = 0;
        maxProbability=50;

        st = cpssTmDropWredTraditionalCurveCreate(dev, level, cos, maxProbability, &curvePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropWredTraditionalCurveCreate
                            (dev, level, cos, maxProbability, &curvePtr) ,level);

        /*
            1.3 check out of range cos.
            Call with cos [8], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        level= CPSS_TM_LEVEL_C_E;
        cos = 10;

        st = cpssTmDropWredTraditionalCurveCreate(dev, level, cos, maxProbability, &curvePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);

        /*
            1.4 check out of range maxProbability.
            Call with maxProbability [0], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cos = 0;
        maxProbability = 0;

        st = cpssTmDropWredTraditionalCurveCreate(dev, level, cos, maxProbability, &curvePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, maxProbability);

        /*
            1.5 Call with curvePtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmDropWredTraditionalCurveCreate(dev, level, cos, maxProbability, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, curvePtr = NULL", dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.6 Check when TM LIB not initialized.
          Expected: GT_NOT_INITIALIZED */

        /* TM LIB not initialized */
        st = cpssTmDropWredTraditionalCurveCreate(dev, level, cos, maxProbability, &curvePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    level= CPSS_TM_LEVEL_A_E;
    cos = 0;
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropWredTraditionalCurveCreate(dev, level, cos, maxProbability, &curvePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropWredTraditionalCurveCreate(dev, level, cos, maxProbability, &curvePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*******************************************************************************
GT_STATUS cpssTmDropProfileCreate
(
    IN  GT_U8                           devNum,
    IN  CPSS_TM_LEVEL_ENT               level,
    IN  GT_U32                          cos,
    IN  CPSS_TM_DROP_PROFILE_PARAMS_STC *profileStrPtr,
    OUT GT_U32                          *profilePtr
);
*/

UTF_TEST_CASE_MAC(cpssTmDropProfileCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    GT_U32                  cos;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrPtr;
    GT_U32                          profilePtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&profileStrPtr, sizeof(profileStrPtr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */


        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        level= CPSS_TM_LEVEL_A_E;
        cos=0;
/*
        profileStrPtr.wredCaTdBw=0;
        profileStrPtr.aqlExponent=0;
        for(ii=0; ii<3; ii++)
        {
            profileStrPtr.curvePtr[ii]=0;
            profileStrPtr.wredMinThresholdRatio[ii]=0;
            profileStrPtr.wredMaxThresholdRatio[ii]=0;
            profileStrPtr.dpRatio[ii]=0;
        }
        profileStrPtr.cbTdBw = 200000;
        profileStrPtr.cbTdThresholdRatio = 80;

        profileStrPtr.wredCaTdMode = CPSS_TM_DROP_MODE_DISABLED_E;

*/
        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
        profileStrPtr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(200000);


        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropProfileCreate
                            (dev, level, cos, &profileStrPtr, &profilePtr) ,level);

        /*
            1.3 check out of range cos.
            Call with cos [8], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        level= CPSS_TM_LEVEL_C_E;

        cos = 9;

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);

        cos=0;

        /*
            1.4 check out of range cbTdThresholdBytes.
            Call with cbTdThresholdBytes  BW(CPSS_TM_MAX_BW_CNS+2), other parameters same as in 1.1.
            Expected: NON GT_OK.

            *************************************************

            Max drop threshold in bytes is greater than  32 bit integer value =7ffff(19 bits)*1024(resolution) * 16 (burst size )
            so following test always will be successful - nothing to validate


        profileStrPtr.cbTdThresholdBytes=cpssTmDropConvertBWToBytes(CPSS_TM_MAX_BW_CNS+2);

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrPtr.cbTdThresholdBytes);

        profileStrPtr.cbTdThresholdBytes=cpssTmDropConvertBWToBytes(200000);


        */

        /*
            CATD mode
        */

        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_CA_TD_E;

        profileStrPtr.caTdDp.caTdThreshold[0]=cpssTmDropConvertBWToBytes(200000);
        profileStrPtr.caTdDp.caTdThreshold[1]=cpssTmDropConvertBWToBytes(200000);
        profileStrPtr.caTdDp.caTdThreshold[2]=cpssTmDropConvertBWToBytes(200000);

       st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5 check out of range wredCaTdBw.
            Call with wredCaTdBw [CPSS_TM_MAX_BW_CNS+2], other parameters same as in 1.1.
            Expected: NON GT_OK.
            *************************************************

            Max drop threshold in bytes is greater than  32 bit integer value =7ffff(19 bits)*1024(resolution) * 16 (burst size )
            so following test always will be successful - nothing to validate

        profileStrPtr.caTdDp.caTdThreshold[0]=cpssTmDropConvertBWToBytes(CPSS_TM_MAX_BW_CNS+2);
        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrPtr.caTdDp.caTdThreshold[0]);

       */

        /*
            1.6 Call with profilePtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profilePtr = NULL", dev);

        /*
            WRED mode
        */

        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E;
        profileStrPtr.caWredDp.aqlExponent=0;
        for(ii=0; ii<3; ii++)
        {
            profileStrPtr.caWredDp.curveIndex[ii]=0;
            profileStrPtr.caWredDp.dpCurveScale[ii]=0;
            profileStrPtr.caWredDp.caWredTdMinThreshold[ii]=0;
            profileStrPtr.caWredDp.caWredTdMaxThreshold[ii]=0;
        }

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.7 check out of range aqlExponent.
            Call with aqlExponent [CPSS_TM_MAX_BW_CNS+2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileStrPtr.caWredDp.aqlExponent=0xff;

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrPtr.caWredDp.aqlExponent);

        profileStrPtr.caWredDp.aqlExponent=0;

        /*
            1.8 Check for wrong enum values wredCaTdMode and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropProfileCreate
                            (dev, level, cos, &profileStrPtr, &profilePtr) ,profileStrPtr.dropMode);

        /*
            1.9 check out of range curvePtr.
            Call with curvePtr [9], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileStrPtr.caWredDp.curveIndex[0]=9;

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrPtr.caWredDp.curveIndex[0]);

        profileStrPtr.caWredDp.curveIndex[0] =0;


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;
    cos=0;
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmDropProfileRead
(
    IN GT_U8                                devNum,
    IN CPSS_TM_LEVEL_ENT                    level,
    IN GT_U32                               cos,
    IN GT_U32                               profileInd,
    OUT CPSS_TM_DROP_PROFILE_PARAMS_STC     *profileStrPtr
);
*/

UTF_TEST_CASE_MAC(cpssTmDropProfileRead)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    GT_U32                  cos;
    CPSS_TM_DROP_PROFILE_PARAMS_STC wredProfileStr;
    GT_U32                          wredProfileInd;
    CPSS_TM_DROP_PROFILE_PARAMS_STC caProfileStr;
    GT_U32                          caProfileInd;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrPtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U8                   color_num;
    GT_U8                   resolution;

     /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&profileStrPtr, sizeof(profileStrPtr));
    cpssOsBzero((GT_VOID*)&wredProfileStr, sizeof(wredProfileStr));
    cpssOsBzero((GT_VOID*)&caProfileStr, sizeof(caProfileStr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        level= CPSS_TM_LEVEL_C_E;
        cos=0;


        wredProfileStr.dropMode = CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E;
        wredProfileStr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(200000);
        wredProfileStr.caWredDp.aqlExponent=0;
        for(ii=0; ii<3; ii++)
        {
            wredProfileStr.caWredDp.curveIndex[ii]=0;
            wredProfileStr.caWredDp.dpCurveScale[ii]=0;
            wredProfileStr.caWredDp.caWredTdMinThreshold[ii]=cpssTmDropConvertBWToBytes(1000);
            wredProfileStr.caWredDp.caWredTdMaxThreshold[ii]=cpssTmDropConvertBWToBytes(1000*(ii+1));
        }


        st = cpssTmDropProfileCreate(dev, level, cos, &wredProfileStr, &wredProfileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        caProfileStr.dropMode = CPSS_TM_DROP_MODE_CB_TD_CA_TD_E;
        caProfileStr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(100000000);
        for(ii=0; ii<3; ii++)
        {
            caProfileStr.caTdDp.caTdThreshold[ii]=cpssTmDropConvertBWToBytes(1000*(ii+1));
        }

        st = cpssTmDropProfileCreate(dev, level, cos, &caProfileStr, &caProfileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st =cpssTmDropColorNumResolutionGet(dev, level, &color_num, &resolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(2, color_num, dev);

        st = cpssTmDropProfileRead(dev, level, cos, 0, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (profileStrPtr.cbTdThresholdBytes != CPSS_TM_MAX_DROP_THRESHOLD_CNS) st= GT_FAIL;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


         st = cpssTmDropProfileRead(dev, level, cos, wredProfileInd, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
       /*
            1.1.1 Check the read values
                    Expected: GT_OK
        */
        st= GT_OK;
        if (wredProfileStr.dropMode!= profileStrPtr.dropMode ) st= GT_FAIL;

        if (st == GT_OK )
        {
            if (wredProfileStr.cbTdThresholdBytes != profileStrPtr.cbTdThresholdBytes
            || wredProfileStr.caWredDp.aqlExponent!= profileStrPtr.caWredDp.aqlExponent
            )
            st= GT_FAIL;
        }

        if (st == GT_OK )
        {
            for(ii=0; ii<3; ii++)
            {
                if (wredProfileStr.caWredDp.curveIndex[ii]!= profileStrPtr.caWredDp.curveIndex[ii]
                    || wredProfileStr.caWredDp.caWredTdMinThreshold[ii]!= profileStrPtr.caWredDp.caWredTdMinThreshold[ii]
                    || wredProfileStr.caWredDp.caWredTdMaxThreshold[ii]!= profileStrPtr.caWredDp.caWredTdMaxThreshold[ii]
                    )
                    st= GT_FAIL;
            }
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmDropProfileRead(dev, level, cos, caProfileInd, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st= GT_OK;
        if (caProfileStr.dropMode!= profileStrPtr.dropMode)  st= GT_FAIL;
        if (st== GT_OK)
        {
            if (caProfileStr.cbTdThresholdBytes != profileStrPtr.cbTdThresholdBytes)  st= GT_FAIL;
        }

        if (st== GT_OK)
        {
            for(ii=0; ii<3; ii++)
            {
                if (caProfileStr.caTdDp.caTdThreshold[ii] != profileStrPtr.caTdDp.caTdThreshold[ii]) st= GT_FAIL;
            }
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropProfileRead
                            (dev, level, cos, wredProfileInd, &profileStrPtr) ,level);

        /*
            1.3 check out of range cos.
            Call with cos [8], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        level= CPSS_TM_LEVEL_C_E;
        cos = 9;

        st = cpssTmDropProfileRead(dev, level, cos, wredProfileInd, &profileStrPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);

        cos=0;


        /*
            1.5 Call with profileStrPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

         st = cpssTmDropProfileRead(dev, level, cos, wredProfileInd, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profileStrPtr = NULL", dev);

        /*
            1.6 check out of range profileInd.
            Call with profileInd [260], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        wredProfileInd=wredProfileInd+260;

       st = cpssTmDropProfileRead(dev, level, cos, wredProfileInd, &profileStrPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrPtr.cbTdThresholdBytes);

        wredProfileInd=wredProfileInd-260;



        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;
    cos=0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropProfileRead(dev, level, cos, wredProfileInd, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropProfileRead(dev, level, cos, wredProfileInd, &profileStrPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmDropProfileDelete
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    IN GT_U32                   cos,
    IN GT_U32                   profile
);
*/

UTF_TEST_CASE_MAC(cpssTmDropProfileDelete)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    GT_U32                  cos;
    GT_U32                          profileInd;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrPtr;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&profileStrPtr, sizeof (profileStrPtr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */


        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        level= CPSS_TM_LEVEL_A_E;
        cos=0;

        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
        profileStrPtr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(200000);

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmDropProfileDelete(dev, level, cos, profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profileInd);

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropProfileDelete
                            (dev, level, cos, profileInd) ,level);

        /*
            1.3 check out of range cos.
            Call with cos [8], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */
        level= CPSS_TM_LEVEL_C_E;
        cos = 9;

        st = cpssTmDropProfileDelete(dev, level, cos, profileInd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);

        cos=0;

        /*
            1.4 check out of range profileInd.
            Call with profileInd [260], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileInd=profileInd+260;

       st = cpssTmDropProfileDelete(dev, level, cos, profileInd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileInd);

        profileInd=profileInd-260;



        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;
    cos=0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropProfileDelete(dev, level, cos, profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropProfileDelete(dev, level, cos, profileInd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmDropColorNumSet
(
    IN GT_U8                        devNum,
    IN CPSS_TM_LEVEL_ENT            level,
    IN CPSS_TM_COLOR_NUM_ENT        number
);
*/

UTF_TEST_CASE_MAC(cpssTmDropColorNumSet)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    CPSS_TM_COLOR_NUM_ENT   number;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        level = CPSS_TM_LEVEL_A_E;
        number = CPSS_TM_COLOR_NUM_1_E;

        st = cpssTmDropColorNumSet(dev, level, number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropColorNumSet
                            (dev, level, number) ,level);

        /*
            1.3 Check for wrong enum values number and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropColorNumSet
                            (dev, level, number) ,number);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;
    number= CPSS_TM_COLOR_NUM_1_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropColorNumSet(dev, level, number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropColorNumSet(dev, level, number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmAgingChangeStatus
(
    IN GT_U8         devNum,
    IN GT_U32        status
);
GT_STATUS cpssTmAgingStatusGet
(
    IN GT_U8         devNum,
    IN GT_U32        *status
);

*/
UTF_TEST_CASE_MAC(cpssTmAgingChangeStatus)
{
    GT_STATUS               st = GT_OK;

    GT_U8         dev;
    GT_U32        status= GT_TRUE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32        readed_status;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */


        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        status= GT_TRUE;

        st = cpssTmAgingChangeStatus(dev, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmAgingStatusGet(dev, &readed_status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(readed_status, status, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        status= GT_FALSE;

        st = cpssTmAgingChangeStatus(dev, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmAgingStatusGet(dev, &readed_status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(readed_status, status, dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmAgingChangeStatus(dev, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmAgingStatusGet(dev, &readed_status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAgingChangeStatus(dev, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmAgingStatusGet(dev, &readed_status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmDpSourceSet
(
    IN GT_U8                   devNum,
    IN CPSS_TM_LEVEL_ENT       level,
    IN GT_U32                  color,
    IN CPSS_TM_DP_SOURCE_ENT   source
);
GT_STATUS cpssTmDpSourceGet
(
    IN GT_U8                   devNum,
    IN CPSS_TM_LEVEL_ENT       level,
    IN GT_U32                  color,
    OUT CPSS_TM_DP_SOURCE_ENT  *source
);
*/
UTF_TEST_CASE_MAC(cpssTmDpSourceSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_TM_LEVEL_ENT       level = CPSS_TM_LEVEL_Q_E;
    GT_U32                  color = 0;
    CPSS_TM_DP_SOURCE_ENT   source = CPSS_TM_DP_SOURCE_AQL_E;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_TM_DP_SOURCE_ENT   source_get ;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        level = CPSS_TM_LEVEL_B_E;
        color = 1;
        source = CPSS_TM_DP_SOURCE_QL_E;

        st = cpssTmDpSourceSet(dev, level, color, source);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        level = CPSS_TM_LEVEL_A_E;
        color = 0;
        source = CPSS_TM_DP_SOURCE_AQL_E;

        st = cpssTmDpSourceSet(dev, level, color, source);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmDpSourceGet(dev, level, color, &source_get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(source, source_get, dev);

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDpSourceSet
                            (dev, level, color, source), level);

        /*
            1.3 Check for wrong enum values number and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDpSourceSet
                            (dev, level, color, source), source);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDpSourceSet(dev, level, color, source);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmDpSourceGet(dev, level, color, &source_get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDpSourceSet(dev, level, color, source);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmDpSourceGet(dev, level, color, &source_get);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmDropQueryResponceSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portDp,
    IN CPSS_TM_LEVEL_ENT       level
);
GT_STATUS cpssTmDropQueryResponceGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  * portDp,
    IN CPSS_TM_LEVEL_ENT       * level
);
*/
UTF_TEST_CASE_MAC(cpssTmDropQueryResponceSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32                  portDp = 0;
    CPSS_TM_LEVEL_ENT       level = CPSS_TM_LEVEL_Q_E;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  portDp_out;
    CPSS_TM_LEVEL_ENT       level_out;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        portDp = 1;
        level = CPSS_TM_LEVEL_B_E;

        st = cpssTmDropQueryResponceSet(dev, portDp, level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmDropQueryResponceGet(dev, &portDp_out, &level_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(level, level_out, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(portDp, portDp_out, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        portDp = 0;
        level = CPSS_TM_LEVEL_P_E;

        st = cpssTmDropQueryResponceSet(dev, portDp, level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmDropQueryResponceGet(dev, &portDp_out, &level_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(level, level_out, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(portDp, portDp_out, dev);


        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropQueryResponceSet
                            (dev, portDp, level), level);

        /*
            1.3 Check for wrong enum values number and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        portDp = 2;

        st = cpssTmDropQueryResponceSet(dev, portDp, level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        portDp = 0;

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropQueryResponceSet(dev, portDp, level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmDropQueryResponceGet(dev, &portDp_out, &level_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropQueryResponceSet(dev, portDp, level);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmDropQueryResponceGet(dev, &portDp_out, &level_out);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmDropQueueCosSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  index,
    IN GT_U32                  cos
);
GT_STATUS cpssTmDropQueueCosGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  index,
    IN GT_U32                  *cos
);
*/
UTF_TEST_CASE_MAC(cpssTmDropQueueCosSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      index = 0;
    GT_U32      cos = 0;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_TM_TREE_PARAMS_STC tmTreeParams;
    GT_U32      cos_out;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        index = 1;
        cos = 2;

        st = cpssTmDropQueueCosSet(dev, index, cos);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmDropQueueCosGet(dev, index, &cos_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(cos,cos_out, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmTreeParamsGet(dev, &tmTreeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = tmTreeParams.maxQueues-1;  /*16K/4K;*/
        cos = 4;

        st = cpssTmDropQueueCosSet(dev, index, cos);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmDropQueueCosGet(dev, index, &cos_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(cos,cos_out, dev);

        /*
            1.2 Check for wrong enum values number and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        cos = 8;

        st = cpssTmDropQueueCosSet(dev, index, cos);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        cos = 0;
        index += 2;
        st = cpssTmDropQueueCosSet(dev, index, cos);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        st = cpssTmDropQueueCosGet(dev, index, &cos_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropQueueCosSet(dev, index, cos);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmDropQueueCosGet(dev, index, &cos_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropQueueCosSet(dev, index, cos);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmDropQueueCosGet(dev, index, &cos_out);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmDropProfileAgingBlockCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_DROP_PROFILE_PARAMS_STC         profileStcArr[CPSS_TM_AGING_PROFILES_CNS],
    OUT GT_U32                                  *profileIndexPtr
);
*/

UTF_TEST_CASE_MAC(cpssTmDropProfileAgingBlockCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrArr[CPSS_TM_AGING_PROFILES_CNS];
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                          profilePtr;
    GT_U32                          nextProfilePtr;
     GT_U32                  j;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileForRead;
    GT_U32      cos = 0;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrPtr;
    GT_U32                          dummyProfilePtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)profileStrArr, sizeof(profileStrArr));
    cpssOsBzero((GT_VOID*)&profileForRead, sizeof(profileForRead));
    cpssOsBzero((GT_VOID*)&profileStrPtr, sizeof(profileStrPtr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */
        for (j=0; j<CPSS_TM_AGING_PROFILES_CNS; j++)
        {
            profileStrArr[j].dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
            profileStrArr[j].cbTdThresholdBytes = cpssTmDropConvertBWToBytes(200000);
        }


        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* test aging granularity  :   slot 0 is occupied by default drop , so aging profile range should start from 4 */
        UTF_VERIFY_EQUAL1_PARAM_MAC(4,profilePtr, dev);

        for (j=0; j<CPSS_TM_AGING_PROFILES_CNS; j++)
        {
            profileStrArr[j].dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
            profileStrArr[j].cbTdThresholdBytes = cpssTmDropConvertBWToBytes(300000 - j*50000);
        }
        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &nextProfilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* test aging granularity  :   next aging profile should start from 8 */
        UTF_VERIFY_EQUAL1_PARAM_MAC(8,nextProfilePtr, dev);

        /* next created regular drop profile sholld be created in first free interval : 1::3, i.e 1 */


        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
        profileStrPtr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(100000);
        st = cpssTmDropProfileCreate(dev, CPSS_TM_LEVEL_Q_E , 0, &profileStrPtr, &dummyProfilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(1,dummyProfilePtr, dev);



        /* test reading content of  aging profile */
        for (j=0; j<CPSS_TM_AGING_PROFILES_CNS; j++)
        {
            st = cpssTmDropProfileRead(dev, CPSS_TM_LEVEL_Q_E, cos, nextProfilePtr + j, &profileForRead);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1 Check the read values
                        Expected: GT_OK
            */
            st= GT_OK;
            if ( profileStrArr[j].dropMode!= profileForRead.dropMode
              || profileStrArr[j].cbTdThresholdBytes!= profileForRead.cbTdThresholdBytes
              )
                st= GT_FAIL;

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        }

         /* try to delete one of aging profile as a regular profile using  delete API for regular drop profiles - should be error */
        st = cpssTmDropProfileDelete(dev, CPSS_TM_LEVEL_Q_E, cos, profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);

         /* try to delete one of profiles of aging profile - should be error */
        st = cpssTmDropProfileDelete(dev, CPSS_TM_LEVEL_Q_E, cos, profilePtr+1);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);
        st = cpssTmDropProfileDelete(dev, CPSS_TM_LEVEL_Q_E, cos, profilePtr+2);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);
        st = cpssTmDropProfileDelete(dev, CPSS_TM_LEVEL_Q_E, cos, profilePtr+3);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cos);
        /* delete aging profile */
        st = cpssTmDropProfileAgingBlockDelete(dev, profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



        /*
            1.2 check out of range cbTdThresholdBytes.
            Call with cbTdThresholdBytes = BW(CPSS_TM_MAX_BW_CNS+2), other parameters same as in 1.1.
            Expected: NON GT_OK.
            ************************************************************************************************
            Max drop threshold in bytes is greater than  32 bit integer value =7ffff(19 bits)*1024(resolution) * 16 (burst size )
            so following test always will be successful - nothing to validate

         profileStrArr[0].cbTdThresholdBytes=cpssTmDropConvertBWToBytes(CPSS_TM_MAX_BW_CNS+2);

        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrArr[0].cbTdThresholdBytes);

        profileStrArr[0].cbTdThresholdBytes =0;
       */


        /*
            1.3 Call with profilePtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profilePtr = NULL", dev);


       /*
            1.5 Check for wrong enum values wredCaTdMode and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmDropProfileAgingBlockCreate
                            (dev, profileStrArr, &profilePtr) ,profileStrArr[0].dropMode);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &profilePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmDropProfileAgingBlockDelete
(
    IN GT_U8                    devNum,
    IN GT_U32                   profileIndex
);
*/

UTF_TEST_CASE_MAC(cpssTmDropProfileAgingBlockDelete)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    GT_U32                          profileInd;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrArr[CPSS_TM_AGING_PROFILES_CNS];
    GT_U32                  j;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)profileStrArr, sizeof(profileStrArr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */
        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        for (j=0; j<CPSS_TM_AGING_PROFILES_CNS; j++)
        {
            profileStrArr[j].dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
            profileStrArr[j].cbTdThresholdBytes = cpssTmDropConvertBWToBytes(300000 - j*50000);
        }


        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */


        st = cpssTmDropProfileAgingBlockDelete(dev,profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &profileInd);

        /*
            1.2 check out of range profileInd.
            Call with profileInd [260], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileInd=profileInd+260;

        st = cpssTmDropProfileAgingBlockDelete(dev,profileInd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileInd);

        profileInd=profileInd-260;



        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropProfileAgingBlockDelete(dev,profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropProfileAgingBlockDelete(dev,profileInd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*******************************************************************************
GT_STATUS cpssTmDropProfileAgingUpdate
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileIndex,
    IN  GT_U32                                  profileOffset,
    IN  CPSS_TM_DROP_PROFILE_PARAMS_STC         *profileParamsPtr
);
*/

UTF_TEST_CASE_MAC(cpssTmDropProfileAgingUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    GT_U32                          profileInd;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrArr[CPSS_TM_AGING_PROFILES_CNS];
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileForUpdate;
    GT_U32                          dummyProfile;
    GT_U32                  j;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)profileStrArr, sizeof(profileStrArr));
    cpssOsBzero((GT_VOID*)&profileForUpdate, sizeof(profileForUpdate));


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */
        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        for (j=0; j<CPSS_TM_AGING_PROFILES_CNS; j++)
        {
            profileStrArr[j].dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
            profileStrArr[j].cbTdThresholdBytes = cpssTmDropConvertBWToBytes(300000 - j*50000);
        }


        st = cpssTmDropProfileAgingBlockCreate(dev, profileStrArr, &profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */



        profileForUpdate.dropMode = CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E;
        profileForUpdate.caWredDp.aqlExponent=0;
        for(j=0; j <3; j++)
        {
            profileForUpdate.caWredDp.curveIndex[j]=0;
            profileForUpdate.caWredDp.dpCurveScale[j]=0;
            profileForUpdate.caWredDp.caWredTdMinThreshold[j]=0;
            profileForUpdate.caWredDp.caWredTdMaxThreshold[j]=0;
        }

        profileForUpdate.dropMode = CPSS_TM_DROP_MODE_CB_TD_CA_TD_E;
        profileForUpdate.caTdDp.caTdThreshold[0]=cpssTmDropConvertBWToBytes(200000);
        profileForUpdate.caTdDp.caTdThreshold[1]=cpssTmDropConvertBWToBytes(200000);
        profileForUpdate.caTdDp.caTdThreshold[2]=cpssTmDropConvertBWToBytes(200000);



        st = cpssTmDropProfileAgingUpdate(dev,profileInd, 0 , &profileForUpdate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        profileForUpdate.dropMode = CPSS_TM_DROP_MODE_CB_TD_CA_TD_E;
        profileForUpdate.caTdDp.caTdThreshold[0]=cpssTmDropConvertBWToBytes(200000);
        profileForUpdate.caTdDp.caTdThreshold[1]=cpssTmDropConvertBWToBytes(200000);
        profileForUpdate.caTdDp.caTdThreshold[2]=cpssTmDropConvertBWToBytes(200000);

        st = cpssTmDropProfileAgingUpdate(dev,profileInd, 2 , &profileForUpdate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range profileInd.
            Call with profileInd [260], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileInd=profileInd+260;

        st = cpssTmDropProfileAgingUpdate(dev,profileInd, 0 , &profileForUpdate);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileInd);

        profileInd=profileInd-260;

        /*
            1.3 check out of range offset.
            Call with offset 6, other parameters same as in 1.1.
            Expected: NON GT_OK.
        */
        st = cpssTmDropProfileAgingUpdate(dev,profileInd, 6 , &profileForUpdate);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileInd);

        /*
            1.4 ctry to update not aging profile
            Expected: NON GT_OK.
        */
        st = cpssTmDropProfileCreate(dev, CPSS_TM_LEVEL_Q_E , 0, &profileForUpdate, &dummyProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmDropProfileAgingUpdate(dev,dummyProfile, 1 , &profileForUpdate);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, dummyProfile);



        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmDropProfileAgingBlockDelete(dev,profileInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmDropProfileAgingBlockDelete(dev,profileInd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmDrop)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropWredCurveCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropWredTraditionalCurveCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropProfileCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropProfileRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropProfileDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropColorNumSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAgingChangeStatus)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDpSourceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropQueryResponceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropQueueCosSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropProfileAgingBlockCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropProfileAgingBlockDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmDropProfileAgingUpdate)
UTF_SUIT_END_TESTS_MAC(cpssTmDrop)

