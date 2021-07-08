/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssTmDropUT.c
*
* DESCRIPTION:
*       Unit tests for cpssTmShapingUT, that provides
*       TM Shaping facility implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpss/generic/tm/cpssTmNodesUpdate.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


#define MAX_EBS_CBS_CNS             4096


/*******************************************************************************
GT_STATUS cpssTmShapingProfileCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr,
    OUT GT_U32                                  *profileIndPtr
);
*/
UTF_TEST_CASE_MAC(cpssTmShapingProfileCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                    dev;
    CPSS_TM_LEVEL_ENT        level;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStrPtr;
    GT_U32                                  profileIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);




        /*1.1 Check with legal parameters, status[GT_TRUE].
                Expected: GT_OK */

        level= CPSS_TM_LEVEL_A_E;
        profileStrPtr.cirBw = 5000;
        profileStrPtr.cbs = 10;    /*KBytes*/
        profileStrPtr.eirBw = 5000;
        profileStrPtr.ebs = 10; /*KBytes*/


        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmShapingProfileCreate
                            (dev, level, &profileStrPtr, &profileIndPtr) ,level);


       /*
            1.3 check out of profileStrPtr.cirBw.
            Call with profileStrPtr.cirBw [MAX_EBS_CBS_CNS+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileStrPtr.cbs = MAX_EBS_CBS_CNS+10;

        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrPtr.cirBw);

        profileStrPtr.cbs = 0;

        /*
            1.4 check out of profileStrPtr.eirBw.
            Call with profileStrPtr.eirBw [MAX_EBS_CBS_CNS+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileStrPtr.ebs = MAX_EBS_CBS_CNS+10;

        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileStrPtr.eirBw );

        profileStrPtr.ebs = 0;

         /*
            1.5 Call with profileStrPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

         st = cpssTmShapingProfileCreate(dev, level, NULL, &profileIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profileStrPtr = NULL", dev);

        /*
            1.6 Call with profileIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profileIndPtr = NULL", dev);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmShapingProfileDelete
(
    IN  GT_U8   devNum,
    IN  GT_U32  profileInd
);
*/
UTF_TEST_CASE_MAC(cpssTmShapingProfileDelete)
{
    GT_STATUS               st = GT_OK;

    GT_U8                    dev;
    CPSS_TM_LEVEL_ENT        level;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStrPtr;
    GT_U32                                  profileIndPtr = 1;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        level= CPSS_TM_LEVEL_A_E;
        profileStrPtr.cirBw = 5000;
        profileStrPtr.cbs = 10; /*KBytes*/
        profileStrPtr.eirBw = 5000;
        profileStrPtr.ebs = 10; /*KBytes*/


        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters, status[GT_TRUE].
                Expected: GT_OK */


        st = cpssTmShapingProfileDelete(dev, profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

       /*
            1.3 check out of range profileIndPtr.
            Call with profileIndPtr [profInd+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileIndPtr= profileIndPtr+10;

        st = cpssTmShapingProfileDelete(dev, profileIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndPtr);

        profileIndPtr=profileIndPtr-10;



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
        st = cpssTmShapingProfileDelete(dev, profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmShapingProfileDelete(dev, profileIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmShapingProfileRead
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  GT_U32                                  profileInd,
    OUT CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
);
*/
UTF_TEST_CASE_MAC(cpssTmShapingProfileRead)
{
    GT_STATUS               st = GT_OK;

    GT_U8                    dev;
    CPSS_TM_LEVEL_ENT        level;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStrPtr;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStr;
    GT_U32                                  profileIndPtr = 1;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        level= CPSS_TM_LEVEL_A_E;
        profileStr.cirBw = 5000;
        profileStr.cbs = 10;  /*KBytes*/
        profileStr.eirBw = 5000;
        profileStr.ebs = 10; /*KBytes*/


        st = cpssTmShapingProfileCreate(dev, level, &profileStr, &profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters, status[GT_TRUE].
                Expected: GT_OK */


        st = cpssTmShapingProfileRead(dev, level,profileIndPtr, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1.1 Check read values.
                Expected: GT_OK */

        st=GT_OK;
        if (profileStr.cirBw!= profileStrPtr.cirBw ||
            profileStr.cbs != profileStrPtr.cbs ||
            profileStr.eirBw != profileStrPtr.eirBw ||
            profileStr.ebs != profileStrPtr.ebs)
            st= GT_FALSE;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmShapingProfileRead
                            (dev, level,profileIndPtr, &profileStrPtr) ,level);

         /*
            1.3 Call with profileStrPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmShapingProfileRead(dev, level,profileIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profileStrPtr = NULL", dev);


       /*
            1.4 check out of profileIndPtr.
            Call with profileIndPtr [profileIndPtr+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileIndPtr = profileIndPtr+10;

        st = cpssTmShapingProfileRead(dev, level,profileIndPtr, &profileStrPtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        profileIndPtr = profileIndPtr-10;



        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmShapingProfileRead(dev, level,profileIndPtr, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmShapingProfileRead(dev, level,profileIndPtr, &profileStrPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmShapingProfileUpdate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  GT_U32                                  profileInd,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
);
*/

UTF_TEST_CASE_MAC(cpssTmShapingProfileUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                    dev;
    CPSS_TM_LEVEL_ENT        level= CPSS_TM_LEVEL_A_E;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStrPtr;
    GT_U32                                  profileIndPtr = 1;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;

     /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    level= CPSS_TM_LEVEL_A_E;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        level= CPSS_TM_LEVEL_A_E;
        profileStrPtr.cirBw = 5000;
        profileStrPtr.cbs = 10; /*KBytes*/
        profileStrPtr.eirBw = 5000;
        profileStrPtr.ebs = 10; /*KBytes*/


        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters, status[GT_TRUE].
                Expected: GT_OK */

        level= CPSS_TM_LEVEL_A_E;
        profileStrPtr.cirBw = 6000;
        profileStrPtr.cbs = 10; /*KBytes*/
        profileStrPtr.eirBw = 6000;
        profileStrPtr.ebs = 10; /*KBytes*/

        st = cpssTmShapingProfileUpdate(dev, level,profileIndPtr, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmShapingProfileUpdate
                            (dev, level,profileIndPtr, &profileStrPtr) ,level);

         /*
            1.3 Call with profileStrPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmShapingProfileUpdate(dev, level,profileIndPtr, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profileStrPtr = NULL", dev);


       /*
            1.4 check out of profileIndPtr.
            Call with profileIndPtr [profileIndPtr+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profileIndPtr = profileIndPtr+10;

        st = cpssTmShapingProfileUpdate(dev, level,profileIndPtr, &profileStrPtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        profileIndPtr = profileIndPtr-10;



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
        st = cpssTmShapingProfileUpdate(dev, level,profileIndPtr, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmShapingProfileUpdate(dev, level,profileIndPtr, &profileStrPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmShapingProfileValidate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr,
);
*/
UTF_TEST_CASE_MAC(cpssTmShapingProfileValidate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                    dev;
    CPSS_TM_LEVEL_ENT        level;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStrPtr;
    GT_U32                                  profileIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;

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

        level= CPSS_TM_LEVEL_A_E;
        profileStrPtr.cirBw = 5000000;
        profileStrPtr.cbs = 1;    /*KBytes*/
        profileStrPtr.eirBw = 2000000;
        profileStrPtr.ebs = 1; /*KBytes*/

        st = cpssTmShapingProfileValidate(dev, level, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);


        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);




         /* should fail because too little bursts */
        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_SIZE, st, dev);


         /* should fail because too little bursts  and update values in  the shaping profile structure*/
        st = cpssTmShapingProfileValidate(dev, level, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_SIZE, st, dev);

         /* should success because burst sizes are already updated*/
        st = cpssTmShapingProfileValidate(dev, level, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


         /* should success because burst sizes are already updated*/
        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profileIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_A_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmShapingProfileValidate(dev, level, &profileStrPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmShapingProfileValidate(dev, level, &profileStrPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*******************************************************************************
GT_STATUS cpssTmPortShapingValidate
(
    IN GT_U8                 devNum,
    IN GT_U32                cirBw,
    IN GT_U32                eirBw,
    IN GT_U32 *              pcbs,
    IN GT_U32 *              pebs
)
*/

UTF_TEST_CASE_MAC(cpssTmPortShapingValidate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U32                       portInd;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    GT_U32                       cirBw;
    GT_U32                       eirBw;
    GT_U32                       cbs;
    GT_U32                       ebs;


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

        /* Configuration of port params */
        paramsPtr.cirBw = 10000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 5; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        st = cpssTmPortShapingValidate(dev, paramsPtr.cirBw,paramsPtr.eirBw ,&paramsPtr.cbs, &paramsPtr.ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);


        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_SIZE, st, dev);


        st = cpssTmPortShapingValidate(dev, paramsPtr.cirBw,paramsPtr.eirBw ,&paramsPtr.cbs, &paramsPtr.ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_SIZE, st, dev);

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */
        cirBw = 30000000;
        eirBw = 20000000;
        cbs = 5; /*KBytes*/
        ebs = 3; /*KBytes*/

        st = cpssTmPortShapingUpdate(dev, portInd, cirBw, eirBw, cbs, ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_SIZE, st, dev);

        st = cpssTmPortShapingValidate(dev, cirBw,eirBw ,&cbs, &ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_SIZE, st, dev);

        st = cpssTmPortShapingUpdate(dev, portInd, cirBw, eirBw, cbs, ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


       st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cirBw = 10000000;
    eirBw = 0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortShapingValidate(dev, cirBw, eirBw, &cbs, &ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortShapingValidate(dev, cirBw, eirBw, &cbs, &ebs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmShaping suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmShaping)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmShapingProfileUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmShapingProfileRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmShapingProfileDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmShapingProfileCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmShapingProfileValidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortShapingValidate)

UTF_SUIT_END_TESTS_MAC(cpssTmShaping)
