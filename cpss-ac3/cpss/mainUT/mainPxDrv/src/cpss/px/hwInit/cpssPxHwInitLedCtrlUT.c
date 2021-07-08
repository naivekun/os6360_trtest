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
* @file cpssPxHwInitLedCtrlUT.c
*
* @brief Unit tests for PX LED API
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>

#define PRV_TGF_MEMBS_NUM(_val) (sizeof(_val) / sizeof(_val[0]))
static CPSS_PX_LED_CLASS_MANIPULATION_STC prvTgfClassManipulationConfigs[] =
{
    {GT_FALSE, CPSS_LED_BLINK_SELECT_0_E, GT_FALSE, 0x0, GT_FALSE, GT_FALSE},
    {GT_TRUE,  CPSS_LED_BLINK_SELECT_0_E, GT_FALSE, 0x0, GT_FALSE, GT_FALSE},
    {GT_FALSE, CPSS_LED_BLINK_SELECT_1_E, GT_FALSE, 0x0, GT_FALSE, GT_FALSE},
    {GT_FALSE, CPSS_LED_BLINK_SELECT_0_E, GT_TRUE,  0x0, GT_FALSE, GT_FALSE},
    {GT_FALSE, CPSS_LED_BLINK_SELECT_0_E, GT_FALSE, 0x1, GT_FALSE, GT_FALSE},
    {GT_FALSE, CPSS_LED_BLINK_SELECT_0_E, GT_FALSE, 0x0, GT_TRUE,  GT_FALSE},
    {GT_FALSE, CPSS_LED_BLINK_SELECT_0_E, GT_FALSE, 0x0, GT_FALSE, GT_TRUE }
};

static CPSS_PX_LED_CONF_STC prvTgfLedConfig[] =
{
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_CLASS_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_3_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_7_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
     {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_3_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_7_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_7_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_TRUE, CPSS_LED_CLOCK_OUT_FREQUENCY_500_E},
    {CPSS_LED_ORDER_MODE_BY_PORT_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_BLINK_DUTY_CYCLE_0_E, CPSS_LED_BLINK_DURATION_0_E,
     CPSS_LED_PULSE_STRETCH_0_NO_E, 0, 20,
     GT_FALSE, CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E}
};

static CPSS_LED_GROUP_CONF_STC prvTgfLedGroupCfg[] =
{
    {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x1},
    {0x0, 0x0, 0x1, 0x0},
    {0x0, 0x1, 0x0, 0x0},
    {0x1, 0x0, 0x0, 0x0}
};

