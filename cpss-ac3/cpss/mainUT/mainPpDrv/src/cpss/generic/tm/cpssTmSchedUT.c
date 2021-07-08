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
* @file cpssTmSchedUT.c
*
* @brief Unit tests for cpssTmSchedUT, that provides
* TM Scheduling facility implementation.
*
* @version   3
********************************************************************************
*/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpss/generic/tm/cpssTmNodesUpdate.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssTmSchedPortExternalBpSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   portExtBp
)
*/
UTF_TEST_CASE_MAC(cpssTmSchedPortExternalBpSet)
{
/*                                                                                     .
    ITERATE_DEVICES (DxChx)
    1.1. Check supported portExtBp.
    Call with portExtBp [GT_FALSE/GT_TRUE].
    Expected: GT_OK                       .
    1.2. Call cpssTmSchedPortExternalBpGet with non-NULL portExtBpPtr.
    Expected: GT_OK and same value as written.                                      .
    1.3. Call with out of range portExtBp.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8                   dev;
    GT_BOOL                 portExtBp;
#if 0
    GT_BOOL                 portExtBpRet;
#endif
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

        /*
          1.1. Check supported portExtBp. Call with portExtBp [GT_FALSE/GT_TRUE].
          Expected: GT_OK.
        */
        portExtBp = GT_FALSE;

        st = cpssTmSchedPortExternalBpSet(dev, portExtBp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portExtBp);

        portExtBp = GT_TRUE;

        st = cpssTmSchedPortExternalBpSet(dev, portExtBp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portExtBp);

        /*
           TBD: Read from HW?                                                                    .
           1.2. Call cpssTmSchedPortExternalBpGet with non-NULL portExtBpPtr.
           Expected: GT_OK and same value as written.
        */
#if 0
        st = cpssTmSchedPortExternalBpGet(dev, &portExtBpRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssTmSchedPortExternalBpGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(portExtBp, portExtBpRet,
         "cpssTmSchedPortExternalBpGet: get another portExtBp than was set: dev = %d", dev);
#endif
        /*
           1.3. Call with wrong enum values portExtBp.
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmSchedPortExternalBpSet
                            (dev, portExtBp),
                            portExtBp);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.4 Check when TM LIB not initialized.
                Expected: GT_NOT_INITIALIZED */

        st = cpssTmSchedPortExternalBpSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
    }

#if 0 /* TBD */
    portExtBp = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmSchedPortExternalBpSet(dev, portExtBp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmSchedPortExternalBpSet(dev, portExtBp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
#endif
}

UTF_TEST_CASE_MAC(cpssTmSchedPeriodicSchemeConfig)
{
    /*
    ITERATE_DEVICES
    1.1. Call cpssTmSchedPeriodicSchemeConfig with relevant value
    Expected: GT_OK.
    1.2. Call with out of range minBw[10000], maxBw[5000]
         minBw should be less or equal to maxBW
    Expected: NOT GT_OK.
    1.3. Call with out of range increamentalBw[10000], minBW[5000]
         increamentalBw should be less or equal to minBW
    Expected: NOT GT_OK.
    1.4. Call with wrong value minBW[50000], increamentalBw[40000]
         min bw should be a multiple of inc bw
    Expected: NOT GT_OK.

 */
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_TM_LEVEL_PERIODIC_PARAMS_STC    periodicSchemeArray[UT_TM_MAX_LEVELS_CNS];

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

        /*set default values*/
        periodicSchemeArray[CPSS_TM_LEVEL_Q_E].periodicState = GT_FALSE;
        periodicSchemeArray[CPSS_TM_LEVEL_Q_E].shaperDecoupling = GT_FALSE;

        periodicSchemeArray[CPSS_TM_LEVEL_A_E].periodicState = GT_TRUE;
        periodicSchemeArray[CPSS_TM_LEVEL_A_E].shaperDecoupling = GT_FALSE;

        periodicSchemeArray[CPSS_TM_LEVEL_B_E].periodicState = GT_TRUE;
        periodicSchemeArray[CPSS_TM_LEVEL_B_E].shaperDecoupling = GT_FALSE;

        periodicSchemeArray[CPSS_TM_LEVEL_C_E].periodicState = GT_TRUE;
        periodicSchemeArray[CPSS_TM_LEVEL_C_E].shaperDecoupling = GT_FALSE;

        periodicSchemeArray[CPSS_TM_LEVEL_P_E].periodicState = GT_TRUE;
        periodicSchemeArray[CPSS_TM_LEVEL_P_E].shaperDecoupling = GT_FALSE;

        /* 1.1 */
        st = cpssTmSchedPeriodicSchemeConfig(dev, periodicSchemeArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* close*/
        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.5 Check when TM LIB not initialized.
                Expected: GT_NOT_INITIALIZED */

        st = cpssTmSchedPeriodicSchemeConfig(dev, periodicSchemeArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

    }

#if 0 /* TBD */
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmSchedPeriodicSchemeConfig(dev, periodicSchemeArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmSchedPeriodicSchemeConfig(dev, periodicSchemeArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
#endif
}

/*******************************************************************************
GT_STATUS cpssTmShapingPeriodicUpdateEnableSet
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    IN GT_BOOL                  status
);
*/

UTF_TEST_CASE_MAC(cpssTmShapingPeriodicUpdateEnableSet)
{
    GT_STATUS               st = GT_OK;

    GT_U8                    dev;
    CPSS_TM_LEVEL_ENT        level;
    GT_BOOL                  status= GT_TRUE;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStrPtr;
    GT_U32                                  profInd;
    CPSS_TM_LEVEL_PERIODIC_PARAMS_STC    periodicStatus[UT_TM_MAX_LEVELS_CNS];


    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E , "JIRA:6794");

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
        profileStrPtr.cbs = 10;    /*KBytes*/
        profileStrPtr.eirBw = 5000;
        profileStrPtr.ebs = 10; /*KBytes*/

        st = cpssTmShapingProfileCreate(dev, level, &profileStrPtr, &profInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



        st = cpssTmShapingPeriodicUpdateStatusGet(dev,periodicStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters, status[GT_TRUE].
                Expected: GT_OK */

        level= CPSS_TM_LEVEL_A_E;


        status =GT_FALSE;
        st = cpssTmShapingPeriodicUpdateEnableSet(dev, level, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmShapingPeriodicUpdateStatusGet(dev,periodicStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(status, periodicStatus[CPSS_TM_LEVEL_A_E].periodicState, dev);


        status =GT_TRUE;
        st = cpssTmShapingPeriodicUpdateEnableSet(dev, level, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmShapingPeriodicUpdateStatusGet(dev,periodicStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(status, periodicStatus[CPSS_TM_LEVEL_A_E].periodicState, dev);

        /*1.2 Check with legal parameters, status[GT_FALSE].
                Expected: GT_OK */


        status =GT_FALSE;
        st = cpssTmShapingPeriodicUpdateEnableSet(dev, level, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmShapingPeriodicUpdateEnableSet
                            (dev, level, status) ,level);


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
        st = cpssTmShapingPeriodicUpdateEnableSet(dev, level, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmShapingPeriodicUpdateEnableSet(dev, level, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------
GT_STATUS cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   bytes
);
*/
UTF_TEST_CASE_MAC(cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    GT_U32                  bytes = 0;
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

        bytes= 0;

        st = cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet(dev, bytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 check out of range bytes (should be <= 0x7f).
            Call with bytes [0x8F], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bytes = 0x8F;

        st = cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet(dev, bytes);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bytes);

        bytes=0;

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.3 Check when TM LIB not initialized.
                Expected: GT_NOT_INITIALIZED */

        st = cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet(dev, bytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    bytes=0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet(dev, bytes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet(dev, bytes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------
GT_STATUS cpssTmPortQuantumLimitsGet
(
    IN GT_U8                         devNum,
    OUT CPSS_TM_QUANTUM_LIMITS_STC  * cpssQuantumLimits
);

*/
UTF_TEST_CASE_MAC(cpssTmPortQuantumLimitsGet)
{
    GT_STATUS                       st = GT_OK;

    GT_U8                           dev;
    GT_U32                          queueInd;
    GT_U32                          aNodeInd;
    GT_U32                          bNodeInd;
    GT_U32                          cNodeInd;
    GT_U32                          portInd;
    CPSS_TM_PORT_PARAMS_STC         portParams;
    CPSS_TM_PORT_DROP_PER_COS_STC   portDropParams;
    CPSS_TM_QUANTUM_LIMITS_STC      cpssQuantumLimits;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;

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

        st = cpssTmTestTreeBuild(dev, &queueInd, &aNodeInd, &bNodeInd, &cNodeInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortQuantumLimitsGet(dev, &cpssQuantumLimits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        portInd = 1;
        st = cpssTmPortConfigurationRead(dev, portInd, &portParams, &portDropParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
#ifdef ASIC_SIMULATION
        /* currently simulator have troubles reading HiWord  of register , port quantum values must be updated here for default values explicitly */
        portParams.quantumArr[0] = 40;
        portParams.quantumArr[1] = 40;
        portParams.quantumArr[2] = 40;
        portParams.quantumArr[3] = 40;
        portParams.quantumArr[4] = 40;
        portParams.quantumArr[5] = 40;
        portParams.quantumArr[6] = 40;
        portParams.quantumArr[7] = 40;
#endif
        /* verify min and max quantum */
        portParams.quantumArr[0] = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution);
        portParams.quantumArr[1] = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution);

        /*
        cpssOsPrintf("cpssTmPortQuantumLimitsGet: minQuantum: %d, maxQuantum: %d\n",
                     (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution),
                     (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution));
        */
        portInd = 2;
        st = cpssTmPortCreate(dev, portInd, &portParams, 1, 1, 1, 1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify wrong min quantums */

        portParams.quantumArr[0] = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution) - 1;
        portInd = 3;
        st = cpssTmPortCreate(dev, portInd, &portParams, 1, 1, 1, 1);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify wrong max quantums */
        portParams.quantumArr[1] = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution) + 1;
        portInd = 4;
        st = cpssTmPortCreate(dev, portInd, &portParams, 1, 1, 1, 1);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.2 Check when TM LIB not initialized.
                Expected: GT_NOT_INITIALIZED */

         st = cpssTmPortQuantumLimitsGet(dev, &cpssQuantumLimits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortQuantumLimitsGet(dev, &cpssQuantumLimits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortQuantumLimitsGet(dev, &cpssQuantumLimits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------
GT_STATUS cpssTmNodeQuantumLimitsGet
(
    IN GT_U8                         devNum,
    OUT CPSS_TM_QUANTUM_LIMITS_STC  * cpssQuantumLimits
);

*/
UTF_TEST_CASE_MAC(cpssTmNodeQuantumLimitsGet)
{
    GT_STATUS                   st = GT_OK;

    GT_U8                       dev;

    CPSS_TM_A_NODE_PARAMS_STC   aParams;
    CPSS_TM_B_NODE_PARAMS_STC   bParams;
    CPSS_TM_C_NODE_PARAMS_STC   cParams;
    CPSS_TM_QUEUE_PARAMS_STC    qParams;


    GT_U32                      aNodeInd;
    GT_U32                      bNodeInd;
    GT_U32                      cNodeInd;
    GT_U32                      queueInd;

    CPSS_TM_QUANTUM_LIMITS_STC  cpssQuantumLimits;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

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

        st = cpssTmNodeQuantumLimitsGet(dev, &cpssQuantumLimits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        cpssOsPrintf("cpssTmNodeQuantumLimitsGet: minQuantum: %d, maxQuantum: %d\n",
                     (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution),
                     (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution));
        */

        st = cpssTmTestTreeBuild(dev, &queueInd, &aNodeInd, &bNodeInd, &cNodeInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        /* verify min and max quantum C level */
        st = cpssTmCnodeConfigurationRead(dev, cNodeInd, &cParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cParams.quantum = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution);
        st = cpssTmCnodeUpdate(dev, cNodeInd, &cParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cParams.quantum = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution);
        st = cpssTmCnodeUpdate(dev, cNodeInd, &cParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify min and max quantum B level */
        st = cpssTmBnodeConfigurationRead(dev, bNodeInd, &bParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        bParams.quantum = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution);
        st = cpssTmBnodeUpdate(dev, bNodeInd, &bParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        bParams.quantum = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution);
        st = cpssTmBnodeUpdate(dev, bNodeInd, &bParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify min and max quantum A level */
        st = cpssTmAnodeConfigurationRead(dev, aNodeInd, &aParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        aParams.quantum = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution);
        st = cpssTmAnodeUpdate(dev, aNodeInd, &aParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        aParams.quantum = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution);
        st = cpssTmAnodeUpdate(dev, aNodeInd, &aParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify min and max quantum Q level */
        st = cpssTmQueueConfigurationRead(dev, queueInd, &qParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        qParams.quantum = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution);
        st = cpssTmQueueUpdate(dev, queueInd, &qParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        qParams.quantum = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution);
        st = cpssTmQueueUpdate(dev, queueInd, &qParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify wrong min quantums */
        cParams.quantum = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution) - 1;
        st = cpssTmCnodeUpdate(dev, cNodeInd, &cParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify wrong max quantums */
        qParams.quantum = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution) + 1;
        st = cpssTmQueueUpdate(dev, queueInd, &qParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.2 Check when TM LIB not initialized.
                Expected: GT_NOT_INITIALIZED */

         st = cpssTmNodeQuantumLimitsGet(dev, &cpssQuantumLimits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmNodeQuantumLimitsGet(dev, &cpssQuantumLimits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNodeQuantumLimitsGet(dev, &cpssQuantumLimits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmSched suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmSched)
   /* UTF_SUIT_DECLARE_TEST_MAC(cpssTmBasic)*/
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmSchedPortExternalBpSet)
    /*UTF_SUIT_DECLARE_TEST_MAC(cpssTmTreeChangeStatus)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmTestMaxPortsCreate)*/
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmSchedPeriodicSchemeConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmShapingPeriodicUpdateEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortQuantumLimitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNodeQuantumLimitsGet)
UTF_SUIT_END_TESTS_MAC(cpssTmSched)

