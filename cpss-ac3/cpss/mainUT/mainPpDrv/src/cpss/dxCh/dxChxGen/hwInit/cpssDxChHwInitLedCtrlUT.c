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
* @file cpssDxChHwInitLedCtrlUT.c
*
* @brief Unit tests for cpssDxChHwInitLedCtrlUT, that provides
* CPSS DXCH level basic Hw initialization functions.
*
* @version   21
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/* Valid port num value used for testing */
#define HWINIT_VALID_PORTNUM_CNS         0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamClassAndGroupConfig
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ledControl,
    IN  CPSS_DXCH_LED_CLASS_CONF_STC        *ledClassConfPtr,
    IN  GT_U32                              ledClassNum,
    IN  CPSS_DXCH_LED_GROUP_CONF_STC        *ledGroupConfPtr,
    IN  GT_U32                              ledGroupNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamClassAndGroupConfig)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with descSizePtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                              ledControl = 0;
    CPSS_DXCH_LED_CLASS_CONF_STC        ledClassConf[2];
    GT_U32                              ledClassNum = 0;
    CPSS_DXCH_LED_GROUP_CONF_STC        ledGroupConf[2];
    GT_U32                              ledGroupNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)ledClassConf, sizeof(ledClassConf));
    cpssOsBzero((GT_VOID*)ledGroupConf, sizeof(ledGroupConf));
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChLedStreamClassAndGroupConfig(dev, ledControl, ledClassConf,
                                        ledClassNum, ledGroupConf, ledGroupNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



        /*
            1.2. Call with descSizePtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamClassAndGroupConfig(dev, ledControl, ledClassConf,
                                        ledClassNum, ledGroupConf, ledGroupNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamClassAndGroupConfig(dev, ledControl, ledClassConf,
                                    ledClassNum, ledGroupConf, ledGroupNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamHyperGStackTxQStatusEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamHyperGStackTxQStatusEnableSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above exclude Lion)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamHyperGStackTxQStatusEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChLedStreamHyperGStackTxQStatusEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamHyperGStackTxQStatusEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamHyperGStackTxQStatusEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamHyperGStackTxQStatusEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChLedStreamHyperGStackTxQStatusEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamHyperGStackTxQStatusEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamHyperGStackTxQStatusEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamHyperGStackTxQStatusEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamHyperGStackTxQStatusEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamHyperGStackTxQStatusEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamHyperGStackTxQStatusEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamHyperGStackTxQStatusEnableGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above exclude Lion)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChLedStreamHyperGStackTxQStatusEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamHyperGStackTxQStatusEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamHyperGStackTxQStatusEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamHyperGStackTxQStatusEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamClassIndicationSet)
{
/*
    ITERATE_DEVICES (xCat and above exclude Lion)
    1.1. Call function with ledInterfaceNum[0 / 1], classNum[9 / 10 / 11],
                     indication[CPSS_DXCH_LED_INDICATION_PRIMARY_E /
                                CPSS_DXCH_LED_INDICATION_P_REJ_E /
                                CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E].
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamClassIndicationGet with the same parameters
    Expected: GT_OK and the same values.
    1.3. Call with wrong ledInterfaceNum [2].
    Expected: GT_BAD_PARAM
    1.4. Call with wrong classNum [8 / 12].
    Expected: GT_BAD_PARAM
    1.5. Call with wrong enum values indication.
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_U32      ledInterfaceNum = 0;
    GT_U32      classNum        = 0;
    CPSS_DXCH_LED_INDICATION_ENT indication = CPSS_DXCH_LED_INDICATION_FC_RX_E;
    CPSS_DXCH_LED_INDICATION_ENT indicationGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ledInterfaceNum[0 / 1], classNum[9 / 10 / 11],
                     indication[CPSS_DXCH_LED_INDICATION_PRIMARY_E /
                                CPSS_DXCH_LED_INDICATION_P_REJ_E /
                                CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E].
            Expected: GT_OK.
        */

        for(ledInterfaceNum = 0; ledInterfaceNum < 2; ledInterfaceNum++)
            for(classNum = 9; classNum < 12; classNum++)
            {
                /* call with indication = CPSS_DXCH_LED_INDICATION_PRIMARY_E */
                indication = CPSS_DXCH_LED_INDICATION_PRIMARY_E;

                st = cpssDxChLedStreamClassIndicationSet(dev,
                                        ledInterfaceNum, classNum, indication);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                                        ledInterfaceNum, classNum, indication);
                if(GT_OK == st)
                {
                    /*
                        1.2. Call cpssDxChLedStreamClassIndicationGet
                        Expected: GT_OK and the same indicationPtr.
                    */
                    st = cpssDxChLedStreamClassIndicationGet(dev,
                                        ledInterfaceNum, classNum, &indicationGet);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChLedStreamClassIndicationGet: %d %d %d %d",
                                    dev, ledInterfaceNum, classNum, indicationGet);
                    UTF_VERIFY_EQUAL5_STRING_MAC(indication, indicationGet,
                        "get another indication than was set: %d %d %d %d %d",
                        dev, ledInterfaceNum, classNum, indication, indicationGet);
                }

                /* call with indication = CPSS_DXCH_LED_INDICATION_P_REJ_E */
                indication = CPSS_DXCH_LED_INDICATION_P_REJ_E;

                st = cpssDxChLedStreamClassIndicationSet(dev,
                                        ledInterfaceNum, classNum, indication);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                                        ledInterfaceNum, classNum, indication);
                if(GT_OK == st)
                {
                    /*
                        1.2. Call cpssDxChLedStreamClassIndicationGet
                        Expected: GT_OK and the same indicationPtr.
                    */
                    st = cpssDxChLedStreamClassIndicationGet(dev,
                                        ledInterfaceNum, classNum, &indicationGet);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChLedStreamClassIndicationGet: %d %d %d %d",
                                    dev, ledInterfaceNum, classNum, indicationGet);
                    UTF_VERIFY_EQUAL5_STRING_MAC(indication, indicationGet,
                        "get another indication than was set: %d %d %d %d %d",
                        dev, ledInterfaceNum, classNum, indication, indicationGet);
                }

                /* call with indication = CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E */
                indication = CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E;

                st = cpssDxChLedStreamClassIndicationSet(dev,
                                        ledInterfaceNum, classNum, indication);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                                        ledInterfaceNum, classNum, indication);
                if(GT_OK == st)
                {
                    /*
                        1.2. Call cpssDxChLedStreamClassIndicationGet
                        Expected: GT_OK and the same indicationPtr.
                    */
                    st = cpssDxChLedStreamClassIndicationGet(dev,
                                        ledInterfaceNum, classNum, &indicationGet);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChLedStreamClassIndicationGet: %d %d %d %d",
                                    dev, ledInterfaceNum, classNum, indicationGet);
                    UTF_VERIFY_EQUAL5_STRING_MAC(indication, indicationGet,
                        "get another indication than was set: %d %d %d %d %d",
                        dev, ledInterfaceNum, classNum, indication, indicationGet);
                }
            }

        /*restore value*/
        ledInterfaceNum = 0;
        classNum        = 9;
        indication = CPSS_DXCH_LED_INDICATION_FC_RX_E;

        /*
            1.3. Call with wrong ledInterfaceNum [2].
            Expected: GT_BAD_PARAM.
        */
        ledInterfaceNum = 2;

        st = cpssDxChLedStreamClassIndicationSet(dev,
                                ledInterfaceNum, classNum, indication);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                ledInterfaceNum, classNum, indication);

        /*restore value*/
        ledInterfaceNum = 0;

        /*
            1.4. Call with wrong classNum [8 / 12].
            Expected: GT_BAD_PARAM
        */

        /*call with classNum = 8*/
        classNum = 8;
        st = cpssDxChLedStreamClassIndicationSet(dev,
                                ledInterfaceNum, classNum, indication);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                ledInterfaceNum, classNum, indication);

        /*call with classNum = 12*/
        classNum = 12;
        st = cpssDxChLedStreamClassIndicationSet(dev,
                                ledInterfaceNum, classNum, indication);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                ledInterfaceNum, classNum, indication);


        /*restore value*/
        classNum = 9;

        /*
            1.5. Call with wrong enum values indication.
            Expected: GT_BAD_PARAM
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamClassIndicationSet
                            (dev, ledInterfaceNum, classNum, indication),
                            indication);
    }

    /*restore value*/
    ledInterfaceNum = 0;
    classNum        = 9;
    indication = CPSS_DXCH_LED_INDICATION_FC_RX_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamClassIndicationSet(dev,
                                ledInterfaceNum, classNum, indication);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamClassIndicationSet(dev,
                            ledInterfaceNum, classNum, indication);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamClassIndicationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          classNum,
    OUT CPSS_DXCH_LED_INDICATION_ENT   *indicationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamClassIndicationGet)
{
/*
    ITERATE_DEVICES (xCat and above exclude Lion)
    1.1. Call function with ledInterfaceNum[0 / 1], classNum[9 / 10 / 11],
         and not null indicationPtr.
    Expected: GT_OK.
    1.2. Call with wrong ledInterfaceNum [2].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong classNum [8 / 12].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong indicationPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_U32      ledInterfaceNum = 0;
    GT_U32      classNum        = 0;
    CPSS_DXCH_LED_INDICATION_ENT indicationGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ledInterfaceNum[0 / 1], classNum[9 / 10 / 11],
                 and not null indicationPtr.
            Expected: GT_OK.
        */

        for(ledInterfaceNum = 0; ledInterfaceNum < 2; ledInterfaceNum++)
            for(classNum = 9; classNum < 12; classNum++)
            {
                st = cpssDxChLedStreamClassIndicationGet(dev,
                                    ledInterfaceNum, classNum, &indicationGet);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChLedStreamClassIndicationGet: %d %d %d %d",
                                dev, ledInterfaceNum, classNum, indicationGet);
            }

        /*restore value*/
        ledInterfaceNum = 0;
        classNum        = 9;

        /*
            1.2. Call with wrong ledInterfaceNum [2].
            Expected: GT_BAD_PARAM.
        */
        ledInterfaceNum = 2;

        st = cpssDxChLedStreamClassIndicationGet(dev,
                                ledInterfaceNum, classNum, &indicationGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, ledInterfaceNum, classNum);

        /*restore value*/
        ledInterfaceNum = 0;

        /*
            1.3. Call with wrong classNum [8 / 12].
            Expected: GT_BAD_PARAM
        */

        /*call with classNum = 8*/
        classNum = 8;
        st = cpssDxChLedStreamClassIndicationGet(dev,
                                ledInterfaceNum, classNum, &indicationGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, ledInterfaceNum, classNum);

        /*call with classNum = 12*/
        classNum = 12;
        st = cpssDxChLedStreamClassIndicationGet(dev,
                                ledInterfaceNum, classNum, &indicationGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, ledInterfaceNum, classNum);

        /*restore value*/
        classNum = 9;

        /*
            1.4. Call with wrong indicationPtr.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChLedStreamClassIndicationGet(dev, ledInterfaceNum, classNum, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, ledInterfaceNum, classNum);
    }

    /*restore value*/
    ledInterfaceNum = 0;
    classNum        = 9;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamClassIndicationGet(dev,
                                ledInterfaceNum, classNum, &indicationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamClassIndicationGet(dev,
                            ledInterfaceNum, classNum, &indicationGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamCpuOrPort27ModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_LED_CPU_OR_PORT27_ENT indicatedPort
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamCpuOrPort27ModeSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above exclude Lion)
    1.1. Call function with indicatedPort [CPSS_DXCH_LED_CPU_E /
                                           CPSS_DXCH_LED_PORT27_E].
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamCpuOrPort27ModeGet
    Expected: GT_OK and the same indicatedPortPtr.
    1.3. Call with wrong enum values indicatedPort.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_LED_CPU_OR_PORT27_ENT indicatedPort     = CPSS_DXCH_LED_CPU_E;
    CPSS_DXCH_LED_CPU_OR_PORT27_ENT indicatedPortGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with indicatedPort [CPSS_DXCH_LED_CPU_E /
                                                   CPSS_DXCH_LED_PORT27_E].
            Expected: GT_OK.
        */
        /* Call with indicatedPort [GT_FALSE] */
        indicatedPort = GT_FALSE;

        st = cpssDxChLedStreamCpuOrPort27ModeSet(dev, indicatedPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, indicatedPort);

        if (GT_OK == st)
        {
            /*
                1.2. Call cpssDxChLedStreamCpuOrPort27ModeGet
                Expected: GT_OK and the same indicatedPortPtr.
            */
            st = cpssDxChLedStreamCpuOrPort27ModeGet(dev, &indicatedPortGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChLedStreamCpuOrPort27ModeGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(indicatedPort, indicatedPortGet,
                           "get another indicatedPortPtr than was set: %d", dev);
        }

        /* Call with indicatedPort [GT_TRUE] */
        indicatedPort = GT_TRUE;

        st = cpssDxChLedStreamCpuOrPort27ModeSet(dev, indicatedPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, indicatedPort);

        if (GT_OK == st)
        {
            /*
                1.2. Call cpssDxChLedStreamCpuOrPort27ModeGet
                Expected: GT_OK and the same indicatedPortPtr.
            */
            st = cpssDxChLedStreamCpuOrPort27ModeGet(dev, &indicatedPortGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChLedStreamCpuOrPort27ModeGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(indicatedPort, indicatedPortGet,
                           "get another indicatedPortPtr than was set: %d", dev);
        }

        /*
            1.3. Call with wrong enum values indicatedPort.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamCpuOrPort27ModeSet
                            (dev, indicatedPort),
                            indicatedPort);
    }

    indicatedPort = CPSS_DXCH_LED_CPU_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamCpuOrPort27ModeSet(dev, indicatedPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamCpuOrPort27ModeSet(dev, indicatedPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamCpuOrPort27ModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_LED_CPU_OR_PORT27_ENT *indicatedPortPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamCpuOrPort27ModeGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above exclude Lion)
    1.1. Call with non-null indicatedPortPtr.
    Expected: GT_OK.
    1.2. Call with null indicatedPortPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_LED_CPU_OR_PORT27_ENT indicatedPort;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null indicatedPortPtr.
            Expected: GT_OK.
        */
        st = cpssDxChLedStreamCpuOrPort27ModeGet(dev, &indicatedPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null indicatedPortPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamCpuOrPort27ModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamCpuOrPort27ModeGet(dev, &indicatedPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamCpuOrPort27ModeGet(dev, &indicatedPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamDirectModeEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   ledInterfaceNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamDirectModeEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above exclude Lion)
    1.1. Call function with enable [GT_FALSE / GT_TRUE]
         and ledInterfaceNum [0 / 1].
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamDirectModeEnableGet
    Expected: GT_OK and the same enablePtr.
    1.3. Call with wrong ledInterfaceNum [2].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_U32      ledInterfaceNum = 0;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE]
                 and ledInterfaceNum [0 / 1].
            Expected: GT_OK.
        */

        /* call with ledInterfaceNum = 0 */
        ledInterfaceNum = 0;

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChLedStreamDirectModeEnableSet(dev, ledInterfaceNum, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamDirectModeEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamDirectModeEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChLedStreamDirectModeEnableSet(dev, ledInterfaceNum, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamDirectModeEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamDirectModeEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);


        /* call with ledInterfaceNum = 1 */
        ledInterfaceNum = 1;

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChLedStreamDirectModeEnableSet(dev, ledInterfaceNum, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamDirectModeEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamDirectModeEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChLedStreamDirectModeEnableSet(dev, ledInterfaceNum, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamDirectModeEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamDirectModeEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /*
            1.3. Call with wrong ledInterfaceNum [2].
            Expected: GT_BAD_PARAM.
        */
        ledInterfaceNum = 2;

        st = cpssDxChLedStreamDirectModeEnableSet(dev, ledInterfaceNum, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, ledInterfaceNum);

        /*restore value*/
        ledInterfaceNum = 0;
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamDirectModeEnableSet(dev, ledInterfaceNum, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamDirectModeEnableSet(dev, ledInterfaceNum, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamDirectModeEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   ledInterfaceNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamDirectModeEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above exclude Lion)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U32      ledInterfaceNum = 0;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong ledInterfaceNum [2].
            Expected: GT_BAD_PARAM.
        */
        ledInterfaceNum = 2;

        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, ledInterfaceNum);

        /*restore value*/
        ledInterfaceNum = 0;

        /*
            1.3. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamDirectModeEnableGet(dev, ledInterfaceNum, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortGroupClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortGroupClassManipulationSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (dxChx)
    1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            classNum [0 - 6],
                        and classParamsPtr{invertEnable[GT_TRUE],
                                           blinkEnable [GT_TRUE],
                                           blinkSelect [CPSS_LED_BLINK_SELECT_0_E],
                                           forceEnable [GT_TRUE],
                                           forceData   [0].
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamPortGroupClassManipulationGet.
    Expected: GT_OK and the same classParamsPtr values.
    1.3. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            classNum [7 - 11],
                        and classParamsPtr{invertEnable[GT_TRUE],
                                           blinkEnable[GT_TRUE],
                                           blinkSelect[CPSS_LED_BLINK_SELECT_1_E],
                                           forceEnable[GT_TRUE],
                                           forceData[0].
    Expected: GT_OK.
    1.4. Call cpssDxChLedStreamPortGroupClassManipulationGet.
    Expected: GT_OK and the same classParamsPtr values.
    1.5. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.7. Call with classNum [7] and
         portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
    Expected: NOT GT_OK.
    1.8. Call with wrong classNum [12] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.9. Call with wrong classParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.10. Call with wrong enum values classParams.blinkSelect.
    Expected: GT_BAD_PARAM.
    1.11. Call with wrong classNum.forceData [0xFFFF] and the same parameters.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_PORT_GROUPS_BMP              portGroupsBmp   = 1;
    GT_U32                          ledInterfaceNum = 0;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                          classNum        = 0;
    GT_U32                          classNumMax;
    GT_U32                          ledInterfaceMax;
    CPSS_LED_CLASS_MANIPULATION_STC classParams;
    CPSS_LED_CLASS_MANIPULATION_STC classParamsGet;
    GT_U32    portGroupId;

    cpssOsBzero((GT_VOID*) &classParams, sizeof(classParams));
    cpssOsBzero((GT_VOID*) &classParamsGet, sizeof(classParamsGet));

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /*
                1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                        ledInterfaceNum [0 / 1],
                                        portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                        classNum [0 - 6],
                                    and classParamsPtr{invertEnable[GT_TRUE],
                                                       blinkEnable [GT_TRUE],
                                                       blinkSelect [CPSS_LED_BLINK_SELECT_0_E],
                                                       forceEnable [GT_TRUE],
                                                       forceData   [0].
                Expected: GT_OK.
            */
            portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            ledInterfaceNum = 0;
            portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
            classNum        = 0;
            classNumMax = 7;
            ledInterfaceMax = 2;
            classParams.invertEnable = GT_TRUE;
            classParams.blinkEnable  = GT_TRUE;
            classParams.blinkSelect  = CPSS_LED_BLINK_SELECT_0_E;
            classParams.forceEnable  = GT_TRUE;
            classParams.forceData    = 0;

            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* Bobcat2 A0 not supported  */
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
                {
                    continue;
                }
                classNumMax = 6;
                ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
                classParams.pulseStretchEnable = GT_TRUE;
                classParams.disableOnLinkDown = GT_TRUE;
                classParams.invertEnable = GT_FALSE;
            }

            for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
            {
                for(classNum = 0; classNum < classNumMax; classNum++)
                {
                    st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                              ledInterfaceNum, portType, classNum, &classParams);


                    if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                         ledInterfaceNum == 1) &&
                         ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                          (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                    {
                        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                    }
                    else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                    {
                        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                        /*
                            1.2. Call cpssDxChLedStreamPortGroupClassManipulationGet.
                            Expected: GT_OK and the same enablePtr.
                        */
                        st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                                  ledInterfaceNum, portType, classNum, &classParamsGet);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChLedStreamPortGroupClassManipulationGet: %d", dev);

                        /* Verifying struct fields */
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.invertEnable,
                                                     classParamsGet.invertEnable,
                                   "get another classParams->invertEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkEnable,
                                                     classParamsGet.blinkEnable,
                                   "get another classParams->blinkEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkSelect,
                                                     classParamsGet.blinkSelect,
                                   "get another classParams->blinkSelect than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceEnable,
                                                     classParamsGet.forceEnable,
                                   "get another classParams->forceEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceData,
                                                     classParamsGet.forceData,
                                   "get another classParams->forceData than was set: %d", dev);

                        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL1_STRING_MAC(classParams.pulseStretchEnable,
                                                         classParamsGet.pulseStretchEnable,
                                       "get another classParams->pulseStretchEnable than was set: %d", dev);
                            UTF_VERIFY_EQUAL1_STRING_MAC(classParams.disableOnLinkDown,
                                                         classParamsGet.disableOnLinkDown,
                                       "get another classParams->disableOnLinkDown than was set: %d", dev);
                        }

                    }

                }
            }
            /*
                1.3. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                        ledInterfaceNum [0 / 1],
                                        portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                        classNum [7 - 11],
                                    and classParamsPtr{invertEnable[GT_TRUE],
                                                       blinkEnable[GT_TRUE],
                                                       blinkSelect[CPSS_LED_BLINK_SELECT_1_E],
                                                       forceEnable[GT_TRUE],
                                                       forceData[0].
                Expected: GT_OK.
            */
            ledInterfaceNum = 1;
            portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;
            classNum        = 7;
            classNumMax      = 12;
            ledInterfaceMax = 2;
            classParams.invertEnable = GT_TRUE;
            classParams.blinkEnable  = GT_TRUE;
            classParams.blinkSelect  = CPSS_LED_BLINK_SELECT_1_E;
            classParams.forceEnable  = GT_TRUE;
            classParams.forceData    = 0;
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* Bobcat2 A0 not supported  */
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
                {
                    continue;
                }
                classNum = 0;
                classNumMax = 6;

                ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);

                classParams.pulseStretchEnable = GT_FALSE;
                classParams.disableOnLinkDown = GT_FALSE;
                classParams.invertEnable = GT_FALSE;
            }

            for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
            {
                for(; classNum < classNumMax; classNum++)
                {
                    st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                              ledInterfaceNum, portType, classNum, &classParams);

                    if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                         ledInterfaceNum == 1) &&
                         ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                          (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                    {
                        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                    }
                    else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                    {
                        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                        /*
                            1.4. Call cpssDxChLedStreamPortGroupClassManipulationGet.
                            Expected: GT_OK and the same enablePtr.
                        */
                        st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                            ledInterfaceNum, portType, classNum, &classParamsGet);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChLedStreamPortGroupClassManipulationGet: %d", dev);

                        /* Verifying struct fields */
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.invertEnable,
                                                     classParamsGet.invertEnable,
                                   "get another classParams->invertEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkEnable,
                                                     classParamsGet.blinkEnable,
                                   "get another classParams->blinkEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkSelect,
                                                     classParamsGet.blinkSelect,
                                   "get another classParams->blinkSelect than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceEnable,
                                                     classParamsGet.forceEnable,
                                   "get another classParams->forceEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceData,
                                                     classParamsGet.forceData,
                                   "get another classParams->forceData than was set: %d", dev);

                        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL1_STRING_MAC(classParams.pulseStretchEnable,
                                                         classParamsGet.pulseStretchEnable,
                                       "get another classParams->pulseStretchEnable than was set: %d", dev);
                            UTF_VERIFY_EQUAL1_STRING_MAC(classParams.disableOnLinkDown,
                                                         classParamsGet.disableOnLinkDown,
                                       "get another classParams->disableOnLinkDown than was set: %d", dev);
                        }
                    }
                }
            }
            /*
                1.5. Call with wrong ledInterfaceNum [2] and the same parameters.
                Expected: GT_BAD_PARAM.
            */
            ledInterfaceNum = 2;
            portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;
            classNum        = 7;
            classParams.invertEnable = GT_TRUE;
            classParams.blinkEnable  = GT_TRUE;
            classParams.blinkSelect  = CPSS_LED_BLINK_SELECT_1_E;
            classParams.forceEnable  = GT_TRUE;
            classParams.forceData    = 0;
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                classNum = 6;
                ledInterfaceNum = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceNum, dev);
                classParams.invertEnable = GT_FALSE;
            }

            st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                      ledInterfaceNum, portType, classNum, &classParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            ledInterfaceNum = 0;

            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
            {
                /*
                    1.6. Call with wrong enum values portType.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupClassManipulationSet
                                    (dev, portGroupsBmp, ledInterfaceNum,
                                     portType, classNum, &classParams),
                                    portType);
            }

            /*
                1.7. Call with classNum [7] and
                     portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
                Expected: NOT GT_OK.
            */
            classNum = 7;
            portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;

            st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                      ledInterfaceNum, portType, classNum, &classParams);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            classNum = 0;

            /*
                1.8. Call with wrong classNum [12] and the same parameters.
                Expected: GT_BAD_PARAM.
            */
            classNum = 12;

            st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                      ledInterfaceNum, portType, classNum, &classParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            classNum = 6;

            /*
                1.9. Call with wrong classParamsPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                      ledInterfaceNum, portType, classNum, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.10. Call with wrong enum values classParams.blinkSelect.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupClassManipulationSet
                                (dev, portGroupsBmp, ledInterfaceNum,
                                 portType, classNum, &classParams),
                                classParams.blinkSelect);

            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
            {
                /*
                    1.11. Call with wrong classNum.forceData [0xFFFF] and the same parameters.
                    Expected: GT_OUT_OF_RANGE.
                */
                classParams.forceData = 0xFFFF;

                st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                          ledInterfaceNum, portType, classNum, &classParams);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
            }

            classParams.forceData = 0;

            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                      ledInterfaceNum, portType, classNum, &classParams);

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
        st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortGroupClassManipulationSet(dev, portGroupsBmp,
                              ledInterfaceNum, portType, classNum, &classParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortGroupClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortGroupClassManipulationGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (dxChx)
    1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            classNum [0 - 6],
                            and not null classParamsPtr.
    Expected: GT_OK.
    1.2. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            classNum [7 - 11],
                            and not null classParamsPtr.
    Expected: GT_OK.
    1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.5. Call with classNum [7] and
         portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
    Expected: NOT GT_OK.
    1.6. Call with wrong classNum [12] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong classParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_PORT_GROUPS_BMP              portGroupsBmp   = 1;
    GT_U32                          ledInterfaceNum = 0;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                          classNum        = 0;
    CPSS_LED_CLASS_MANIPULATION_STC classParams;
    GT_U32                          classNumMax;
    GT_U32                          ledInterfaceMax;
    GT_U32                          portGroupId;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
        /*
            1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                    ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                    classNum [0 - 6],
                                    and not null classParamsPtr.
            Expected: GT_OK.
        */
        portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        ledInterfaceNum = 0;
        portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
        classNum        = 0;
        classNumMax = 7;
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            classNumMax = 6;
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
        }

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(classNum = 0; classNum < classNumMax; classNum++)
            {
                st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                          ledInterfaceNum, portType, classNum, &classParams);

                if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                     ledInterfaceNum == 1) &&
                     ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
            }
        }
        /*
            1.2. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                    ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                    classNum [7 - 11],
                                    and not null classParamsPtr.
            Expected: GT_OK.
        */
        ledInterfaceNum = 1;
        portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        classNum        = 7;
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            classNum = 0;
            classNumMax = 6;
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
        }
        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
            for(; classNum < classNumMax; classNum++)
            {
                st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                          ledInterfaceNum, portType, classNum, &classParams);

                if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                     ledInterfaceNum == 1) &&
                     ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
            }

        /*
            1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
            Expected: GT_BAD_PARAM.
        */
        ledInterfaceNum = 2;
        portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        classNum        = 7;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            ledInterfaceNum = ledInterfaceMax;

        }

        st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ledInterfaceNum = 0;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {
            /*
                1.4. Call with wrong enum values portType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupClassManipulationGet
                                (dev, portGroupsBmp, ledInterfaceNum,
                                 portType, classNum, &classParams),
                                portType);
        }
        /*
            1.5. Call with classNum [7] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
            Expected: NOT GT_OK.
        */
        classNum = 7;
        portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;

        st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        classNum = 0;

        /*
            1.6. Call with wrong classNum [12] and the same parameters.
            Expected: GT_BAD_PARAM.
        */
        classNum = 12;

        st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        classNum = 0;

        /*
            1.7. Call with wrong classParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, classNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                         ledInterfaceNum, portType, classNum, &classParams);

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
        st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortGroupClassManipulationGet(dev, portGroupsBmp,
                              ledInterfaceNum, portType, classNum, &classParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortGroupConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortGroupConfigSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (dxChx)
    1.1. Call function with
          portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
          ledInterfaceNum [0 / 1],
          ledConfPtr->ledOrganize      [CPSS_LED_ORDER_MODE_BY_PORT_E];
          ledConfPtr->disableOnLinkDown[GT_TRUE];
          ledConfPtr->blink0DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E];
          ledConfPtr->blink0Duration   [CPSS_LED_BLINK_DURATION_0_E];
          ledConfPtr->blink1DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E];
          ledConfPtr->blink1Duration   [CPSS_LED_BLINK_DURATION_0_E];
          ledConfPtr->pulseStretch     [CPSS_LED_PULSE_STRETCH_1_E];
          ledConfPtr->ledStart         [0];
          ledConfPtr->ledEnd           [1 - 255];
          ledConfPtr->clkInvert        [GT_TRUE];
          ledConfPtr->class5select     [CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E];
          ledConfPtr->class13select    [CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E];
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamPortGroupConfigGet
    Expected: GT_OK and the same ledConfPtr values.
    1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong ledConfPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with wrong enum values ledConf.ledOrganize.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values ledConf.blink0DutyCycle.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong enum values ledConf.blink0Duration.
    Expected: GT_BAD_PARAM.
    1.8. Call with wrong enum values ledConf.blink1DutyCycle.
    Expected: GT_BAD_PARAM.
    1.9. Call with wrong enum values ledConf.blink1Duration.
    Expected: GT_BAD_PARAM.
    1.10. Call with wrong enum values ledConf.pulseStretch.
    Expected: GT_BAD_PARAM.
    1.11. Call with wrong ledConf.ledStart [256] and the same parameters.
    Expected: GT_OUT_OF_RANGE.
    1.12. Call with wrong ledConf.ledEnd [256] and the same parameters.
    Expected: GT_OUT_OF_RANGE.
    1.13. Call with wrong enum values ledConf.class13select.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                dev;
    GT_PORT_GROUPS_BMP   portGroupsBmp   = 1;
    GT_U32               ledInterfaceNum = 0;
    GT_U32               ledInterfaceMax;
    CPSS_LED_CONF_STC    ledConf;
    CPSS_LED_CONF_STC    ledConfGet;
    GT_U32    portGroupId;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &ledConf, sizeof(ledConf));
    cpssOsBzero((GT_VOID*) &ledConfGet, sizeof(ledConfGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            ledInterfaceMax = 2;
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* Bobcat2 A0 not supported  */
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
                {
                    continue;
                }
                ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            }

        /*
            1.1. Call function with
                  portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                  ledInterfaceNum [0 - 1],
                  ledConfPtr->ledOrganize      [CPSS_LED_ORDER_MODE_BY_PORT_E];
                  ledConfPtr->disableOnLinkDown[GT_TRUE];
                  ledConfPtr->blink0DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E];
                  ledConfPtr->blink0Duration   [CPSS_LED_BLINK_DURATION_0_E];
                  ledConfPtr->blink1DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E];
                  ledConfPtr->blink1Duration   [CPSS_LED_BLINK_DURATION_0_E];
                  ledConfPtr->pulseStretch     [CPSS_LED_PULSE_STRETCH_1_E];
                  ledConfPtr->ledStart         [0];
                  ledConfPtr->ledEnd           [1 - 255];
                  ledConfPtr->clkInvert        [GT_TRUE];
                  ledConfPtr->class5select     [CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E];
                  ledConfPtr->class13select    [CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E];
            Expected: GT_OK.
        */
        portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
            for(ledConf.ledEnd = 0; ledConf.ledEnd < 256; ledConf.ledEnd++)
            {
                st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp,
                                                        ledInterfaceNum, &ledConf);
                if (ledInterfaceNum == 1 &&
                    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.2. Call cpssDxChLedStreamPortGroupConfigGet
                        Expected: GT_OK and the same enablePtr.
                    */
                    st = cpssDxChLedStreamPortGroupConfigGet(dev, portGroupsBmp,
                                                            ledInterfaceNum, &ledConfGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamPortGroupConfigGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledOrganize,
                                                 ledConfGet.ledOrganize,
                               "get another ledConf->ledOrganize than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.disableOnLinkDown,
                                                 ledConfGet.disableOnLinkDown,
                               "get another ledConf->disableOnLinkDown than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.blink0DutyCycle,
                                                 ledConfGet.blink0DutyCycle,
                               "get another ledConf->blink0DutyCycle than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.blink0Duration,
                                                 ledConfGet.blink0Duration,
                               "get another ledConf->blink0Duration than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.pulseStretch,
                                                 ledConfGet.pulseStretch,
                               "get another ledConf->pulseStretch than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledStart,
                                                 ledConfGet.ledStart,
                               "get another ledConf->ledStart than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledEnd,
                                                 ledConfGet.ledEnd,
                               "get another ledConf->ledEnd than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.clkInvert,
                                                 ledConfGet.clkInvert,
                               "get another ledConf->clkInvert than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.class5select,
                                                 ledConfGet.class5select,
                               "get another ledConf->class5select than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.class13select,
                                                 ledConfGet.class13select,
                               "get another ledConf->class13select than was set: %d", dev);
                }

                if(GT_TRUE == prvUtfReduceLogSizeFlagGet())
                {
                    /* reduce number of tests for log */
                    break;
                }
            }

            ledConf.ledEnd = 5;

            /*
                1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
                Expected: GT_BAD_PARAM.
            */
            ledInterfaceNum = 2;
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                ledInterfaceNum = ledInterfaceMax;
            }

            st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp,
                                                    ledInterfaceNum, &ledConf);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            ledInterfaceNum = 0;
            if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
            {
                ledInterfaceNum = ledInterfaceMax - 1;
            }

            /*
                1.4. Call with wrong ledConfPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp, ledInterfaceNum, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.5. Call with wrong enum values ledConf.ledOrganize.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                ledConf.ledOrganize);

            /*
                1.6. Call with wrong enum values ledConf.blink0DutyCycle.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                ledConf.blink0DutyCycle);

            /*
                1.7. Call with wrong enum values ledConf.blink0Duration.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                ledConf.blink0Duration);

            /*
                1.8. Call with wrong enum values ledConf.blink1DutyCycle.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                ledConf.blink1DutyCycle);

            /*
                1.9. Call with wrong enum values ledConf.blink1Duration.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                ledConf.blink1Duration);

            /*
                1.10. Call with wrong enum values ledConf.pulseStretch.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                ledConf.pulseStretch);

            /*
                1.11. Call with wrong ledConf.ledStart [256] and the same parameters.
                Expected: GT_OUT_OF_RANGE.
            */
            ledConf.ledStart = 256;

            st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp,
                                                    ledInterfaceNum, &ledConf);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            ledConf.ledStart = 0;

            /*
                1.12. Call with wrong ledConf.ledEnd [256] and the same parameters.
                Expected: GT_OUT_OF_RANGE.
            */
            ledConf.ledEnd = 256;

            st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp,
                                                    ledInterfaceNum, &ledConf);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            ledConf.ledEnd = 1;
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
            {
                /*
                    1.12. Call with wrong enum values ledConf.class5select.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                    (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                    ledConf.class5select);

                /*
                    1.13. Call with wrong enum values ledConf.class13select.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupConfigSet
                                    (dev, portGroupsBmp, ledInterfaceNum, &ledConf),
                                    ledConf.class13select);
            }

            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp,
                                              ledInterfaceNum, &ledConf);

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
        st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp,
                                                 ledInterfaceNum, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortGroupConfigSet(dev, portGroupsBmp,
                                            ledInterfaceNum, &ledConf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortGroupConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortGroupConfigGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (dxChx)
    1.1. Call function with ledInterfaceNum [0 - 1], and not null ledConfPtr.
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamPortGroupConfigGet
    Expected: GT_OK and the same ledConfPtr values.
    1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong ledConfPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                dev;
    GT_PORT_GROUPS_BMP   portGroupsBmp   = 1;
    GT_U32               ledInterfaceNum = 0;
    GT_U32               ledInterfaceMax;
    CPSS_LED_CONF_STC    ledConf;
    GT_U32    portGroupId;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
        }

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /*
                1.1. Call function with ledInterfaceNum [0 - 1], and not null ledConfPtr.
                Expected: GT_OK.
            */
        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            st = cpssDxChLedStreamPortGroupConfigGet(dev, portGroupsBmp,
                                                    ledInterfaceNum, &ledConf);
                if (ledInterfaceNum == 1 &&
                    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
            {
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
            {
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        /*
            1.3. Call with wrong ledInterfaceNum [6] and the same parameters.
            Expected: GT_BAD_PARAM.
        */
        ledInterfaceNum = ledInterfaceMax + 1;

        st = cpssDxChLedStreamPortGroupConfigGet(dev, portGroupsBmp,
                                                ledInterfaceNum, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        /*
            1.4. Call with wrong ledConfPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamPortGroupConfigGet(dev, portGroupsBmp, ledInterfaceNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChLedStreamPortGroupConfigGet(dev, portGroupsBmp,
                                                  ledInterfaceNum, &ledConf);

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
        st = cpssDxChLedStreamPortGroupConfigGet(dev, portGroupsBmp,
                                                 ledInterfaceNum, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortGroupConfigGet(dev, portGroupsBmp,
                                            ledInterfaceNum, &ledConf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortGroupGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortGroupGroupConfigSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (dxChx)
    1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            groupNum [0 - 3],
                        and groupParamsPtr{classA[0],
                                           classB[1],
                                           classC[2],
                                           classD[3]}.
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamPortGroupGroupConfigGet
    Expected: GT_OK and the same groupParamsPtr values.
    1.3. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            groupNum [0 - 1],
                        and groupParamsPtr{classA[0],
                                           classB[1],
                                           classC[2],
                                           classD[3]}.
    Expected: GT_OK.
    1.4. Call cpssDxChLedStreamPortGroupGroupConfigGet
    Expected: GT_OK and the same groupParamsPtr values.
    1.5. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.7. Call with groupNum [2] and
         portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
    Expected: NOT GT_OK.
    1.8. Call with wrong groupNum [4] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.9. Call with wrong groupParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.10. Call with wrong groupParamsPtr->classA [16].
    Expected: GT_OUT_OF_RANGE.
    1.11. Call with wrong groupParamsPtr->classB [16].
    Expected: GT_OUT_OF_RANGE.
    1.12. Call with wrong groupParamsPtr->classC [16].
    Expected: GT_OUT_OF_RANGE.
    1.13. Call with wrong groupParamsPtr->classD [16].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_PORT_GROUPS_BMP              portGroupsBmp   = 1;
    GT_U32                          ledInterfaceNum = 0;
    GT_U32                          ledInterfaceMax;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                          groupNum        = 0;
    GT_U32                          groupNumMax;
    CPSS_LED_GROUP_CONF_STC         groupParams;
    CPSS_LED_GROUP_CONF_STC         groupParamsGet;
    GT_U32    portGroupId;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &groupParams, sizeof(groupParams));
    cpssOsBzero((GT_VOID*) &groupParamsGet, sizeof(groupParamsGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;
        ledInterfaceMax = 2;
        groupNumMax = 3;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            groupNumMax = 2;
        }

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
        /*
            1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                    ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                    groupNum [0 - 3],
                                and groupParamsPtr{classA[0],
                                                   classB[1],
                                                   classC[2],
                                                   classD[3]}.
            Expected: GT_OK.
        */
        portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        ledInterfaceNum = 0;
        portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
        groupNum        = 0;
        groupParams.classA = 0;
        groupParams.classB = 1;
        groupParams.classC = 2;
        groupParams.classD = 3;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < groupNumMax; groupNum++)
            {
                st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                          ledInterfaceNum, portType, groupNum, &groupParams);

                if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                     ledInterfaceNum == 1) &&
                     ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.2. Call cpssDxChLedStreamPortGroupGroupConfigGet.
                        Expected: GT_OK and the same groupParamsPtr.
                    */
                    st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                              ledInterfaceNum, portType, groupNum, &groupParamsGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamPortGroupGroupConfigGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classA,
                                                 groupParamsGet.classA,
                               "get another groupParams->classA than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classB,
                                                 groupParamsGet.classB,
                               "get another groupParams->classB than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classC,
                                                 groupParamsGet.classC,
                               "get another groupParams->classC than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classD,
                                                 groupParamsGet.classD,
                               "get another groupParams->classD than was set: %d", dev);
                }
            }
        }
        /*
            1.3. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                    ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                    groupNum [0 - 1],
                                and groupParamsPtr{classA[0],
                                                   classB[1],
                                                   classC[2],
                                                   classD[3]}.
            Expected: GT_OK.
        */
        ledInterfaceNum = 1;
        portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < 2; groupNum++)
            {
                st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                          ledInterfaceNum, portType, groupNum, &groupParams);

                if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                     ledInterfaceNum == 1) &&
                      ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.4. Call cpssDxChLedStreamPortGroupGroupConfigGet.
                        Expected: GT_OK and the same enablePtr.
                    */
                    st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                        ledInterfaceNum, portType, groupNum, &groupParamsGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamPortGroupGroupConfigGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classA,
                                                 groupParamsGet.classA,
                               "get another groupParams->classA than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classB,
                                                 groupParamsGet.classB,
                               "get another groupParams->classB than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classC,
                                                 groupParamsGet.classC,
                               "get another groupParams->classC than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classD,
                                                 groupParamsGet.classD,
                               "get another groupParams->classD than was set: %d", dev);
                }
            }
        }
        /*
            1.5. Call with wrong ledInterfaceNum [6] and the same parameters.
            Expected: GT_BAD_PARAM.
        */
        groupNum        = 0;
        ledInterfaceNum = ledInterfaceMax;

        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {

            /*
                1.6. Call with wrong enum values portType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupGroupConfigSet
                                (dev, portGroupsBmp, ledInterfaceNum,
                                 portType, groupNum, &groupParams),
                                portType);
        }

        /*
            1.7. Call with groupNum [2] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
            Expected: NOT GT_OK.
        */
        groupNum = 2;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        groupNum = 0;

        /*
            1.8. Call with wrong groupNum [4] and the same parameters.
            Expected: GT_BAD_PARAM.
        */
        groupNum = 4;

        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        groupNum = 0;

        /*
            1.9. Call with wrong groupParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.10. Call with wrong groupParamsPtr->classA [16].
            Expected: GT_OUT_OF_RANGE.
        */
        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        groupParams.classA = 16;

        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classA = 0;

        /*
            1.11. Call with wrong groupParamsPtr->classB [16].
            Expected: GT_OUT_OF_RANGE.
        */
        groupParams.classB = 16;

        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classB = 0;

        /*
            1.12. Call with wrong groupParamsPtr->classC [16].
            Expected: GT_OUT_OF_RANGE.
        */
        groupParams.classC = 16;

        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classC = 0;

        /*
            1.13. Call with wrong groupParamsPtr->classD [16].
            Expected: GT_OUT_OF_RANGE.
        */
        groupParams.classD = 16;

        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classD = 0;
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                       ledInterfaceNum, portType, groupNum, &groupParams);

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
        st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortGroupGroupConfigSet(dev, portGroupsBmp,
                              ledInterfaceNum, portType, groupNum, &groupParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortGroupGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortGroupGroupConfigGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (dxChx)
    1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            groupNum [0 - 3],
                        and not null groupParamsPtr.
    Expected: GT_OK.
    1.2. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                            ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            groupNum [0 - 1],
                        and not null groupParamsPtr.
    Expected: GT_OK.
    1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.5. Call with groupNum [2] and
         portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
    Expected: NOT GT_OK.
    1.6. Call with wrong groupNum [4] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong groupParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_PORT_GROUPS_BMP              portGroupsBmp   = 1;
    GT_U32                          ledInterfaceNum = 0;
    GT_U32                          ledInterfaceMax;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                          groupNum        = 0;
    GT_U32                          groupNumMax;
    CPSS_LED_GROUP_CONF_STC         groupParams;
    GT_U32    portGroupId;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;
        ledInterfaceMax = 2;
        groupNumMax = 3;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            groupNumMax = 2;
        }

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
        /*
            1.1. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                    ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                    groupNum [0 - 3],
                                    and not null groupParamsPtr.
            Expected: GT_OK.
        */
        portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        ledInterfaceNum = 0;
        portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
        groupNum        = 0;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < groupNumMax; groupNum++)
            {
                st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                          ledInterfaceNum, portType, groupNum, &groupParams);

                if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                     ledInterfaceNum == 1) &&
                      ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }

            }
        }
        /*
            1.2. Call function with portGroupsBmp [CPSS_PORT_GROUP_UNAWARE_MODE_CNS],
                                    ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                    groupNum [0 - 1],
                                    and not null groupParamsPtr.
            Expected: GT_OK.
        */
        portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < 2; groupNum++)
            {
                st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                          ledInterfaceNum, portType, groupNum, &groupParams);

                if ((portType == CPSS_DXCH_LED_PORT_TYPE_XG_E ||
                     ledInterfaceNum == 1) &&
                      ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
            }
        }
        /*
            1.3. Call with wrong ledInterfaceNum [6] and the same parameters.
            Expected: GT_BAD_PARAM.
        */
        groupNum        = 0;
        ledInterfaceNum = ledInterfaceMax;

        st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {

            /*
                1.4. Call with wrong enum values portType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamPortGroupGroupConfigGet
                                (dev, portGroupsBmp, ledInterfaceNum,
                                 portType, groupNum, &groupParams),
                                portType);
        }

        /*
            1.5. Call with groupNum [2] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
            Expected: NOT GT_OK.
        */
        groupNum = 2;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        groupNum = 0;

        /*
            1.6. Call with wrong groupNum [4] and the same parameters.
            Expected: GT_BAD_PARAM.
        */
        groupNum = 4;

        st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        groupNum = 0;

        /*
            1.7. Call with wrong groupParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                              ledInterfaceNum, portType, groupNum, &groupParams);

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
        st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                                  ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortGroupGroupConfigGet(dev, portGroupsBmp,
                              ledInterfaceNum, portType, groupNum, &groupParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamClassManipulationGet
(
    IN  GT_U8                            dev,
    IN  GT_U32                           ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT      portType,
    IN  GT_U32                           classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC  *classParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamClassManipulationGet)
{
/*
    ITERATE_DEVICES(dxChx)
    1.1. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            classNum [0 - 6],
                            and not null classParamsPtr.
    Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
    1.2. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            classNum [7 - 11],
                            and not null classParamsPtr.
    Expected: NOT GT_OK for Lion and GT_OK for other cases.
    1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: NOT GT_OK.
    1.4. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.5. Call with classNum [7] and
         portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
    Expected: NOT GT_OK.
    1.6. Call with wrong classNum [12] and
         portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect classNum).
    Expected: NOT GT_OK.
    1.7. Call with wrong classParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_U32                          ledInterfaceNum = 0;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                          classNum        = 0;
    GT_U32                          classNumMax;
    GT_U32                          ledInterfaceMax;
    CPSS_LED_CLASS_MANIPULATION_STC classParams;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &classParams, sizeof(classParams));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        classNum = 0;
        classNumMax = 7;
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            classNumMax = 6;
            classNum = 0;
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
        }
        /*
            1.1. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                    classNum [0 - 6],
                                    and not null classParamsPtr.
            Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
        */
        portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(; classNum < classNumMax; classNum++)
            {
                st = cpssDxChLedStreamClassManipulationGet(dev,
                             ledInterfaceNum, portType, classNum, &classParams);

                if ( (ledInterfaceNum == 1) &&
                     ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
            }
        }
        /*
            1.2. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                    classNum [7 - 11],
                                    and not null classParamsPtr.
            Expected: NOT GT_OK for Lion and GT_OK for other cases.
        */
        classNum = 7;
        classNumMax = 12;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            classNum = 0;
            classNumMax = 6;
        }

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(; classNum < classNumMax; classNum++)
            {
                st = cpssDxChLedStreamClassManipulationGet(dev,
                             ledInterfaceNum, portType, classNum, &classParams);

                if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
            }
        }
        /*
            1.3. Call with wrong ledInterfaceNum [6] and the same parameters.
            Expected: NOT GT_OK.
        */
        ledInterfaceNum = ledInterfaceMax;
        portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        classNum        = 7;

        st = cpssDxChLedStreamClassManipulationGet(dev,
                     ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ledInterfaceNum = 0;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {
            /*
                1.4. Call with wrong enum values portType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamClassManipulationGet
                                (dev, ledInterfaceNum,
                                 portType, classNum, &classParams),
                                portType);

        }
        /*
            1.5. Call with classNum [7] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
            Expected: NOT GT_OK.
        */
        classNum = 7;
        portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;

        st = cpssDxChLedStreamClassManipulationGet(dev,
                     ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        classNum = 0;

        /*
            1.6. Call with wrong classNum [12] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect classNum).
            Expected: NOT GT_OK.
        */
        classNum = 12;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        st = cpssDxChLedStreamClassManipulationGet(dev,
                     ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        classNum = 0;

        /*
            1.7. Call with wrong classParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamClassManipulationGet(dev,
                     ledInterfaceNum, portType, classNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* restore correct values */
    ledInterfaceNum = 0;
    portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    classNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamClassManipulationGet(dev, ledInterfaceNum,
                            portType, classNum, &classParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamClassManipulationGet(dev, ledInterfaceNum,
                            portType, classNum, &classParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamClassManipulationSet
(
    IN  GT_U8                            dev,
    IN  GT_U32                           ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT      portType,
    IN  GT_U32                           classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC  *classParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamClassManipulationSet)
{
/*
    ITERATE_DEVICES(All DxCh Devices)
    1.1. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            classNum [0 - 6],
                        and classParamsPtr{invertEnable[GT_TRUE],
                                           blinkEnable [GT_TRUE],
                                           blinkSelect [CPSS_LED_BLINK_SELECT_0_E],
                                           forceEnable [GT_TRUE],
                                           forceData   [0].
    Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
    1.2. Call cpssDxChLedStreamClassManipulationGet.
    Expected: GT_OK and the same enablePtr.
    1.3. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            classNum [7 - 11],
                        and classParamsPtr{invertEnable[GT_FALSE],
                                           blinkEnable[GT_FALSE],
                                           blinkSelect[CPSS_LED_BLINK_SELECT_1_E],
                                           forceEnable[GT_FALSE],
                                           forceData[1].
    Expected: NOT GT_OK for Lion and GT_OK for other cases.
    1.4. Call cpssDxChLedStreamClassManipulationGet.
    Expected: GT_OK and the same enablePtr.
    1.5. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: NOT GT_OK.
    1.6. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.7. Call with classNum [7] and
         portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
    Expected: NOT GT_OK.
    1.8. Call with wrong classNum [12] and
         portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect classNum).
    Expected: NOT GT_OK.
    1.9. Call with wrong classParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.10. Call with wrong enum values classParams.blinkSelect.
    Expected: GT_BAD_PARAM.
    1.11. Call with wrong classNum.forceData [0xFFF+1],
          portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E]
          and the same parameters.
    Expected: GT_OUT_OF_RANGE.
    1.12. Call with wrong classNum.forceData [4]
          portType [CPSS_DXCH_LED_PORT_TYPE_XG_E]
          and the same parameters.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS st = GT_OK;

    GT_U8                            dev;
    GT_U32                           ledInterfaceNum = 0;
    CPSS_DXCH_LED_PORT_TYPE_ENT      portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                           classNum = 0;
    GT_U32                           classNumMax;
    GT_U32                           ledInterfaceMax;
    CPSS_LED_CLASS_MANIPULATION_STC  classParams;
    CPSS_LED_CLASS_MANIPULATION_STC  classParamsGet;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Zero structures first */
    cpssOsBzero((GT_VOID*) &classParams, sizeof(classParams));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                    classNum [0 - 6],
                                and classParamsPtr{invertEnable[GT_TRUE],
                                                   blinkEnable [GT_TRUE],
                                                   blinkSelect [CPSS_LED_BLINK_SELECT_0_E],
                                                   forceEnable [GT_TRUE],
                                                   forceData   [0].
            Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
        */
        portType  = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
        classParams.invertEnable = GT_TRUE;
        classParams.blinkEnable  = GT_TRUE;
        classParams.blinkSelect  = CPSS_LED_BLINK_SELECT_0_E;
        classParams.forceEnable  = GT_TRUE;
        classParams.forceData    = 0;
        classNumMax = 7;
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            classNumMax = 6;
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            classParams.invertEnable = GT_FALSE;
        }

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(; classNum < classNumMax; classNum++)
            {
                st = cpssDxChLedStreamClassManipulationSet(dev,
                               ledInterfaceNum, portType, classNum, &classParams);

                if ((ledInterfaceNum == 1) &&
                    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.2. Call cpssDxChLedStreamClassManipulationGet.
                        Expected: GT_OK and the same enablePtr.
                    */
                    st = cpssDxChLedStreamClassManipulationGet(dev,
                               ledInterfaceNum, portType, classNum, &classParamsGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamClassManipulationGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.invertEnable,
                                                 classParamsGet.invertEnable,
                               "get another classParams->invertEnable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkEnable,
                                                 classParamsGet.blinkEnable,
                               "get another classParams->blinkEnable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkSelect,
                                                 classParamsGet.blinkSelect,
                               "get another classParams->blinkSelect than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceEnable,
                                                 classParamsGet.forceEnable,
                               "get another classParams->forceEnable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceData,
                                                 classParamsGet.forceData,
                               "get another classParams->forceData than was set: %d", dev);
                }
            }
        }
        /*
            1.3. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                    classNum [7 - 11],
                                and classParamsPtr{invertEnable[GT_FALSE],
                                                   blinkEnable[GT_FALSE],
                                                   blinkSelect[CPSS_LED_BLINK_SELECT_1_E],
                                                   forceEnable[GT_FALSE],
                                                   forceData[1].
            Expected: NOT GT_OK for Lion and GT_OK for other cases.
        */
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        classParams.invertEnable = GT_FALSE;
        classParams.blinkEnable  = GT_FALSE;
        classParams.blinkSelect  = CPSS_LED_BLINK_SELECT_1_E;
        classParams.forceEnable  = GT_FALSE;
        classParams.forceData    = 1;
        classNum                 = 7;
        classNumMax = 12;
        ledInterfaceMax = 2;

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            classNum = 0;
            classNumMax = 6;
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
        }

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(; classNum < classNumMax; classNum++)
            {
                st = cpssDxChLedStreamClassManipulationSet(dev,
                               ledInterfaceNum, portType, classNum, &classParams);

                if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.4. Call cpssDxChLedStreamClassManipulationGet.
                        Expected: GT_OK and the same enablePtr.
                    */
                    st = cpssDxChLedStreamClassManipulationGet(dev,
                               ledInterfaceNum, portType, classNum, &classParamsGet);

                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamClassManipulationGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.invertEnable,
                                                 classParamsGet.invertEnable,
                               "get another classParams->invertEnable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkEnable,
                                                 classParamsGet.blinkEnable,
                               "get another classParams->blinkEnable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.blinkSelect,
                                                 classParamsGet.blinkSelect,
                               "get another classParams->blinkSelect than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceEnable,
                                                 classParamsGet.forceEnable,
                               "get another classParams->forceEnable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(classParams.forceData,
                                                 classParamsGet.forceData,
                               "get another classParams->forceData than was set: %d", dev);
                }
            }
        }
        /*
            1.5. Call with wrong ledInterfaceNum [2] and the same parameters.
            Expected: NOT GT_OK.
        */
        ledInterfaceNum = ledInterfaceMax;
        portType        = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        classNum        = 7;
        classParams.invertEnable = GT_TRUE;
        classParams.blinkEnable  = GT_TRUE;
        classParams.blinkSelect  = CPSS_LED_BLINK_SELECT_1_E;
        classParams.forceEnable  = GT_TRUE;
        classParams.forceData    = 0;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            ledInterfaceNum = 5;
            classParams.invertEnable = GT_FALSE;
        }

        st = cpssDxChLedStreamClassManipulationSet(dev,
                      ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ledInterfaceNum = 0;

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {
            /*
                1.6. Call with wrong enum values portType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamClassManipulationSet
                                (dev, ledInterfaceNum,
                                 portType, classNum, &classParams),
                                portType);
        }

        /*
            1.7. Call with classNum [7] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E] (incorrect classNum).
            Expected: NOT GT_OK.
        */
        classNum = 7;
        portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;

        st = cpssDxChLedStreamClassManipulationSet(dev,
                      ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        classNum = 0;

        /*
            1.8. Call with wrong classNum [12] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect classNum).
            Expected: NOT GT_OK.
        */
        classNum = 12;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        st = cpssDxChLedStreamClassManipulationSet(dev,
                      ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        classNum = 6;

        /*
            1.9. Call with wrong classParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamClassManipulationSet(dev,
                      ledInterfaceNum, portType, classNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.10. Call with wrong enum values classParams.blinkSelect.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamClassManipulationSet
                            (dev, ledInterfaceNum,
                             portType, classNum, &classParams),
                            classParams.blinkSelect);

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {
            /*
                1.11. Call with wrong classNum.forceData [0xFFF+1],
                      portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E]
                      and the same parameters.
                Expected: GT_OUT_OF_RANGE.
            */
            portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
            classParams.forceData = 0xFFF+1;

            st = cpssDxChLedStreamClassManipulationSet(dev,
                       ledInterfaceNum, portType, classNum, &classParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        }

        classParams.forceData = 0;

        /*
            1.12. Call with wrong classNum.forceData [2]
                  portType [CPSS_DXCH_LED_PORT_TYPE_XG_E]
                  and the same parameters.
            Expected: NOT GT_OK.
        */
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        classParams.forceData = 4;

        st = cpssDxChLedStreamClassManipulationSet(dev,
                   ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        classParams.forceData = 0;
    }

    /* restore correct values */
    ledInterfaceNum = 0;
    portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    classNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamClassManipulationSet(dev,
               ledInterfaceNum, portType, classNum, &classParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamClassManipulationSet(dev,
           ledInterfaceNum, portType, classNum, &classParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamConfigSet)
{
/*
    ITERATE_DEVICES(dxChx)
    1.1. Call function with
          ledInterfaceNum [0 / 1],
          ledConfPtr->ledOrganize      [CPSS_LED_ORDER_MODE_BY_PORT_E /
                                        CPSS_LED_ORDER_MODE_BY_CLASS_E];
          ledConfPtr->disableOnLinkDown[GT_TRUE / GT_FALSE];
          ledConfPtr->blink0DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E /
                                        CPSS_LED_BLINK_DUTY_CYCLE_1_E];
          ledConfPtr->blink0Duration   [CPSS_LED_BLINK_DURATION_0_E /
                                        CPSS_LED_BLINK_DURATION_1_E];
          ledConfPtr->blink1DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E /
                                        CPSS_LED_BLINK_DUTY_CYCLE_1_E];
          ledConfPtr->blink1Duration   [CPSS_LED_BLINK_DURATION_0_E /
                                        CPSS_LED_BLINK_DURATION_1_E];
          ledConfPtr->pulseStretch     [CPSS_LED_PULSE_STRETCH_0_NO_E /
                                        CPSS_LED_PULSE_STRETCH_1_E];
          ledConfPtr->ledStart         [0 / 1];
          ledConfPtr->ledEnd           [0 - 255];
          ledConfPtr->clkInvert        [GT_TRUE / GT_FALSE];
          ledConfPtr->class5select     [CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E /
                                        CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E];
          ledConfPtr->class13select    [CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E /
                                        CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E];
    Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
    1.2. Call cpssDxChLedStreamConfigGet
    Expected: GT_OK and the same enablePtr.
    1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: NOT GT_OK.
    1.4. Call with wrong ledConfPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with wrong enum values ledConf.ledOrganize.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values ledConf.blink0DutyCycle.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong enum values ledConf.blink0Duration.
    Expected: GT_BAD_PARAM.
    1.8. Call with wrong enum values ledConf.blink1DutyCycle.
    Expected: GT_BAD_PARAM.
    1.9. Call with wrong enum values ledConf.blink1Duration.
    Expected: GT_BAD_PARAM.
    1.10. Call with wrong enum values ledConf.pulseStretch.
    Expected: GT_BAD_PARAM.
    1.11. Call with wrong ledConf.ledStart [256] and the same parameters.
    Expected: GT_OUT_OF_RANGE.
    1.12. Call with wrong ledConf.ledEnd [256] and the same parameters.
    Expected: GT_OUT_OF_RANGE.
    1.12. Call with wrong enum values ledConf.class5select.
    Expected: GT_BAD_PARAM.
    1.13. Call with wrong enum values ledConf.class13select.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                dev;
    GT_U32               ledInterfaceNum = 0;
    GT_U32               ledInterfaceMax;
    CPSS_LED_CONF_STC    ledConf;
    CPSS_LED_CONF_STC    ledConfGet;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &ledConf, sizeof(ledConf));
    cpssOsBzero((GT_VOID*) &ledConfGet, sizeof(ledConfGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                  ledInterfaceNum [0 / 1],
                  ledConfPtr->ledOrganize      [CPSS_LED_ORDER_MODE_BY_PORT_E /
                                                CPSS_LED_ORDER_MODE_BY_CLASS_E];
                  ledConfPtr->disableOnLinkDown[GT_TRUE / GT_FALSE];
                  ledConfPtr->blink0DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E /
                                                CPSS_LED_BLINK_DUTY_CYCLE_1_E];
                  ledConfPtr->blink0Duration   [CPSS_LED_BLINK_DURATION_0_E /
                                                CPSS_LED_BLINK_DURATION_1_E];
                  ledConfPtr->blink1DutyCycle  [CPSS_LED_BLINK_DUTY_CYCLE_0_E /
                                                CPSS_LED_BLINK_DUTY_CYCLE_1_E];
                  ledConfPtr->blink1Duration   [CPSS_LED_BLINK_DURATION_0_E /
                                                CPSS_LED_BLINK_DURATION_1_E];
                  ledConfPtr->pulseStretch     [CPSS_LED_PULSE_STRETCH_0_NO_E /
                                                CPSS_LED_PULSE_STRETCH_1_E];
                  ledConfPtr->ledStart         [0 / 1];
                  ledConfPtr->ledEnd           [0 - 255];
                  ledConfPtr->clkInvert        [GT_TRUE / GT_FALSE];
                  ledConfPtr->class5select     [CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E /
                                                CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E];
                  ledConfPtr->class13select    [CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E /
                                                CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E];
            Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
        */

        /* call with first params group */
        ledConf.ledOrganize       = CPSS_LED_ORDER_MODE_BY_PORT_E;
        ledConf.disableOnLinkDown = GT_TRUE;
        ledConf.blink0DutyCycle   = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
        ledConf.blink0Duration    = CPSS_LED_BLINK_DURATION_0_E;
        ledConf.blink1DutyCycle   = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
        ledConf.blink1Duration    = CPSS_LED_BLINK_DURATION_0_E;
        ledConf.pulseStretch      = CPSS_LED_PULSE_STRETCH_0_NO_E;
        ledConf.ledStart          = 0;
        ledConf.clkInvert         = GT_TRUE;
        ledConf.class5select      = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
        ledConf.class13select     = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);

            ledConf.disableOnLinkDown = GT_FALSE;
            ledConf.clkInvert         = GT_FALSE;
        }

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(ledConf.ledEnd = 0; ledConf.ledEnd < 256; ledConf.ledEnd++)
            {
                st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, &ledConf);

                if (ledInterfaceNum == 1 &&
                    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.2. Call cpssDxChLedStreamConfigGet
                        Expected: GT_OK and the same enablePtr.
                    */
                    st = cpssDxChLedStreamConfigGet(dev, ledInterfaceNum, &ledConfGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamConfigGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledOrganize,
                                                 ledConfGet.ledOrganize,
                               "get another ledConf->ledOrganize than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.disableOnLinkDown,
                                                 ledConfGet.disableOnLinkDown,
                               "get another ledConf->disableOnLinkDown than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.blink0DutyCycle,
                                                 ledConfGet.blink0DutyCycle,
                               "get another ledConf->blink0DutyCycle than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.blink0Duration,
                                                 ledConfGet.blink0Duration,
                               "get another ledConf->blink0Duration than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.pulseStretch,
                                                 ledConfGet.pulseStretch,
                               "get another ledConf->pulseStretch than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledStart,
                                                 ledConfGet.ledStart,
                               "get another ledConf->ledStart than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledEnd,
                                                 ledConfGet.ledEnd,
                               "get another ledConf->ledEnd than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.clkInvert,
                                                 ledConfGet.clkInvert,
                               "get another ledConf->clkInvert than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.class5select,
                                                 ledConfGet.class5select,
                               "get another ledConf->class5select than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.class13select,
                                                 ledConfGet.class13select,
                               "get another ledConf->class13select than was set: %d", dev);
                }
            }
        }
        /* call with second params group */
        ledConf.ledOrganize       = CPSS_LED_ORDER_MODE_BY_CLASS_E;
        ledConf.disableOnLinkDown = GT_FALSE;
        ledConf.blink0DutyCycle   = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
        ledConf.blink0Duration    = CPSS_LED_BLINK_DURATION_1_E;
        ledConf.blink1DutyCycle   = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
        ledConf.blink1Duration    = CPSS_LED_BLINK_DURATION_1_E;
        ledConf.pulseStretch      = CPSS_LED_PULSE_STRETCH_1_E;
        ledConf.ledStart          = 1;
        ledConf.clkInvert         = GT_FALSE;
        ledConf.class5select      = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;
        ledConf.class13select     = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E;
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }

            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            ledConf.class5select      = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
            ledConf.class13select     = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
        }

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(ledConf.ledEnd = 1; ledConf.ledEnd < 256; ledConf.ledEnd++)
            {
                st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, &ledConf);
                if ((ledInterfaceNum == 1) &&
                    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.2. Call cpssDxChLedStreamConfigGet
                        Expected: GT_OK and the same enablePtr.
                    */
                    st = cpssDxChLedStreamConfigGet(dev, ledInterfaceNum, &ledConfGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamConfigGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledOrganize,
                                                 ledConfGet.ledOrganize,
                               "get another ledConf->ledOrganize than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.disableOnLinkDown,
                                                 ledConfGet.disableOnLinkDown,
                               "get another ledConf->disableOnLinkDown than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.blink0DutyCycle,
                                                 ledConfGet.blink0DutyCycle,
                               "get another ledConf->blink0DutyCycle than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.blink0Duration,
                                                 ledConfGet.blink0Duration,
                               "get another ledConf->blink0Duration than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.pulseStretch,
                                                 ledConfGet.pulseStretch,
                               "get another ledConf->pulseStretch than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledStart,
                                                 ledConfGet.ledStart,
                               "get another ledConf->ledStart than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.ledEnd,
                                                 ledConfGet.ledEnd,
                               "get another ledConf->ledEnd than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.clkInvert,
                                                 ledConfGet.clkInvert,
                               "get another ledConf->clkInvert than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.class5select,
                                                 ledConfGet.class5select,
                               "get another ledConf->class5select than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(ledConf.class13select,
                                                 ledConfGet.class13select,
                               "get another ledConf->class13select than was set: %d", dev);
                }
                if(GT_TRUE == prvUtfReduceLogSizeFlagGet())
                {
                    /* reduce number of tests for log */
                    break;
                }
            }
        }
        ledConf.ledEnd = 5;

        /*
            1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
            Expected: NOT GT_OK.
        */
        ledInterfaceNum = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            ledInterfaceNum = ledInterfaceMax;
        }

        st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, &ledConf);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        /*
            1.4. Call with wrong ledConfPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with wrong enum values ledConf.ledOrganize.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                            (dev, ledInterfaceNum, &ledConf),
                            ledConf.ledOrganize);

        /*
            1.6. Call with wrong enum values ledConf.blink0DutyCycle.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                            (dev, ledInterfaceNum, &ledConf),
                            ledConf.blink0DutyCycle);

        /*
            1.7. Call with wrong enum values ledConf.blink0Duration.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                            (dev, ledInterfaceNum, &ledConf),
                            ledConf.blink0Duration);

        /*
            1.8. Call with wrong enum values ledConf.blink1DutyCycle.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                            (dev, ledInterfaceNum, &ledConf),
                            ledConf.blink1DutyCycle);

        /*
            1.9. Call with wrong enum values ledConf.blink1Duration.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                            (dev, ledInterfaceNum, &ledConf),
                            ledConf.blink1Duration);

        /*
            1.10. Call with wrong enum values ledConf.pulseStretch.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                            (dev, ledInterfaceNum, &ledConf),
                            ledConf.pulseStretch);

        /*
            1.11. Call with wrong ledConf.ledStart [256] and the same parameters.
            Expected: GT_OUT_OF_RANGE.
        */
        ledConf.ledStart = 256;

        st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        ledConf.ledStart = 0;

        /*
            1.12. Call with wrong ledConf.ledEnd [256] and the same parameters.
            Expected: GT_OUT_OF_RANGE.
        */
        ledConf.ledEnd = 256;

        st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        ledConf.ledEnd = 1;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {
            /*
                1.12. Call with wrong enum values ledConf.class5select.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                                (dev, ledInterfaceNum, &ledConf),
                                ledConf.class5select);

            /*
                1.13. Call with wrong enum values ledConf.class13select.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamConfigSet
                                (dev, ledInterfaceNum, &ledConf),
                                ledConf.class13select);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamConfigSet(dev, ledInterfaceNum, &ledConf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamConfigGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (dxChx)
    1.1. Call function with ledInterfaceNum [0 - 1], and not null ledConfPtr.
    Expected: NOT GT_OK for lion when ledInterfaceNum == 1 and GT_OK for other cases.
    1.2. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong ledConfPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                dev;
    GT_U32               ledInterfaceNum = 0;
    GT_U32               ledInterfaceMax;
    CPSS_LED_CONF_STC    ledConf;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        ledInterfaceMax = 2;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
        }

        /*
            1.1. Call function with ledInterfaceNum [0 - 1], and not null ledConfPtr.
            Expected: NOT GT_OK for lion when ledInterfaceNum == 1 and GT_OK for other cases.
        */
        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            st = cpssDxChLedStreamConfigGet(dev, ledInterfaceNum, &ledConf);

            if (ledInterfaceNum == 1 &&
                ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                 (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
            {
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
            {
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        /*
            1.2. Call with wrong ledInterfaceNum [6] and the same parameters.
            Expected: NOT GT_OK.
        */
        ledInterfaceNum = ledInterfaceMax;

        st = cpssDxChLedStreamConfigGet(dev, ledInterfaceNum, &ledConf);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        /*
            1.3. Call with wrong ledConfPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamConfigGet(dev, ledInterfaceNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamConfigGet(dev, ledInterfaceNum, &ledConf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamConfigGet(dev, ledInterfaceNum, &ledConf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamGroupConfigSet)
{
/*
    ITERATE_DEVICES(dxChx)
    1.1. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            groupNum [0 - 3],
                        and groupParamsPtr{classA[0],
                                           classB[1],
                                           classC[2],
                                           classD[3]}.
    Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
    1.2. Call cpssDxChLedStreamGroupConfigGet.
    Expected: GT_OK and the same groupParamsPtr.
    1.3. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            groupNum [0 - 1],
                        and groupParamsPtr{classA[5],
                                           classB[6],
                                           classC[7],
                                           classD[8]}.
    Expected: NOT GT_OK for Lion and GT_OK for other cases.
    1.4. Call cpssDxChLedStreamGroupConfigGet.
    Expected: GT_OK and the same enablePtr.
    1.5. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.7. Call with groupNum [2] and
         portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
    Expected: NOT GT_OK.
    1.8. Call with wrong groupNum [4] and the same parameters.
    Expected: GT_BAD_PARAM.
    1.9. Call with wrong groupParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.10. Call with wrong groupParamsPtr->classA [16].
    Expected: GT_OUT_OF_RANGE.
    1.11. Call with wrong groupParamsPtr->classB [16].
    Expected: GT_OUT_OF_RANGE.
    1.12. Call with wrong groupParamsPtr->classC [16].
    Expected: GT_OUT_OF_RANGE.
    1.13. Call with wrong groupParamsPtr->classD [16].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_U32                          ledInterfaceNum = 0;
    GT_U32                          ledInterfaceMax;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                          groupNum        = 0;
    GT_U32                          groupNumMax;
    CPSS_LED_GROUP_CONF_STC         groupParams;
    CPSS_LED_GROUP_CONF_STC         groupParamsGet;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &groupParams, sizeof(groupParams));
    cpssOsBzero((GT_VOID*) &groupParamsGet, sizeof(groupParamsGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        ledInterfaceMax = 2;
        groupNumMax = 3;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            groupNumMax = 2;
        }
        /*
            1.1. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                    groupNum [0 - 3],
                                and groupParamsPtr{classA[0],
                                                   classB[1],
                                                   classC[2],
                                                   classD[3]}.
            Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
        */
        portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
        groupParams.classA = 0;
        groupParams.classB = 1;
        groupParams.classC = 2;
        groupParams.classD = 3;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < groupNumMax; groupNum++)
            {
                st = cpssDxChLedStreamGroupConfigSet(dev,
                             ledInterfaceNum, portType, groupNum, &groupParams);

                if (( ledInterfaceNum == 1) &&
                     ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.2. Call cpssDxChLedStreamGroupConfigGet.
                        Expected: GT_OK and the same groupParamsPtr.
                    */
                    st = cpssDxChLedStreamGroupConfigGet(dev,
                              ledInterfaceNum, portType, groupNum, &groupParamsGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamGroupConfigGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classA,
                                                 groupParamsGet.classA,
                               "get another groupParams->classA than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classB,
                                                 groupParamsGet.classB,
                               "get another groupParams->classB than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classC,
                                                 groupParamsGet.classC,
                               "get another groupParams->classC than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classD,
                                                 groupParamsGet.classD,
                               "get another groupParams->classD than was set: %d", dev);
                }
            }
        }
        /*
            1.3. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                    groupNum [0 - 1],
                                and groupParamsPtr{classA[5],
                                                   classB[6],
                                                   classC[7],
                                                   classD[8]}.
            Expected: NOT GT_OK for Lion, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 and GT_OK for other cases.
        */
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        groupParams.classA = 5;
        groupParams.classB = 6;
        groupParams.classC = 7;
        groupParams.classD = 8;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            groupParams.classA = 4;
            groupParams.classB = 5;
            groupParams.classC = 6;
            groupParams.classD = 7;
        }
        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < 2; groupNum++)
            {
                st = cpssDxChLedStreamGroupConfigSet(dev,
                             ledInterfaceNum, portType, groupNum, &groupParams);

                if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    /*
                        1.4. Call cpssDxChLedStreamGroupConfigGet.
                        Expected: GT_OK and the same enablePtr.
                    */
                    st = cpssDxChLedStreamGroupConfigGet(dev,
                             ledInterfaceNum, portType, groupNum, &groupParamsGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                   "cpssDxChLedStreamGroupConfigGet: %d", dev);

                    /* Verifying struct fields */
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classA,
                                                 groupParamsGet.classA,
                               "get another groupParams->classA than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classB,
                                                 groupParamsGet.classB,
                               "get another groupParams->classB than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classC,
                                                 groupParamsGet.classC,
                               "get another groupParams->classC than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(groupParams.classD,
                                                 groupParamsGet.classD,
                               "get another groupParams->classD than was set: %d", dev);
                }
            }
        }
        /*
            1.5. Call with wrong ledInterfaceNum [6] and the same parameters.
            Expected: NOT GT_OK.
        */
        groupNum        = 0;
        ledInterfaceNum = ledInterfaceMax;

        st = cpssDxChLedStreamGroupConfigSet(dev,
                             ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {
            /*
                1.6. Call with wrong enum values portType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamGroupConfigSet
                                (dev, ledInterfaceNum,
                                 portType, groupNum, &groupParams),
                                portType);
        }

        /*
            1.7. Call with groupNum [6] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
            Expected: NOT GT_OK.
        */
        groupNum = 6;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        st = cpssDxChLedStreamGroupConfigSet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.8. Call with wrong groupNum [4] and the same parameters.
            Expected: NOT GT_OK.
        */
        groupNum = 4;
        portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;

        st = cpssDxChLedStreamGroupConfigSet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        groupNum = 0;
        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        /*
            1.9. Call with wrong groupParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamGroupConfigSet(dev,
                     ledInterfaceNum, portType, groupNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.10. Call with wrong groupParamsPtr->classA [16].
            Expected: GT_OUT_OF_RANGE.
        */
        groupParams.classA = 16;

        st = cpssDxChLedStreamGroupConfigSet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classA = 0;

        /*
            1.11. Call with wrong groupParamsPtr->classB [16].
            Expected: GT_OUT_OF_RANGE.
        */
        groupParams.classB = 16;

        st = cpssDxChLedStreamGroupConfigSet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classB = 0;

        /*
            1.12. Call with wrong groupParamsPtr->classC [16].
            Expected: GT_OUT_OF_RANGE.
        */
        groupParams.classC = 16;

        st = cpssDxChLedStreamGroupConfigSet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classC = 0;

        /*
            1.13. Call with wrong groupParamsPtr->classD [16].
            Expected: GT_OUT_OF_RANGE.
        */
        groupParams.classD = 16;

        st = cpssDxChLedStreamGroupConfigSet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        groupParams.classD = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamGroupConfigSet(dev,
                   ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamGroupConfigSet(dev,
                 ledInterfaceNum, portType, groupNum, &groupParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamGroupConfigGet)
{
/*
    ITERATE_DEVICES(dxChx)
    1.1. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                            groupNum [0 - 3],
                            and not null groupParamsPtr.
    Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
    1.2. Call function with ledInterfaceNum [0 / 1],
                            portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                            groupNum [0 - 1],
                            and not null groupParamsPtr.
    Expected: NOT GT_OK for Lion and GT_OK for other cases.
    1.3. Call with wrong ledInterfaceNum [2] and the same parameters.
    Expected: NOT GT_OK.
    1.4. Call with wrong enum values portType.
    Expected: GT_BAD_PARAM.
    1.5. Call with groupNum [2] and
         portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
    Expected: NOT GT_OK.
    1.6. Call with wrong groupNum [4] and the same parameters.
    Expected: NOT GT_OK.
    1.7. Call with wrong groupParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_U32                          ledInterfaceNum = 0;
    GT_U32                          ledInterfaceMax;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType        = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;
    GT_U32                          groupNum        = 0;
    GT_U32                          groupNumMax;
    CPSS_LED_GROUP_CONF_STC         groupParams;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        ledInterfaceMax = 2;
        groupNumMax = 3;
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Bobcat2 A0 not supported  */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
            {
                continue;
            }
            ledInterfaceMax = prvCpssLedStreamNumOfInterfacesInPortGroupGet(dev);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, ledInterfaceMax, dev);
            groupNumMax = 2;
        }

        /*
            1.1. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E],
                                    groupNum [0 - 3],
                                    and not null groupParamsPtr.
            Expected: NOT GT_OK for Lion when ledInterfaceNum == 1 and GT_OK for other cases.
        */
        portType = CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < groupNumMax; groupNum++)
            {
                st = cpssDxChLedStreamGroupConfigGet(dev,
                             ledInterfaceNum, portType, groupNum, &groupParams);

                if ( (ledInterfaceNum == 1) &&
                     ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                      (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }

            }
        }
        /*
            1.2. Call function with ledInterfaceNum [0 / 1],
                                    portType [CPSS_DXCH_LED_PORT_TYPE_XG_E],
                                    groupNum [0 - 1],
                                    and not null groupParamsPtr.
            Expected: NOT GT_OK for Lion and GT_OK for other cases.
        */
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        for(ledInterfaceNum = 0; ledInterfaceNum < ledInterfaceMax; ledInterfaceNum++)
        {
            for(groupNum = 0; groupNum < 2; groupNum++)
            {
                st = cpssDxChLedStreamGroupConfigGet(dev,
                           ledInterfaceNum, portType, groupNum, &groupParams);

                if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE) && (ledInterfaceNum < 3))
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
            }
        }
        /*
            1.3. Call with wrong ledInterfaceNum [6] and the same parameters.
            Expected: NOT GT_OK.
        */
        groupNum        = 0;
        ledInterfaceNum = ledInterfaceMax;

        st = cpssDxChLedStreamGroupConfigGet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ledInterfaceNum = 0;
        if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE))
        {
            ledInterfaceNum = ledInterfaceMax - 1;
        }

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == 0)
        {
            /*
                1.4. Call with wrong enum values portType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChLedStreamGroupConfigGet
                                (dev, ledInterfaceNum,
                                 portType, groupNum, &groupParams),
                                portType);
        }

        /*
            1.5. Call with groupNum [2] and
                 portType [CPSS_DXCH_LED_PORT_TYPE_XG_E] (incorrect groupNum).
            Expected: NOT GT_OK.
        */
        groupNum = 2;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        st = cpssDxChLedStreamGroupConfigGet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        groupNum = 0;

        /*
            1.6. Call with wrong groupNum [4] and the same parameters.
            Expected: NOT GT_OK.
        */
        groupNum = 4;

        st = cpssDxChLedStreamGroupConfigGet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        groupNum = 0;

        /*
            1.7. Call with wrong groupParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamGroupConfigGet(dev,
                     ledInterfaceNum, portType, groupNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamGroupConfigGet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamGroupConfigGet(dev,
                     ledInterfaceNum, portType, groupNum, &groupParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamSyncLedsEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
    OUT GT_BOOL  *inSyncTrigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamSyncLedsEnableGet)
{
/*
    ITERATE_DEVICES (xCat2)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with null inSyncTrigPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;
    GT_BOOL     inSyncTrig = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E
                                         | UTF_CH1_DIAMOND_E
                                         | UTF_CH2_E
                                         | UTF_CH3_E
                                         | UTF_XCAT_E | UTF_XCAT3_E
                                         | UTF_LION_E
                                         | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChLedStreamSyncLedsEnableGet(dev, &state, &inSyncTrig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamSyncLedsEnableGet(dev, NULL, &inSyncTrig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /*
            1.3. Call with null inSyncTrigPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChLedStreamSyncLedsEnableGet(dev, &state, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E
                                         | UTF_CH1_DIAMOND_E
                                         | UTF_CH2_E
                                         | UTF_CH3_E
                                         | UTF_XCAT_E | UTF_XCAT3_E
                                         | UTF_LION_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamSyncLedsEnableGet(dev, &state, &inSyncTrig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamSyncLedsEnableGet(dev, &state, &inSyncTrig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamSyncLedsEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamSyncLedsEnableSet)
{
/*
    ITERATE_DEVICES (xCat2)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamSyncLedsEnableGet.
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;
    GT_BOOL     inSyncTrig = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E
                                         | UTF_CH1_DIAMOND_E
                                         | UTF_CH2_E
                                         | UTF_CH3_E
                                         | UTF_XCAT_E | UTF_XCAT3_E
                                         | UTF_LION_E
                                         | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChLedStreamSyncLedsEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamSyncLedsEnableGet.
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamSyncLedsEnableGet(dev, &stateGet, &inSyncTrig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamSyncLedsEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChLedStreamSyncLedsEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChLedStreamSyncLedsEnableGet.
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChLedStreamSyncLedsEnableGet(dev, &stateGet, &inSyncTrig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChLedStreamSyncLedsEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E
                                         | UTF_CH1_DIAMOND_E
                                         | UTF_CH2_E
                                         | UTF_CH3_E
                                         | UTF_XCAT_E | UTF_XCAT3_E
                                         | UTF_LION_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamSyncLedsEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamSyncLedsEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet
(
    IN GT_U8            devNum,
    IN GT_U8            portNum,
    IN GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (xCat2)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum      = 0;
    GT_PHYSICAL_PORT_NUM         portNum     = 0;
    GT_BOOL             enable      = GT_FALSE;
    GT_BOOL             enableGet   = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                              UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* Relevant only in xCat2 A1. */
        if(PRV_CPSS_PP_MAC(devNum)->revision >= 2)
        {
            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
            {
                /*
                    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                    Expected: GT_OK.
                */
                enable = GT_TRUE;

                st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                                          portNum,
                                                                          enable);
                if(GT_TRUE == PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum, portNum))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

                    /*
                        1.1.2. Call cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet.
                        Expected: GT_OK and the same enable.
                    */
                    st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                              portNum,
                                                                              &enableGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet: %d",
                        devNum);
                    UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                        "get another enable than was set: %d", devNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,
                                                portNum);
                }

                /* Call with enable [GT_FALSE]. */
                enable = GT_FALSE;

                st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                                          portNum,
                                                                          enable);
                if(GT_TRUE == PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum, portNum))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

                    /*
                        1.1.2. Call cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet.
                        Expected: GT_OK and the same enable.
                    */
                    st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                              portNum,
                                                                              &enableGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet: %d",
                        devNum);
                    UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                        "get another enable than was set: %d", devNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,
                                                portNum);
                }
            }

            enable = GT_TRUE;

            st = prvUtfNextVirtPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                                          portNum,
                                                                          enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for portNum number.                      */
            portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

            st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                                      portNum,
                                                                      enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                            */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                                      portNum,
                                                                      enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
        else
        {
            portNum = 0;
            enable  = GT_TRUE;

            /* For non xCat2 A1 check that function returns non             */
            /*   GT_NOT_APPLICABLE_DEVICE.                                  */
            st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                                      portNum,
                                                                      enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                              UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                                  portNum,
                                                                  enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet(devNum,
                                                              portNum,
                                                              enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_U8           portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (xCat2)
    1.1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum  = 0;
    GT_PHYSICAL_PORT_NUM            portNum = 0;
    GT_BOOL             enable  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                              UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* Relevant only in xCat2 A1. */
        if(PRV_CPSS_PP_MAC(devNum)->revision >= 2)
        {
            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
            {
                /*
                    1.1. Call function with not NULL enablePtr.
                    Expected: GT_OK.
                */
                st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                          portNum,
                                                                          &enable);
                if(GT_TRUE == PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum, portNum))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum,
                                                &enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                }

                if(GT_TRUE == PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum, portNum))
                {
                    /*
                        1.2. Call function with NULL enablePtr.
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                              portNum,
                                                                              NULL);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                        "%d, enablePtr = NULL", devNum);
                }
            }

            st = prvUtfNextVirtPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                          portNum,
                                                                          &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for portNum number.                      */
            portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

            st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                      portNum,
                                                                      &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                            */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                      portNum,
                                                                      &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
        else
        {
            portNum = 0;

            /* For non xCat2 A1 check that function returns non             */
            /*   GT_NOT_APPLICABLE_DEVICE.                                  */
            st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                      portNum,
                                                                      &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                              UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                                  portNum,
                                                                  &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet(devNum,
                                                              portNum,
                                                              &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortPositionSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (xCat3)
    1.1.1. Call with position [0, 10].
    Expected: GT_OK.
    1.1.2. Call cpssDxChLedStreamPortPositionGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS           st = GT_OK;

    GT_U8                   devNum      = 0;
    GT_PHYSICAL_PORT_NUM    portNum     = 0;
    GT_U32                  position    = 0;
    GT_U32                  positionGet = 0;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                              UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Bobcat2 A0 not supported  */
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) == 0)
        {
            continue;
        }

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with 0
                Expected: GT_OK.
            */
            position    = 0;

            st = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &positionGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChLedStreamPortPositionSet: %d %d", devNum, portNum);
                UTF_VERIFY_EQUAL2_STRING_MAC(position, positionGet,
                    "get another position than was set: %d %d", devNum, portNum);

                /* aldrin support only 0 position for port 32*/
                if (!((portNum == 32) &&
                      ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)||
                       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))))
                {
                    position    = 10;
                    st = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                    st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &positionGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChLedStreamPortPositionSet: %d %d", devNum, portNum);
                    UTF_VERIFY_EQUAL2_STRING_MAC(position, positionGet,
                        "get another position than was set: %d %d", devNum, portNum);
                }
            }
        }

        position = 0;

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    position = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                              UTF_LION_E | UTF_LION2_E );

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortPositionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *positionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortPositionGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (xCat3)
    1.1.1. Call cpssDxChLedStreamPortPositionGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS           st = GT_OK;

    GT_U8                   devNum      = 0;
    GT_PHYSICAL_PORT_NUM    portNum     = 0;
    GT_U32                  position    = 0;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                              UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Bobcat2 A0 not supported  */
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) == 0)
        {
            continue;
        }

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with
                Expected: GT_OK.
            */

            st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &position);
            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }

        }

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &position);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &position);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &position);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    position = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                              UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &position);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLedStreamPortPositionGet(devNum, portNum, &position);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         invertEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortClassPolarityInvertEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2; Caelum; Bobcat3)
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2; Caelum; Bobcat3)

     1.1. Call function with classNum[0..5],
                     invertEnable[GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamPortClassPolarityInvertEnableGet with the same parameters
    Expected: GT_OK and the same values.
    1.3. Call with wrong classNum [8 / 12].
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM portNum    = 0;
    GT_U32      classNum            = 0;
    GT_BOOL     enable              = GT_FALSE;
    GT_BOOL     enableGet           = GT_FALSE;
    GT_U32      enableState;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Bobcat2 B0 and above */
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
        {
            continue;
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            for(classNum = 0; classNum < 5; classNum++)
            {
                for (enableState = 0; enableState < 2; enableState++)
                {
                    st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev, portNum,
                                            classNum, enable);
                    if(prvCpssDxChPortRemotePortCheck(dev,portNum))
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                                portNum, classNum, enable);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                                                portNum, classNum, enable);
                        if (st == GT_OK)
                        {

                            /*
                                1.2. Call cpssDxChLedStreamPortClassPolarityInvertEnableGet
                                Expected: GT_OK and the same enable state.
                            */
                            st = cpssDxChLedStreamPortClassPolarityInvertEnableGet(dev,
                                                portNum, classNum, &enableGet);
                            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                "cpssDxChLedStreamPortClassPolarityInvertEnableGet: %d %d %d %d",
                                            dev, portNum, classNum, enable);
                            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                "get another enable than was set: %d", dev);
                        }
                    }

                    /* Change enable state to opposite */
                    enable = !enable;
                }
            }

            /*
                1.3. Call with wrong classNum [8].
                Expected: GT_BAD_PARAM
            */
            classNum = 8;
            st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev, portNum,
                                    classNum, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                    portNum, classNum, enable);
        }

        enable = GT_TRUE;
        classNum = 0;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 2.1. Call function for each non-active port */
            st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev,
                                                                   portNum,
                                                                   classNum,
                                                                   enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 2.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

        st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev,
                                                               portNum,
                                                               classNum,
                                                               enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 2.3. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev,
                                                               portNum,
                                                               classNum,
                                                               enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 3. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    portNum = 0;
    enable = GT_TRUE;
    classNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev,
                                portNum, classNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.1 Call function without of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev,
                                                           portNum,
                                                           classNum,
                                                           enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         *invertEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortClassPolarityInvertEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2; Caelum; Bobcat3)
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2; Caelum; Bobcat3)

     1.1. Call function with classNum[0..5]
    Expected: GT_OK.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM portNum    = 0;
    GT_U32      classNum            = 0;
    GT_BOOL     enableGet           = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Bobcat2 B0 and above */
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
        {
            continue;
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            /*  Call for all valid classes */
            for(classNum = 0; classNum < 5; classNum++)
            {
                st = cpssDxChLedStreamPortClassPolarityInvertEnableGet(dev, portNum, classNum, &enableGet);
                if(prvCpssDxChPortRemotePortCheck(dev,portNum))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                }
            }
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLedStreamPortClassPolarityInvertEnableGet(dev, portNum, classNum, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChLedStreamPortClassPolarityInvertEnableGet(dev, portNum, classNum, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChLedStreamPortClassPolarityInvertEnableGet(dev, portNum, classNum, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    classNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                              UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamPortClassPolarityInvertEnableGet(dev, portNum, classNum, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLedStreamPortClassPolarityInvertEnableGet(dev, portNum, classNum, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortClassIndicationSet)
{
/*
    ITERATE_DEVICES (Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2)
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2)

     1.1. Call function with classNum  [0..5],
                             indication[CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E
                                        CPSS_DXCH_LED_INDICATION_LINK_E
                                        CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E
                                        CPSS_DXCH_LED_INDICATION_RX_ERROR_E
                                        CPSS_DXCH_LED_INDICATION_FC_TX_E
                                        CPSS_DXCH_LED_INDICATION_FC_RX_E
                                        CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E
                                        CPSS_DXCH_LED_INDICATION_GMII_SPEED_E
                                        CPSS_DXCH_LED_INDICATION_MII_SPEED_E]
                                        CPSS_DXCH_LED_INDICATION_COLLISION_E]
    Expected: GT_OK.
    1.2. Call cpssDxChLedStreamPortClassPolarityInvertEnableGet with the same parameters
    Expected: GT_OK and the same values.
    1.3. Call with wrong classNum [8 / 12].
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;
    GT_U8       dev = 0, indication = 0;
    GT_PHYSICAL_PORT_NUM portNum    = 0;
    GT_U32      classNum            = 0;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_DXCH_LED_INDICATION_ENT indicationGet = 0;
    CPSS_DXCH_LED_INDICATION_ENT indicationVal[10] = {CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E,
                                                     CPSS_DXCH_LED_INDICATION_LINK_E,
                                                     CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E,
                                                     CPSS_DXCH_LED_INDICATION_RX_ERROR_E,
                                                     CPSS_DXCH_LED_INDICATION_FC_TX_E,
                                                     CPSS_DXCH_LED_INDICATION_FC_RX_E,
                                                     CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E,
                                                     CPSS_DXCH_LED_INDICATION_GMII_SPEED_E,
                                                     CPSS_DXCH_LED_INDICATION_MII_SPEED_E,
                                                     CPSS_DXCH_LED_INDICATION_COLLISION_E};

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
            UTF_CH1_E|UTF_CH1_DIAMOND_E|UTF_CH2_E|UTF_LION_E|UTF_XCAT_E|UTF_XCAT3_E|UTF_XCAT2_E|UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Bobcat2 B0 and above */
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
        {
            continue;
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            for(classNum = 0; classNum < 5; classNum++)
            {
                for (indication = 0; indication < 9; indication++ )
                {
                    st = cpssDxChLedStreamPortClassIndicationSet(dev, portNum,
                                            classNum, indicationVal[indication]);
                    if(prvCpssDxChPortRemotePortCheck(dev,portNum))
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                                portNum, classNum, indicationVal[indication]);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                                                portNum, classNum, indicationVal[indication]);
                        if (st == GT_OK)
                        {
                            /*
                                1.2. Call cpssDxChLedStreamPortClassIndicationGet
                                Expected: GT_OK and the same indication state.
                            */
                            st = cpssDxChLedStreamPortClassIndicationGet(dev,
                                                portNum, classNum, &indicationGet);
                            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                "cpssDxChLedStreamPortClassIndicationGet: %d %d %d %d",
                                            dev, portNum, classNum, indicationVal[indication]);
                            UTF_VERIFY_EQUAL1_STRING_MAC(indicationVal[indication], indicationGet,
                                "get another indication than was set: %d", dev);
                        }
                    }
                }
            }

            /*
                1.3. Call with wrong classNum [8].
                Expected: GT_BAD_PARAM
            */
            classNum = 8;
            st = cpssDxChLedStreamPortClassIndicationSet(dev, portNum,
                                    classNum, CPSS_DXCH_LED_INDICATION_GMII_SPEED_E);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                    portNum, classNum, CPSS_DXCH_LED_INDICATION_GMII_SPEED_E);
        }

        classNum = 0;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 2.1. Call function for each non-active port */
            st = cpssDxChLedStreamPortClassIndicationSet(dev, portNum, classNum,
                                                CPSS_DXCH_LED_INDICATION_GMII_SPEED_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 2.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

        st = cpssDxChLedStreamPortClassIndicationSet(dev, portNum, classNum,
                                       CPSS_DXCH_LED_INDICATION_GMII_SPEED_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 2.3. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChLedStreamPortClassPolarityInvertEnableSet(dev, portNum, classNum,
                                                     CPSS_DXCH_LED_INDICATION_GMII_SPEED_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 3. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    portNum = 0;
    classNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamPortClassIndicationSet(dev,
                                portNum, classNum, CPSS_DXCH_LED_INDICATION_GMII_SPEED_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.1 Call function without of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedStreamPortClassIndicationSet(dev, portNum, classNum,
                                    CPSS_DXCH_LED_INDICATION_GMII_SPEED_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedStreamPortClassIndicationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    *inndicationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedStreamPortClassIndicationGet)
{
/*
    ITERATE_DEVICES (Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2)
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2)

     1.1. Call function with classNum[0..5]
    Expected: GT_OK.
*/

    GT_STATUS                       st = GT_OK;
    GT_U8                           dev = 0;
    GT_PHYSICAL_PORT_NUM            portNum = 0;
    GT_U32                          classNum = 0;
    CPSS_DXCH_LED_INDICATION_ENT    indicationGet = 0;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
            UTF_CH1_E|UTF_CH1_DIAMOND_E|UTF_CH2_E|UTF_LION_E|UTF_XCAT_E|UTF_XCAT3_E|UTF_XCAT2_E|UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Bobcat2 B0 and above */
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev) == 0)
        {
            continue;
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            /*  Call for all valid classes */
            for(classNum = 0; classNum < 5; classNum++)
            {
                st = cpssDxChLedStreamPortClassIndicationGet(dev, portNum, classNum, &indicationGet);
                if(prvCpssDxChPortRemotePortCheck(dev,portNum))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                }
            }
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLedStreamPortClassIndicationGet(dev, portNum, classNum, &indicationGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChLedStreamPortClassIndicationGet(dev, portNum, classNum, &indicationGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChLedStreamPortClassIndicationGet(dev, portNum, classNum, &indicationGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    classNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                              UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedStreamPortClassIndicationGet(dev, portNum, classNum, &indicationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLedStreamPortClassIndicationGet(dev, portNum, classNum, &indicationGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* UTF_TEST_CASE_MAC(cpssDxChLedBc3FE) *
 *   PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E
 *      all not used mac's LED port number shall be set to 0x3F
 *      (Errata FE-3763116 )
 */


UTF_TEST_CASE_MAC(cpssDxChLedBc3FELedPosition)
{
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM portNum    = 0;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    GT_U32             ledPort;
    GT_U32             geMacLedPos;
    GT_U32             xlgMacLedPos;
    GT_U32             cgMacLedPos;
    GT_BOOL            cgMacValid;
    CPSS_PORTS_BMP_STC initPortsBmp;/* bitmap of ports to init */

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                           UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Bobcat2 B0 and above */
        if (GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
        {
            continue;
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPhysicalPortDetailedMapGet : dev %d port %d",dev,portNum);

            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d", dev, portNum);

            st = cpssDxChPortSpeedGet(dev,portNum,/*OUT*/&speed);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortSpeedGet: %d, %d", dev, portNum);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(CPSS_PORT_SPEED_NA_E, st, "cpssDxChPortSpeedGet : dev %d port %d interface  %d != CPSS_PORT_INTERFACE_MODE_NA_E",dev,portNum,ifMode);

            cgMacValid = GT_FALSE;

            st = prvCpssDxChIsCgUnitInUse(dev, portNum, ifMode, speed, &cgMacValid);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChIsCgUnitInUse : dev %d port %d",dev,portNum);

            st = cpssDxChLedStreamPortPositionGet(dev,portNum,&ledPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChLedStreamPortPositionGet : dev %d port %d",dev,portNum);

            st = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_GE_E, /*OUT*/&geMacLedPos);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d MAC GE",dev,portNum);


            st = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_XLG_E,/*OUT*/&xlgMacLedPos);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d MAC GE",dev,portNum);

            if (cgMacValid == GT_TRUE)
            {
                st = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_CG_E, /*OUT*/&cgMacLedPos);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d MAC CG",dev,portNum);
            }

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) /* port not initialized, check for 0x3F in [GE/XLG/CG]MACs led position */
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,geMacLedPos,  "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d GE MAC led postion != 0x3f",dev,portNum);
                UTF_VERIFY_EQUAL2_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,xlgMacLedPos, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d XLG MAC led postion != 0x3f",dev,portNum);
                if (cgMacValid == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,cgMacLedPos,  "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d CG MAC led postion != 0x3f",dev,portNum);
                }
            }
            else
            {
                PRV_CPSS_PORT_TYPE_ENT macType;
                st = cpssDxChPortSpeedGet(dev,portNum,/*OUT*/&speed);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortSpeedGet: %d, %d", dev, portNum);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(CPSS_PORT_SPEED_NA_E, st, "cpssDxChPortSpeedGet : dev %d port %d interface  %d != CPSS_PORT_INTERFACE_MODE_NA_E",dev,portNum,ifMode);

                macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(dev,portMapPtr->portMap.macNum);

                /*-----------------------------------------------------------------------------------------------*
                    *  verify non active macs for 0x3f,
                    *  verify active mac for led position retrieved by cpssDxChLedStreamPortPositionGet() from DB
                    *
                    *-----------------------------------------------------------------------------------------------*/
                switch (macType)
                {
                    case PRV_CPSS_PORT_GE_E:
                        UTF_VERIFY_EQUAL4_STRING_MAC(ledPort,geMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d GE MAC led postion %d!= %d",dev,portNum,geMacLedPos,ledPort);
                        UTF_VERIFY_EQUAL3_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,xlgMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d XLG MAC led postion %d != 0x3f",dev,portNum,xlgMacLedPos);
                        if (cgMacValid == GT_TRUE)
                        {
                            UTF_VERIFY_EQUAL3_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,cgMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d CG MAC led postion %d != 0x3f",dev,portNum,cgMacLedPos);
                        }
                    break;
                    case PRV_CPSS_PORT_XLG_E:
                        UTF_VERIFY_EQUAL3_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,geMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d GE MAC led postion %d != 0x3f",dev,portNum,geMacLedPos);
                        UTF_VERIFY_EQUAL4_STRING_MAC(ledPort,xlgMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d XLG MAC led postion %d != %d",dev,portNum,xlgMacLedPos,ledPort);
                        if (cgMacValid == GT_TRUE)
                        {
                            UTF_VERIFY_EQUAL3_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,cgMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d CG MAC led postion %d != 0x3f",dev,portNum,cgMacLedPos);
                        }
                    break;
                    case PRV_CPSS_PORT_CG_E:
                        UTF_VERIFY_EQUAL3_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,geMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d GE MAC led postion %d != 0x3f",dev,portNum,geMacLedPos);
                        UTF_VERIFY_EQUAL3_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,xlgMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d XLG MAC led postion %d != 0x3f",dev,portNum,xlgMacLedPos);
                        if (cgMacValid == GT_TRUE)
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(ledPort,cgMacLedPos,"prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d CG MAC led postion %d!= %d",dev,portNum,cgMacLedPos,ledPort);
                        }
                    break;
                    default:
                        {
                            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_FAIL,"undefined mac type %d",macType);
                        }
                }
                /*-----------------------------------------------------------------------------------------------*
                    *  now try to power off the port.
                    *  verify all mac have PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS led position
                    *-----------------------------------------------------------------------------------------------*/
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
                st = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_FALSE,ifMode, speed);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK,st, "cpssDxChPortModeSpeedSet : POWER-DOWN dev %d port %d",dev,portNum);

                /* read position and  veryfy*/
                st = cpssDxChLedStreamPortPositionGet(dev,portNum,&ledPort);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChLedStreamPortPositionGet : dev %d port %d",dev,portNum);

                st = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_GE_E, /*OUT*/&geMacLedPos);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d MAC GE",dev,portNum);


                st = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_XLG_E,/*OUT*/&xlgMacLedPos);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d MAC GE",dev,portNum);

                if (cgMacValid == GT_TRUE)
                {
                    st = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_CG_E, /*OUT*/&cgMacLedPos);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d MAC CG",dev,portNum);
                }
                /* verify */
                UTF_VERIFY_EQUAL2_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,geMacLedPos,  "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d GE MAC led postion != 0x3f",dev,portNum);
                UTF_VERIFY_EQUAL2_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,xlgMacLedPos, "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d XLG MAC led postion != 0x3f",dev,portNum);
                if (cgMacValid == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS,cgMacLedPos,  "prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet : dev %d port %d CG MAC led postion != 0x3f",dev,portNum);
                }
                /* return port to its initial state */
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
                st = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE,ifMode, speed);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK,st, "cpssDxChPortModeSpeedSet : POWER-UP : dev %d port %d",dev,portNum);
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedPhyControlGlobalSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedPhyControlGlobalSet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function with good parameters
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
    1.2. Call cpssDxChLedPhyControlGlobalGet with the same parameters
    Expected: GT_OK and the same values.
    1.3. Call with wrong parameters.
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    GT_BOOL                             iter;                   /* Just an iterator */
    CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC   ledGlobalConfigSave;
    CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC   ledGlobalConfig;
    CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC   ledGlobalConfigGet;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;
            cpssOsMemSet (&ledGlobalConfigSave, 0, sizeof(ledGlobalConfigSave));
            st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigSave);
            UTF_VERIFY_EQUAL0_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st);

            for (iter = 0; iter < 2; iter++)
            {
                ledGlobalConfig.directMode = GT_FALSE;
                ledGlobalConfig.skipColumns = (2*portNum+iter) % BIT_4;
                ledGlobalConfig.specialLed1Bitmap  = 0x55 << iter;
                ledGlobalConfig.specialLed2Bitmap  = 0xAA << iter;
                ledGlobalConfig.specialLed3Bitmap  = 1 << ((2*portNum+iter) % 9);

                st = cpssDxChLedPhyControlGlobalSet(dev, portNum, &ledGlobalConfig);
                if (isExtendedCascadePort)
                {
                    UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, dev, portNum,
                                            ledGlobalConfig.directMode,
                                            ledGlobalConfig.skipColumns,
                                            ledGlobalConfig.specialLed1Bitmap,
                                            ledGlobalConfig.specialLed2Bitmap,
                                            ledGlobalConfig.specialLed3Bitmap);
                }
                else
                {
                    UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum,
                                            ledGlobalConfig.directMode,
                                            ledGlobalConfig.skipColumns,
                                            ledGlobalConfig.specialLed1Bitmap,
                                            ledGlobalConfig.specialLed2Bitmap,
                                            ledGlobalConfig.specialLed3Bitmap);
                }
                if (st == GT_OK)
                {

                    /*
                        1.2. Call cpssDxChLedPhyControlGlobalGet
                        Expected: GT_OK and the same state.
                    */
                    st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigGet);
                    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, st,
                        "cpssDxChLedPhyControlGlobalGet: %d %d %d %d %d %d %d",
                                    dev, portNum,
                                        ledGlobalConfig.directMode,
                                        ledGlobalConfig.skipColumns,
                                        ledGlobalConfig.specialLed1Bitmap,
                                        ledGlobalConfig.specialLed2Bitmap,
                                        ledGlobalConfig.specialLed3Bitmap);
                    UTF_VERIFY_EQUAL2_STRING_MAC(0,
                        cpssOsMemCmp (&ledGlobalConfig,
                                        &ledGlobalConfigGet,
                                        sizeof(ledGlobalConfig)),
                        "get different data than was set: %d/%d", dev, portNum);
                }
            }

            /*
                1.3. Call with wrong skipColumns [BIT_8].
                Expected: GT_BAD_PARAM
            */
            ledGlobalConfig.skipColumns = BIT_8;
            st = cpssDxChLedPhyControlGlobalSet(dev, portNum, &ledGlobalConfig);
            UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum,
                                ledGlobalConfig.directMode,
                                ledGlobalConfig.skipColumns,
                                ledGlobalConfig.specialLed1Bitmap,
                                ledGlobalConfig.specialLed2Bitmap,
                                ledGlobalConfig.specialLed3Bitmap);

            ledGlobalConfig.skipColumns = 0;

            st = cpssDxChLedPhyControlGlobalSet(dev, portNum, &ledGlobalConfigSave);
            UTF_VERIFY_EQUAL0_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 2.1. Call function for each non-active port */
            st = cpssDxChLedPhyControlGlobalSet(dev,
                                                   portNum,
                                                   &ledGlobalConfig);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 2.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

        st = cpssDxChLedPhyControlGlobalSet(dev,
                                               portNum,
                                               &ledGlobalConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 2.3. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChLedPhyControlGlobalSet(dev,
                                               portNum,
                                               &ledGlobalConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 3. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedPhyControlGlobalSet(dev, portNum, &ledGlobalConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.1 Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedPhyControlGlobalSet(dev, portNum, &ledGlobalConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedPhyControlGlobalGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedPhyControlGlobalGet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC   ledGlobalConfigGet;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;

            st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st, dev, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLedPhyControlGlobalGet(dev, portNum, &ledGlobalConfigGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedPhyControlPerPortSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_PORT_CONF_STC  *ledPerPortConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedPhyControlPerPortSet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function with good parameters
    Expected: GT_OK on remote ports, GT_BAD_PARAM on other ports.
    1.2. Call cpssDxChLedPhyControlPerPortGet with the same parameters
    Expected: GT_OK and the same values.
    1.3. Call with wrong parameters.
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM            portNum    = 0;
    GT_BOOL                         isRemotePort;
    GT_BOOL                         iter;                   /* Just an iterator */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_DXCH_LED_PHY_PORT_CONF_STC  ledPerPortConfigSave;
    CPSS_DXCH_LED_PHY_PORT_CONF_STC  ledPerPortConfig;
    CPSS_DXCH_LED_PHY_PORT_CONF_STC  ledPerPortConfigGet;
    static CPSS_DXCH_LED_PHY_SELECT_ENT goodLed0SelectValues[] =
    {
        CPSS_DXCH_LED_PHY_LINK_ACT_SPEED,
        CPSS_DXCH_LED_PHY_LINK_ACT,
        CPSS_DXCH_LED_PHY_LINK,
        CPSS_DXCH_LED_PHY_10_LINK_ACT,
        CPSS_DXCH_LED_PHY_10_LINK,
        CPSS_DXCH_LED_PHY_1000_LINK_ACT,
        CPSS_DXCH_LED_PHY_10_1000_LINK_ACT,
        CPSS_DXCH_LED_PHY_100_1000_LINK_ACT,
        CPSS_DXCH_LED_PHY_100_1000_LINK,
        CPSS_DXCH_LED_PHY_SPECIAL,
        CPSS_DXCH_LED_PHY_DUPLEX_COL,
        CPSS_DXCH_LED_PHY_FORCE_BLINK,
        CPSS_DXCH_LED_PHY_FORCE_OFF,
        CPSS_DXCH_LED_PHY_FORCE_ON
    };
    static CPSS_DXCH_LED_PHY_SELECT_ENT goodLed1SelectValues[] =
    {
        CPSS_DXCH_LED_PHY_LINK_ACT,
        CPSS_DXCH_LED_PHY_100_LINK_ACT,
        CPSS_DXCH_LED_PHY_100_LINK,
        CPSS_DXCH_LED_PHY_1000_LINK,
        CPSS_DXCH_LED_PHY_10_100_LINK_ACT,
        CPSS_DXCH_LED_PHY_10_100_LINK,
        CPSS_DXCH_LED_PHY_10_1000_LINK_ACT,
        CPSS_DXCH_LED_PHY_10_1000_LINK,
        CPSS_DXCH_LED_PHY_SPECIAL,
        CPSS_DXCH_LED_PHY_ACTIVITY,
        CPSS_DXCH_LED_PHY_FORCE_BLINK,
        CPSS_DXCH_LED_PHY_FORCE_OFF,
        CPSS_DXCH_LED_PHY_FORCE_ON
    };
    static GT_U32      goodLed0SelectValuesSize = sizeof(goodLed0SelectValues) / sizeof(goodLed0SelectValues[0]);
    static GT_U32      goodLed1SelectValuesSize = sizeof(goodLed1SelectValues) / sizeof(goodLed1SelectValues[0]);

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,portNum);
            cpssOsMemSet (&ledPerPortConfigSave, 0, sizeof(ledPerPortConfigSave));
            st = cpssDxChLedPhyControlPerPortGet(dev, portNum,
                                    &ledPerPortConfigSave);
            UTF_VERIFY_EQUAL0_PARAM_MAC(isRemotePort ? GT_OK : GT_BAD_PARAM, st);

            for (iter = 0; iter < 2; iter++)
            {
                ledPerPortConfig.led0Select = goodLed0SelectValues[(2*portNum+iter) % goodLed0SelectValuesSize];
                ledPerPortConfig.led1Select = goodLed1SelectValues[(2*portNum+iter) % goodLed1SelectValuesSize];
                ledPerPortConfig.pulseStretch = (2*portNum+iter) % 5;
                ledPerPortConfig.blinkRate = (2*portNum+iter) % 6;

                st = cpssDxChLedPhyControlPerPortSet(dev, portNum,
                                        &ledPerPortConfig);
                if(isRemotePort == GT_FALSE)
                {
                    UTF_VERIFY_EQUAL6_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                            portNum,
                                            ledPerPortConfig.led0Select,
                                            ledPerPortConfig.led1Select,
                                            ledPerPortConfig.pulseStretch,
                                            ledPerPortConfig.blinkRate);
                }
                else
                {
                    UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev,
                                            portNum,
                                            ledPerPortConfig.led0Select,
                                            ledPerPortConfig.led1Select,
                                            ledPerPortConfig.pulseStretch,
                                            ledPerPortConfig.blinkRate);
                    if (st == GT_OK)
                    {

                        /*
                            1.2. Call cpssDxChLedPhyControlPerPortGet
                            Expected: GT_OK and the same state.
                        */
                        st = cpssDxChLedPhyControlPerPortGet(dev,
                                            portNum, &ledPerPortConfigGet);
                        UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                            "cpssDxChLedPhyControlPerPortGet: %d %d %d %d %d %d",
                                        dev, portNum,
                                            ledPerPortConfig.led0Select,
                                            ledPerPortConfig.led1Select,
                                            ledPerPortConfig.pulseStretch,
                                            ledPerPortConfig.blinkRate);
                        UTF_VERIFY_EQUAL2_STRING_MAC(0,
                            cpssOsMemCmp (&ledPerPortConfig,
                                            &ledPerPortConfigGet,
                                            sizeof(ledPerPortConfig)),
                            "get different data than was set: %d/%d", dev, portNum);
                    }
                }
            }

            /*
                1.3. Call with wrong blinkRate [8].
                Expected: GT_BAD_PARAM
            */
            ledPerPortConfig.blinkRate = 8;
            st = cpssDxChLedPhyControlPerPortSet(dev, portNum,
                                    &ledPerPortConfig);
            UTF_VERIFY_EQUAL6_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                    portNum,
                                    ledPerPortConfig.led0Select,
                                    ledPerPortConfig.led1Select,
                                    ledPerPortConfig.pulseStretch,
                                    ledPerPortConfig.blinkRate);

            st = cpssDxChLedPhyControlPerPortSet(dev, portNum,
                                    &ledPerPortConfigSave);
            UTF_VERIFY_EQUAL0_PARAM_MAC(isRemotePort ? GT_OK : GT_BAD_PARAM, st);
        }

        ledPerPortConfig.blinkRate = 0;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 2.1. Call function for each non-active port */
            st = cpssDxChLedPhyControlPerPortSet(dev,
                                                   portNum,
                                                   &ledPerPortConfig);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 2.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

        st = cpssDxChLedPhyControlPerPortSet(dev,
                                               portNum,
                                               &ledPerPortConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 2.3. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChLedPhyControlPerPortSet(dev,
                                               portNum,
                                               &ledPerPortConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 3. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedPhyControlPerPortSet(dev,
                                portNum, &ledPerPortConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.1 Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLedPhyControlPerPortSet(dev,
                                           portNum,
                                           &ledPerPortConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLedPhyControlPerPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_LED_PHY_PORT_CONF_STC  *ledPerPortConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLedPhyControlPerPortGet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function
    Expected: GT_OK on remote ports, GT_BAD_PARAM on other ports.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM            portNum    = 0;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_DXCH_LED_PHY_PORT_CONF_STC ledPerPortConfigGet;

    /* there is no LED in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChPortInterfaceModeGet: %d, %d", dev, portNum);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            st = cpssDxChLedPhyControlPerPortGet(dev, portNum, &ledPerPortConfigGet);
            if(prvCpssDxChPortRemotePortCheck(dev,portNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }
        }
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLedPhyControlPerPortGet(dev, portNum, &ledPerPortConfigGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChLedPhyControlPerPortGet(dev, portNum, &ledPerPortConfigGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChLedPhyControlPerPortGet(dev, portNum, &ledPerPortConfigGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLedPhyControlPerPortGet(dev, portNum, &ledPerPortConfigGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLedPhyControlPerPortGet(dev, portNum, &ledPerPortConfigGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChHwInitLedCtrl suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChHwInitLedCtrl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamClassAndGroupConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamHyperGStackTxQStatusEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamHyperGStackTxQStatusEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamClassIndicationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamClassIndicationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamCpuOrPort27ModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamCpuOrPort27ModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamDirectModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamDirectModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortGroupClassManipulationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortGroupClassManipulationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortGroupConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortGroupConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortGroupGroupConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortGroupGroupConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamClassManipulationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamClassManipulationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamGroupConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamGroupConfigGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamSyncLedsEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamSyncLedsEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortPositionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortPositionGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortClassPolarityInvertEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortClassPolarityInvertEnableGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortClassIndicationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedStreamPortClassIndicationGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedPhyControlGlobalSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedPhyControlGlobalGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedPhyControlPerPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedPhyControlPerPortGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLedBc3FELedPosition)
UTF_SUIT_END_TESTS_MAC(cpssDxChHwInitLedCtrl)


