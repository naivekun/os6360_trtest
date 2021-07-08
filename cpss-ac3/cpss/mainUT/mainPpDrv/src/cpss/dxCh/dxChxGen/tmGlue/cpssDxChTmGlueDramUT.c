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
* @file cpssDxChTmGlueDramUT.c
*
* @brief Unit tests for CPSS DXCH TM GLUE Dram APIs
*
* @version   9
********************************************************************************
*/


#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDram.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

CPSS_DXCH_TM_GLUE_DRAM_CFG_STC tmDramInitDefaultUtCfg =
{ /* DB board */
    5,  /* activeInterfaceNum */
    0x0F, /* activeInterfaceMask - DDR interfaces 0-4 are availble for TM */
    {   /*   csBitmask, mirrorEn, dqsSwapEn, ckSwapEn */
        {{   0x1,       GT_TRUE,  GT_FALSE,  GT_FALSE},
         {   0x1,       GT_TRUE,  GT_FALSE,  GT_FALSE},
         {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE},
         {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE}},
        CPSS_DRAM_SPEED_BIN_DDR3_1866M_E, /* speedBin */
        CPSS_DRAM_BUS_WIDTH_16_E,         /* busWidth */
        CPSS_DRAM_512MB_E,                /* memorySize */
        CPSS_DRAM_FREQ_667_MHZ_E,         /* memoryFreq */
        0,                                /* casWL */
        0,                                /* casL */
        CPSS_DRAM_TEMPERATURE_HIGH_E      /* interfaceTemp */
    }
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDramInit
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_TM_GLUE_DRAM_CFG_STC                  *dramCfgPtr,
    IN  CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC            *dramAlgoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueDramInit)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call function with algoType [CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_DYNAMIC_E /
                                      CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_STATIC_E]
    Expected: GT_OK.
    1.2. Call api with wrong algoType [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    CPSS_DXCH_TM_GLUE_DRAM_CFG_STC                  dramCfg;
    CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC            algoParams;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* test stuck CPU - need to fix it in future
    UTF_HW_CRASHED_TEST_BYPASS_MAC(UTF_CAELUM_E); */

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemCpy(&dramCfg, &tmDramInitDefaultUtCfg, sizeof(CPSS_DXCH_TM_GLUE_DRAM_CFG_STC));

#ifdef ASIC_SIMULATION
        algoParams.algoType = CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_STATIC_E;
#else
        algoParams.algoType = CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_DYNAMIC_E;
#endif

        algoParams.performWriteLeveling = GT_TRUE;

        dramCfg.activeInterfaceNum = 0; /* 1 is ok for bobk caelum and cetus */
        st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                "pass on wrong interface number %d, dev %d",
                                dramCfg.activeInterfaceNum, dev);

        dramCfg.activeInterfaceNum = 6;
        st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                "pass on wrong interface number %d, dev %d",
                                dramCfg.activeInterfaceNum, dev);

        dramCfg.activeInterfaceNum = 5;
        dramCfg.interfaceParams.speedBin = CPSS_DRAM_SPEED_BIN_DDR3_800D_E;
        st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                "pass on wrong speed bin %d, dev %d",
                                dramCfg.interfaceParams.speedBin, dev);

        dramCfg.interfaceParams.speedBin = CPSS_DRAM_SPEED_BIN_DDR3_1866M_E;
        dramCfg.interfaceParams.busWidth = CPSS_DRAM_BUS_WIDTH_32_E;
        st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                "pass on wrong bus width %d, dev %d",
                                dramCfg.interfaceParams.busWidth, dev);

        dramCfg.interfaceParams.busWidth = CPSS_DRAM_BUS_WIDTH_16_E;
        dramCfg.interfaceParams.memorySize = CPSS_DRAM_32MB_E;
        st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                "pass on wrong memory size %d, dev %d",
                                dramCfg.interfaceParams.busWidth, dev);

        dramCfg.interfaceParams.memorySize = CPSS_DRAM_512MB_E;
        dramCfg.interfaceParams.memoryFreq = CPSS_DRAM_FREQ_1066_MHZ_E;
        st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                "pass on wrong memory frequency %d, dev %d",
                                dramCfg.interfaceParams.busWidth, dev);
    }

    cpssOsMemCpy(&dramCfg, &tmDramInitDefaultUtCfg, sizeof(CPSS_DXCH_TM_GLUE_DRAM_CFG_STC));

    algoParams.algoType = CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_DYNAMIC_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDramInit(dev, &dramCfg, &algoParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChTmGlueDramInitFlagsSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call function with algoType [CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_DYNAMIC_E /
                                      CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_STATIC_E]
    Expected: GT_OK.
    1.2. Call api with wrong algoType [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_U32                                  savedFlags  = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_IS_DEV_EXISTS_MAC(dev) != 0)
        {
            savedFlags = PRV_CPSS_DXCH_PP_MAC(dev)->extMemory.externalMemoryInitFlags;
        }

        st = cpssDxChTmGlueDramInitFlagsSet(dev, savedFlags);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "regular call");
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDramInitFlagsSet(dev, savedFlags);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDramInitFlagsSet(dev, savedFlags);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTmGlueDram suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTmGlueDram)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDramInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDramInitFlagsSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTmGlueDram)