UTF_TEST_CASE_MAC(cpssPxLedStreamClassManipulationSet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid class
    Expected: GT_BAD_PARAM
    1.3. Call with invalid pointer to classParamsPtr
    Expected: GT_BAD_PTR
    1.4. Call with invalid blinkSelect
    Expected: GT_BAD_PARAM
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_LED_CLASS_MANIPULATION_STC manCfgGet;
    GT_U32      class;
    GT_U32      ii;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(class = 0; class < 6; class++)
        {
            for (ii = 0; ii < PRV_TGF_MEMBS_NUM(prvTgfClassManipulationConfigs);
                 ii++)
            {
                st = cpssPxLedStreamClassManipulationSet(dev, class,
                                                         &prvTgfClassManipulationConfigs[ii]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, class);

                st = cpssPxLedStreamClassManipulationGet(dev, class, &manCfgGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, class);
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfClassManipulationConfigs[ii].blinkEnable,
                    manCfgGet.blinkEnable
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfClassManipulationConfigs[ii].blinkSelect,
                    manCfgGet.blinkSelect
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfClassManipulationConfigs[ii].disableOnLinkDown,
                    manCfgGet.disableOnLinkDown
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfClassManipulationConfigs[ii].forceData,
                    manCfgGet.forceData
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfClassManipulationConfigs[ii].pulseStretchEnable,
                    manCfgGet.pulseStretchEnable
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfClassManipulationConfigs[ii].forceEnable,
                    manCfgGet.forceEnable
                );
            }
        }

        /*  1.2. Call with invalid class
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamClassManipulationSet(dev, 6,
            &prvTgfClassManipulationConfigs[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 6);

        /*  1.3. Call with invalid pointer to classParamsPtr
            Expected: GT_BAD_PTR */
        st = cpssPxLedStreamClassManipulationSet(dev, 4, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, 4);

        /*  1.4. Call with invalid blinkSelect
            Expected: GT_BAD_PARAM */
        manCfgGet = prvTgfClassManipulationConfigs[0];
        manCfgGet.blinkSelect = 2;
        st = cpssPxLedStreamClassManipulationSet(dev, 4, &manCfgGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 4);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamClassManipulationSet(dev, 5,
            &prvTgfClassManipulationConfigs[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, 5);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamClassManipulationSet(dev, 5,
        &prvTgfClassManipulationConfigs[0]);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 5);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamClassManipulationGet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid class
    Expected: GT_BAD_PARAM
    1.3. Call with invalid pointer to classParamsPtr
    Expected: GT_BAD_PTR
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_LED_CLASS_MANIPULATION_STC manCfgGet;
    GT_U32      class;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(class = 0; class < 6; class++)
        {
            st = cpssPxLedStreamClassManipulationGet(dev, class, &manCfgGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, class);
        }

        /*  1.2. Call with invalid class
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamClassManipulationSet(dev, 6, &manCfgGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 6);

        /*  1.3. Call with invalid pointer to classParamsPtr
            Expected: GT_BAD_PTR */
        st = cpssPxLedStreamClassManipulationSet(dev, 4, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, 4);

    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamClassManipulationSet(dev, 5, &manCfgGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, 5);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamClassManipulationSet(dev, 5, &manCfgGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 5);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamConfigSet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid pointer to ledConfPtr
    Expected: GT_BAD_PTR
    1.3. Call with invalid pulseStretch
    Expected: GT_BAD_PARAM
    1.4. Call with invalid led clock frequency
    Expected: GT_BAD_PARAM
    1.5. Call with invalid LED order mode
    Expected: GT_BAD_PARAM
    1.6. Call with invalid blink duration
    Expected: GT_BAD_PARAM
    1.7. Call with invalid blink duty cycle
    Expected: GT_BAD_PARAM
    1.8. Call with invalid ledStart..ledEnd
    Expected: GT_BAD_PARAM
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_LED_CONF_STC ledConfGet;
    CPSS_PX_LED_CONF_STC ledConf;
    GT_U32      ii;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(ii = 0; ii < PRV_TGF_MEMBS_NUM(prvTgfLedConfig); ii++)
        {
            st = cpssPxLedStreamConfigSet(dev, &prvTgfLedConfig[ii]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssPxLedStreamConfigGet(dev, &ledConfGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].blink0Duration,
                ledConfGet.blink0Duration
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].blink0DutyCycle,
                ledConfGet.blink0DutyCycle
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].blink1Duration,
                ledConfGet.blink1Duration
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].blink1DutyCycle,
                ledConfGet.blink1DutyCycle
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].ledClockFrequency,
                ledConfGet.ledClockFrequency
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].pulseStretch,
                ledConfGet.pulseStretch
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].ledStart,
                ledConfGet.ledStart
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].ledEnd,
                ledConfGet.ledEnd
            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                prvTgfLedConfig[ii].ledOrganize,
                ledConfGet.ledOrganize
            );
        }

        /*  1.2. Call with invalid pointer to ledConfPtr
            Expected: GT_BAD_PTR */
        st = cpssPxLedStreamConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3. Call with invalid pulseStretch
            Expected: GT_BAD_PARAM */
        ledConf = prvTgfLedConfig[0];
        ledConf.pulseStretch = 8;
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*  1.4. Call with invalid led clock frequency
            Expected: GT_BAD_PARAM */
        ledConf = prvTgfLedConfig[0];
        ledConf.ledClockFrequency = 4;
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*  1.5. Call with invalid LED order mode
            Expected: GT_BAD_PARAM */
        ledConf = prvTgfLedConfig[0];
        ledConf.ledOrganize = 2;
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*  1.6. Call with invalid blink duration
            Expected: GT_BAD_PARAM */
        ledConf = prvTgfLedConfig[0];
        ledConf.blink0Duration = 8;
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*  1.7. Call with invalid blink duty cycle
            Expected: GT_BAD_PARAM */
        ledConf = prvTgfLedConfig[0];
        ledConf.blink0DutyCycle = 4;
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*  1.8. Call with invalid ledStart..ledEnd
            Expected: GT_BAD_PARAM */
        ledConf = prvTgfLedConfig[0];
        ledConf.ledEnd = 5;
        ledConf.ledStart = 10;
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ledConf = prvTgfLedConfig[0];
        ledConf.ledEnd = 258;
        ledConf.ledStart = 280;
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }
    ledConf = prvTgfLedConfig[0];

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamConfigSet(dev, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamConfigSet(dev, &ledConf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamConfigGet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid pointer to ledConfPtr
    Expected: GT_BAD_PTR
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_LED_CONF_STC ledConfGet;
    GT_U32      ii;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(ii = 0; ii < PRV_TGF_MEMBS_NUM(prvTgfLedConfig); ii++)
        {
            st = cpssPxLedStreamConfigGet(dev, &ledConfGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*  1.2. Call with invalid pointer to ledConfPtr
            Expected: GT_BAD_PTR */
        st = cpssPxLedStreamConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamConfigGet(dev, &ledConfGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamConfigGet(dev, &ledConfGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamGroupConfigSet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid pointer to groupParamsPtr
    Expected: GT_BAD_PTR
    1.3. Call with invalid groupParamsPtr
    Expected: GT_OUT_OF_RANGE
    1.4. Call with invalid groupNum
    Expected: GT_BAD_PARAM
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_LED_GROUP_CONF_STC groupConfGet;
    CPSS_LED_GROUP_CONF_STC groupConf;
    GT_U32      ii;
    GT_U32      group;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for (group = 0; group <= 1; group++)
        {
            for (ii = 0; ii < PRV_TGF_MEMBS_NUM(prvTgfLedGroupCfg); ii++)
            {
                st = cpssPxLedStreamGroupConfigSet(dev, group,
                    &prvTgfLedGroupCfg[ii]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);

                st = cpssPxLedStreamGroupConfigGet(dev, group, &groupConfGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);

                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfLedGroupCfg[ii].classA,
                    groupConfGet.classA
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfLedGroupCfg[ii].classB,
                    groupConfGet.classB
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfLedGroupCfg[ii].classC,
                    groupConfGet.classC
                );
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    prvTgfLedGroupCfg[ii].classD,
                    groupConfGet.classD
                );
            }
        }

        /*  1.2. Call with invalid pointer to groupParamsPtr
            Expected: GT_BAD_PTR */
        st = cpssPxLedStreamGroupConfigSet(dev, 0, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, 0);

        /*  1.3. Call with invalid groupParamsPtr
            Expected: GT_OUT_OF_RANGE */
        groupConf = prvTgfLedGroupCfg[0];
        groupConf.classA = 8;
        st = cpssPxLedStreamGroupConfigSet(dev, 0, &groupConf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, 0);

        /*  1.4. Call with invalid groupNum
            Expected: GT_BAD_PARAM */
        groupConf = prvTgfLedGroupCfg[0];
        st = cpssPxLedStreamGroupConfigSet(dev, 2, &groupConf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 2);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamGroupConfigSet(dev, 0, &groupConf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 0);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamGroupConfigSet(dev, 0, &groupConf);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 0);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamGroupConfigGet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid pointer to groupParamsPtr
    Expected: GT_BAD_PTR
    1.3. Call with invalid groupNum
    Expected: GT_BAD_PARAM
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_LED_GROUP_CONF_STC groupConfGet;
    GT_U32      ii;
    GT_U32      group;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for (group = 0; group <= 1; group++)
        {
            for (ii = 0; ii < PRV_TGF_MEMBS_NUM(prvTgfLedGroupCfg); ii++)
            {
                st = cpssPxLedStreamGroupConfigGet(dev, group, &groupConfGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);
            }
        }

        /*  1.2. Call with invalid pointer to groupParamsPtr
            Expected: GT_BAD_PTR */
        st = cpssPxLedStreamGroupConfigGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, 0);

        /*  1.3. Call with invalid groupNum
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamGroupConfigGet(dev, 2, &groupConfGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 2);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamGroupConfigGet(dev, 0, &groupConfGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 0);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamGroupConfigGet(dev, 0, &groupConfGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 0);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamPortPositionSet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid port number
    Expected: GT_BAD_PARAM
    1.3. Call with invalid position
    Expected: GT_OUT_OF_RANGE
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      ii;
    GT_PHYSICAL_PORT_NUM      port = 0;
    GT_U32      positions[] = {0, 8, 16, 24, 32, 40, 48, 56, 63};
    GT_U32      positionGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for (port = 0; port <= 12; port++)
        {
            for (ii = 0; ii < PRV_TGF_MEMBS_NUM(positions); ii++)
            {
                st = cpssPxLedStreamPortPositionSet(dev, port,
                    positions[ii]);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port,
                    positions[ii]);

                st = cpssPxLedStreamPortPositionGet(dev, port, &positionGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }
        port = 0;

        /*  1.2. Call with invalid port number
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamPortPositionSet(dev, 17, positions[0]);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 17, positions[0]);

        /*  1.3. Call with invalid position
            Expected: GT_OUT_OF_RANGE */
        st = cpssPxLedStreamPortPositionSet(dev, port, 64);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, 64);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamPortPositionSet(dev, port, positions[0]);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port,
            positions[0]);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamPortPositionSet(dev, port, positions[0]);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, positions[0]);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamPortPositionGet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid port number
    Expected: GT_BAD_PARAM
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM      port = 0;
    GT_U32      positionGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for (port = 0; port <= 12; port++)
        {
            st = cpssPxLedStreamPortPositionGet(dev, port, &positionGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        port = 0;

        /*  1.2. Call with invalid port number
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamPortPositionGet(dev, 17, &positionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 17);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamPortPositionGet(dev, port, &positionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamPortPositionGet(dev, port, &positionGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamPortClassPolarityInvertEnableSet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid port number
    Expected: GT_BAD_PARAM
    1.3. Call with invalid class number
    Expected: GT_BAD_PARAM
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM      port = 0;
    GT_BOOL     invertEnableGet;
    GT_BOOL     invertEnable;
    GT_U32      class = 0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(port = 0; port <= 12; port++)
        {
            for(class = 0; class < 6; class++)
            {
                for(invertEnable = GT_FALSE; invertEnable < GT_TRUE; invertEnable++)
                {
                    st = cpssPxLedStreamPortClassPolarityInvertEnableSet(
                        dev, port, class, invertEnable);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, class,
                        invertEnable);

                    st = cpssPxLedStreamPortClassPolarityInvertEnableGet(
                        dev, port, class, &invertEnableGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, class);
                }
            }
        }
        port = 0;
        class = 0;

        /*  1.2. Call with invalid port number
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamPortClassPolarityInvertEnableSet(dev, 17, class,
            GT_FALSE);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, 17, class, GT_FALSE);

        /*  1.3. Call with invalid class number
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamPortClassPolarityInvertEnableSet(dev, port, 6,
            GT_FALSE);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, 6, GT_FALSE);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamPortClassPolarityInvertEnableSet(dev, port, class,
            GT_FALSE);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port,
            class, GT_FALSE);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamPortClassPolarityInvertEnableSet(dev, port, class,
        GT_FALSE);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port,
        class, GT_FALSE);
}

UTF_TEST_CASE_MAC(cpssPxLedStreamPortClassPolarityInvertEnableGet)
{
    /*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters
    Expected: GT_OK.
    1.2. Call with invalid port number
    Expected: GT_BAD_PARAM
    1.3. Call with invalid class number
    Expected: GT_BAD_PARAM
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM      port = 0;
    GT_BOOL     invertEnableGet;
    GT_U32      class = 0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(port = 0; port <= 12; port++)
        {
            for(class = 0; class < 6; class++)
            {
                st = cpssPxLedStreamPortClassPolarityInvertEnableGet(
                    dev, port, class, &invertEnableGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, class);
            }
        }
        port = 0;
        class = 0;

        /*  1.2. Call with invalid port number
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamPortClassPolarityInvertEnableGet(
            dev, 17, class, &invertEnableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 17, class);

        /*  1.3. Call with invalid class number
            Expected: GT_BAD_PARAM */
        st = cpssPxLedStreamPortClassPolarityInvertEnableGet(
            dev, port, 6, &invertEnableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, 6);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxLedStreamPortClassPolarityInvertEnableGet(
            dev, port, class, &invertEnableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port,
            class);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxLedStreamPortClassPolarityInvertEnableGet(
        dev, port, class, &invertEnableGet);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, class);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxHwInitDeviceMatrix suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxHwInitLedCtrl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamClassManipulationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamClassManipulationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamGroupConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamGroupConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamPortPositionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamPortPositionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamPortClassPolarityInvertEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxLedStreamPortClassPolarityInvertEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssPxHwInitLedCtrl)

