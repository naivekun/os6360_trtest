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
* @file cpssDxChOamUT.c
*
* @brief Unit tests for cpssDxChOam, that provides CPSS implementation for
* Operation, Administration and Maintenance (OAM) Features.
*
* @version   17
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files
   that already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#define OAM_TABLE_NUM_ENTRIES_CNS(_dev) ((!PRV_CPSS_SIP_5_20_CHECK_MAC(_dev)) ? _2K : _8K)
/*32 bits in entry*/
#define OAM_AGING_TABLE_NUM_ENTRIES_CNS(_dev) (OAM_TABLE_NUM_ENTRIES_CNS(_dev) / 32)


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamAgingDaemonEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamAgingDaemonEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   enable[GT_FALSE, GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChOamAgingDaemonEnableGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_BOOL                         enable      = GT_FALSE;
    GT_BOOL                         value       = GT_FALSE;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                           enable[GT_FALSE, GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        enable = GT_FALSE;

        st = cpssDxChOamAgingDaemonEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamAgingDaemonEnableGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamAgingDaemonEnableGet(dev, stage, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, value,
                                         "got another enable: %d", value);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        enable = GT_FALSE;

        st = cpssDxChOamAgingDaemonEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamAgingDaemonEnableGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamAgingDaemonEnableGet(dev, stage, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, value,
                                         "got another enable: %d", value);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        enable = GT_TRUE;

        st = cpssDxChOamAgingDaemonEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamAgingDaemonEnableGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamAgingDaemonEnableGet(dev, stage, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, value,
                                         "got another enable: %d", value);
        }

        /* restore value */
        enable = GT_FALSE;

        st = cpssDxChOamAgingDaemonEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamAgingDaemonEnableSet
                            (dev, stage, enable),
                            stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamAgingDaemonEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamAgingDaemonEnableSet(dev, stage, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamAgingDaemonEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamAgingDaemonEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_BOOL                         enable      = GT_FALSE;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                           non-null enablePtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamAgingDaemonEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamAgingDaemonEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamAgingDaemonEnableGet
                            (dev, stage, &enable),
                            stage);

        /* restore value */
        enable = GT_FALSE;

        /*
            1.3. Call with out of range enable and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamAgingDaemonEnableGet(dev, stage, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChOamAgingDaemonEnableGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamAgingDaemonEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamAgingDaemonEnableGet(dev, stage, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamAgingPeriodEntrySet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_OAM_STAGE_TYPE_ENT     stage,
    IN GT_U32                           entryIndex,
    IN GT_U64                           agingPeriodValue
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamAgingPeriodEntrySet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   entryIndex[0, 5, 7],
                   agingPeriodValue[0, 0xFFFF, 0x3FFFFFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChOamAgingPeriodEntryGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range entryIndex [8] and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range agingPeriodValue [0x400000000] and
        other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st                  = GT_OK;
    GT_U8                           dev                 = 0;
    GT_U32                          entryIndex          = 0;
    GT_U64                          agingPeriodValue    = {{0}};
    GT_U64                          tmpValue            = {{0}};
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    GT_BOOL                         isEqual             = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                           entryIndex[0, 5, 7],
                           agingPeriodValue[0, 0xFFFF, 0x3FFFFFFFF].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        entryIndex = 0;
        agingPeriodValue.l[0] = 0;
        agingPeriodValue.l[1] = 0;

        st = cpssDxChOamAgingPeriodEntrySet(dev, stage, entryIndex, agingPeriodValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &agingPeriodValue,
                                         (GT_VOID*) &tmpValue,
                                         sizeof(agingPeriodValue))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                                         "got another agingPeriodValue: %u%u",
                                         tmpValue.l[1],tmpValue.l[0]);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 5;
        agingPeriodValue.l[0] = 0xFFFF;
        agingPeriodValue.l[1] = 0;

        st = cpssDxChOamAgingPeriodEntrySet(dev, stage, entryIndex, agingPeriodValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &agingPeriodValue,
                                         (GT_VOID*) &tmpValue,
                                         sizeof(agingPeriodValue))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                                         "got another agingPeriodValue: %u%u",
                                         tmpValue.l[1], tmpValue.l[0]);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        entryIndex = 7;
        agingPeriodValue.l[0] = 0xFFFFFFFF;
        agingPeriodValue.l[1] = 0x03;

        st = cpssDxChOamAgingPeriodEntrySet(dev, stage, entryIndex, agingPeriodValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamTableBaseFlowIdGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &agingPeriodValue,
                                         (GT_VOID*) &tmpValue,
                                         sizeof(agingPeriodValue))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                                         "got another agingPeriodValue: %u%u",
                                         tmpValue.l[1],tmpValue.l[0]);
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 5;
        agingPeriodValue.l[0] = 0xFFFF;
        agingPeriodValue.l[1] = 0;

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamAgingPeriodEntrySet
                            (dev, stage, entryIndex, agingPeriodValue),
                            stage);

        /*
            1.4. Call with out of range entryIndex and other valid params.
            Expected: NON GT_OK.
        */
        entryIndex = BIT_3;

        st = cpssDxChOamAgingPeriodEntrySet(dev, stage, entryIndex, agingPeriodValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entryIndex = 1;

        /*
            1.5. Call with out of range agingPeriodValue and other valid params.
            Expected: NON GT_OK.
        */
        agingPeriodValue.l[0] = 0x00;
        agingPeriodValue.l[1] = 0x04;

        st = cpssDxChOamAgingPeriodEntrySet(dev, stage, entryIndex, agingPeriodValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
    entryIndex = 5;
    agingPeriodValue.l[0] = 0xFFFF;
    agingPeriodValue.l[1] = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamAgingPeriodEntrySet(dev, stage, entryIndex, agingPeriodValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamAgingPeriodEntrySet(dev, stage, entryIndex, agingPeriodValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamAgingPeriodEntryGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_OAM_STAGE_TYPE_ENT         stage,
    IN GT_U32                               entryIndex,
    OUT GT_U64                             *agingPeriodValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamAgingPeriodEntryGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   entryIndex[0, 3, 7],
                   non-null agingPeriodValuePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range entryIndex[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range agingPeriodValuePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st                  = GT_OK;
    GT_U8                           dev                 = 0;
    GT_U32                          entryIndex          = 0;
    GT_U64                          agingPeriodValue    = {{0}};
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                        entryIndex[0, 3, 7],
                        non-null agingPeriodValuePtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        entryIndex = 0;

        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &agingPeriodValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 3;

        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &agingPeriodValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 7;

        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &agingPeriodValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamAgingPeriodEntryGet
                            (dev, stage, entryIndex, &agingPeriodValue),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 1;

        /*
            1.3. Call with out of range entryIndex[BIT_3] and other valid params.
            Expected: NON GT_OK.
        */
        entryIndex = BIT_3;

        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &agingPeriodValue);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamAgingPeriodEntryGet: %d", dev);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 1;

        /*
            1.4. Call with out of range agingPeriodValuePtr[NULL] and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamAgingPeriodEntryGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    entryIndex = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &agingPeriodValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamAgingPeriodEntryGet(dev, stage, entryIndex, &agingPeriodValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamCpuCodeBaseSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_OAM_STAGE_TYPE_ENT                 stage,
    IN CPSS_NET_RX_CPU_CODE_ENT                     baseCpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamCpuCodeBaseSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
              baseCpuCode[CPSS_NET_CONTROL_E,
                          CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E,
                          CPSS_NET_FIRST_USER_DEFINED_E].
    Expected: GT_OK.
    1.2. Call cpssDxChOamCpuCodeBaseGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range baseCpuCode and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    CPSS_NET_RX_CPU_CODE_ENT        baseCpuCode;
    CPSS_NET_RX_CPU_CODE_ENT        tmpValue;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                      baseCpuCode[CPSS_NET_CONTROL_E,
                                  CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E
                                  CPSS_NET_FIRST_USER_DEFINED_E].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        baseCpuCode = CPSS_NET_CONTROL_E;

        st = cpssDxChOamCpuCodeBaseSet(dev, stage, baseCpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamCpuCodeBaseGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(baseCpuCode, tmpValue,
                                         "got another baseCpuCode: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        baseCpuCode = CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E;

        st = cpssDxChOamCpuCodeBaseSet(dev, stage, baseCpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamCpuCodeBaseGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(baseCpuCode, tmpValue,
                                         "got another baseCpuCode: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        baseCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

        st = cpssDxChOamCpuCodeBaseSet(dev, stage, baseCpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamCpuCodeBaseGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamCpuCodeBaseGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(baseCpuCode, tmpValue,
                                         "got another baseCpuCode: %d", tmpValue);
        }

        /* restore value */
        baseCpuCode = CPSS_NET_CONTROL_E;

        /*
            1.3. Call with out of range stage and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamCpuCodeBaseSet
                            (dev, stage, baseCpuCode),
                            stage);

        /*
            1.4. Call with out of range baseCpuCode and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamCpuCodeBaseSet
                            (dev, stage, baseCpuCode),
                            baseCpuCode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore values */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    baseCpuCode = CPSS_NET_CONTROL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamCpuCodeBaseSet(dev, stage, baseCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamCpuCodeBaseSet(dev, stage, baseCpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamCpuCodeBaseGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_OAM_STAGE_TYPE_ENT                 stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT                    *baseCpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamCpuCodeBaseGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   non-null baseCpuCodePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range baseCpuCodePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    CPSS_NET_RX_CPU_CODE_ENT        baseCpuCode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                           non-null baseCpuCodePtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamCpuCodeBaseGet(dev, stage, &baseCpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamCpuCodeBaseGet(dev, stage, &baseCpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamCpuCodeBaseGet
                            (dev, stage, &baseCpuCode),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        /*
            1.3. Call with out of range baseCpuCodePtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamCpuCodeBaseGet(dev, stage, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamCpuCodeBaseGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamCpuCodeBaseGet(dev, stage, &baseCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamCpuCodeBaseGet(dev, stage, &baseCpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamDualEndedLmPacketCommandSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_OAM_STAGE_TYPE_ENT                 stage,
    IN CPSS_PACKET_CMD_ENT                          command,
    IN GT_U32                                       cpuCodeLsBits
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamDualEndedLmPacketCommandSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   command [CPSS_PACKET_CMD_FORWARD_E,
                            CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                            CPSS_PACKET_CMD_DROP_SOFT_E],
                   cpuCodeLsBits[ 0, 1, 3 ].
    Expected: GT_OK.
    1.2. Call cpssDxChOamDualEndedLmPacketCommandGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range command and other valid params.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range cpuCodeLsBits[BIT_2] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_U32                          cpuCodeLsBits   = 0;
    GT_U32                          tmpValue        = 0;
    CPSS_PACKET_CMD_ENT             command;
    CPSS_PACKET_CMD_ENT             tmpCommand;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                           command[CPSS_PACKET_CMD_FORWARD_E,
                                    CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                    CPSS_PACKET_CMD_DROP_SOFT_E],
                           cpuCodeLsBits[0, 1, 3].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        command = CPSS_PACKET_CMD_FORWARD_E;
        cpuCodeLsBits = 0;

        st = cpssDxChOamDualEndedLmPacketCommandSet(dev, stage, command, cpuCodeLsBits);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &tmpCommand, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeLsBits, tmpValue,
                                         "got another cpuCodeLsBits: %d",
                                         tmpValue);

            UTF_VERIFY_EQUAL1_STRING_MAC(command, tmpCommand,
                                         "got another command: %d",
                                         tmpCommand);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        cpuCodeLsBits = 1;

        st = cpssDxChOamDualEndedLmPacketCommandSet(dev, stage, command, cpuCodeLsBits);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &tmpCommand, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeLsBits, tmpValue,
                                         "got another cpuCodeLsBits: %d",
                                         tmpValue);

            UTF_VERIFY_EQUAL1_STRING_MAC(command, tmpCommand,
                                         "got another command: %d",
                                         tmpCommand);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        command = CPSS_PACKET_CMD_DROP_SOFT_E;
        cpuCodeLsBits = 3;

        st = cpssDxChOamDualEndedLmPacketCommandSet(dev, stage, command, cpuCodeLsBits);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamTableBaseFlowIdGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &tmpCommand, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeLsBits, tmpValue,
                                         "got another cpuCodeLsBits: %d",
                                         tmpValue);

            UTF_VERIFY_EQUAL1_STRING_MAC(command, tmpCommand,
                                         "got another command: %d",
                                         tmpCommand);
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        cpuCodeLsBits = 1;

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamDualEndedLmPacketCommandSet
                            (dev, stage, command, cpuCodeLsBits),
                            stage);

        /*
            1.4. Call with out of range command and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamDualEndedLmPacketCommandSet
                            (dev, stage, command, cpuCodeLsBits),
                            command);

        /*
            1.5. Call with out of range cpuCodeLsBits[BIT_2] and other valid params.
            Expected: NON GT_OK.
        */
        cpuCodeLsBits = BIT_2;

        st = cpssDxChOamDualEndedLmPacketCommandSet(dev, stage, command, cpuCodeLsBits);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
    command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    cpuCodeLsBits = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamDualEndedLmPacketCommandSet(dev, stage, command, cpuCodeLsBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamDualEndedLmPacketCommandSet(dev, stage, command, cpuCodeLsBits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamDualEndedLmPacketCommandGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT                stage,
    OUT CPSS_PACKET_CMD_ENT                         *commandPtr,
    OUT GT_U32                                      *cpuCodeLsBitsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamDualEndedLmPacketCommandGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage[CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   non-null commandPtr,
                   non-null cpuCodeLsBitsPtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range commandPtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range cpuCodeLsBitsPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_U32                          cpuCodeLsBits   = 0;
    CPSS_PACKET_CMD_ENT             command;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                           non-null commandPtr,
                           non-null cpuCodeLsBitsPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &command, &cpuCodeLsBits);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &command, &cpuCodeLsBits);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &command, &cpuCodeLsBits);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamDualEndedLmPacketCommandGet
                            (dev, stage, &command, &cpuCodeLsBits),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        /*
            1.3. Call with out of range commandPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, NULL, &cpuCodeLsBits);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamDualEndedLmPacketCommandGet: %d", dev);

        /*
            1.4. Call with out of range cpuCodeLsBitsPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &command, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamDualEndedLmPacketCommandGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &command, &cpuCodeLsBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamDualEndedLmPacketCommandGet(dev, stage, &command, &cpuCodeLsBits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamEntrySet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_OAM_ENTRY_STC            *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamEntrySet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   entryIndex[0, 0xFF, 0x7FF],
                   entry{
                        opcodeParsingEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        megLevelCheckEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        megLevel[0, 3, 7],
                        packetCommandProfile[0, 3, 7],
                        cpuCodeOffset[0, 1, 3],
                        sourceInterfaceCheckEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        sourceInterfaceCheckMode[CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E,
                                                 CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E,
                                                 CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E],
                        sourceInterface{type [CPSS_INTERFACE_PORT_E,
                                              CPSS_INTERFACE_TRUNK_E,
                                              CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum  [devNum],
                                                 portNum [0, max/2, max]},
                                        trunkId [0, max/2, max]},
                        lmCounterCaptureEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        dualEndedLmEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        lmCountingEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        oamPtpOffsetIndex[0, 11, 15],
                        timestampEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        keepaliveAgingEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        ageState[0, 7, 15],
                        agingPeriodIndex[0, 3, 7],
                        agingThreshold[0, 11, 15],
                        hashVerifyEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        lockHashValueEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        protectionLocUpdateEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        flowHash[0, 1024, 4095],
                        excessKeepaliveDetectionEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        excessKeepalivePeriodCounter[0, 3, 15],
                        excessKeepalivePeriodThreshold[0, 7, 15],
                        excessKeepaliveMessageCounter[0, 3, 7],
                        excessKeepaliveMessageThreshold[0, 5, 7],
                        rdiCheckEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        rdiStatus[0, 1, 1],
                        periodCheckEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        keepaliveTxPeriod[0, 5, 7]}.
    Expected: GT_OK.
    1.2. Call cpssDxChOamAgingPeriodEntryGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range entryIndex[BIT_15] and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range entry.megLevel[BIT_3]
    Expected: NON GT_OK.
    1.6. Call with out of range entry.packetCommandProfile[BIT_3]
        and other valid params.
    Expected: NON GT_OK.
    1.7. Call with out of range entry.cpuCodeOffset[BIT_2] and other valid params.
    Expected: NON GT_OK.
    1.8. Call with out of range entry.sourceInterfaceCheckMode and other valid params.
    Expected: GT_BAD_PARAM.
    1.9. Call with entry.sourceInterface.type [CPSS_INTERFACE_TRUNK_E],
         out of range entry.sourceInterface.devPort.portNum (not relevant) and
         others valid params.
    Expected: GT_OK.
    1.10. Call with out of range entry.sourceInterface.trunkId and others valid params.
    Expected: NOT GT_OK.
    1.11. Call with entry.sourceInterface.type [CPSS_INTERFACE_PORT_E],
          out of range entry.sourceInterface.trunkId (not relevant) and
          others valid params.
    Expected: GT_OK.
    1.12. Call with out of range entry.sourceInterface.devPort.hwDevNum and
          others valid params.
    Expected: NOT GT_OK.
    1.13. Call with out of range entry.sourceInterface.devPort.portNum and
          others valid params.
    Expected: NOT GT_OK.
    1.14. Call with wrong entry.sourceInterface.type enum values and
          other valid params.
    Expected: GT_BAD_PARAM.
    1.15. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E] and
          out of range entry.oamPtpOffsetIndex (relevant) and other valid params.
    Expected: NON GT_OK.
    1.16. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E] and
          out of range entry.oamPtpOffsetIndex (not relevant) and other valid params.
    Expected: GT_OK.
    1.17. Call with out of range entry.ageState[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.18. Call with out of range entry.agingPeriodIndex[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.19. Call with out of range entry.agingThreshold[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.20. Call with out of range entry.flowHash[BIT_12] and other valid params.
    Expected: NON GT_OK.
    1.21. Call with out of range entry.excessKeepalivePeriodCounter[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.22. Call with out of range entry.excessKeepalivePeriodThreshold[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.23. Call with out of range entry.excessKeepaliveMessageCounter[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.24. Call with out of range entry.excessKeepaliveMessageThreshold[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.25. Call with out of range entry.rdiStatus[BIT_1] and other valid params.
    Expected: NON GT_OK.
    1.26. Call with out of range entry.keepaliveTxPeriod[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.27. Call with out of range entryPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                       st                  = GT_OK;
    GT_U8                           dev                 = 0;
    GT_U32                          entryIndex          = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage               = 0;
    CPSS_DXCH_OAM_ENTRY_STC         entry               = {0};
    CPSS_DXCH_OAM_ENTRY_STC         tmpEntry            = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   entryIndex[0, 0xFF, 0x7FF],
                   entry{
                        opcodeParsingEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        megLevelCheckEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        megLevel[0, 3, 7],
                        packetCommandProfile[0, 3, 7],
                        cpuCodeOffset[0, 1, 3],
                        sourceInterfaceCheckEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        sourceInterfaceCheckMode[CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E,
                                                 CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E,
                                                 CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E],
                        sourceInterface{type [CPSS_INTERFACE_PORT_E,
                                              CPSS_INTERFACE_TRUNK_E,
                                              CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum  [devNum],
                                                 portNum [0, max/2, max]},
                                        trunkId [1, max/2, max]},
                        lmCounterCaptureEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        dualEndedLmEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        lmCountingEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        oamPtpOffsetIndex[0, 11, 15],
                        timestampEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        keepaliveAgingEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        ageState[0, 7, 15],
                        agingPeriodIndex[0, 3, 7],
                        agingThreshold[0, 11, 15],
                        hashVerifyEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        lockHashValueEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        protectionLocUpdateEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        flowHash[0, 1024, 4095],
                        excessKeepaliveDetectionEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        excessKeepalivePeriodCounter[0, 3, 15],
                        excessKeepalivePeriodThreshold[0, 7, 15],
                        excessKeepaliveMessageCounter[0, 3, 7],
                        excessKeepaliveMessageThreshold[0, 5, 7],
                        rdiCheckEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        rdiStatus[0, 1, 1],
                        periodCheckEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        keepaliveTxPeriod[0, 5, 7]}.
            Expected: GT_OK.

        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        entryIndex = 0;

        entry.opcodeParsingEnable = GT_FALSE;
        entry.megLevelCheckEnable = GT_FALSE;
        entry.megLevel = 0;
        entry.packetCommandProfile = 0;
        entry.cpuCodeOffset = 0;
        entry.sourceInterfaceCheckEnable = GT_TRUE;
        entry.sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E;
        entry.sourceInterface.type = CPSS_INTERFACE_PORT_E;
        entry.sourceInterface.devPort.hwDevNum = dev;
        entry.sourceInterface.devPort.portNum  = 0;
        entry.lmCounterCaptureEnable = GT_FALSE;
        entry.dualEndedLmEnable = GT_FALSE;
        entry.lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_ENABLE_E;
        entry.oamPtpOffsetIndex = 0;
        entry.timestampEnable = GT_TRUE;
        entry.keepaliveAgingEnable = GT_FALSE;
        entry.ageState = 0;
        entry.agingPeriodIndex = 0;
        entry.agingThreshold = 0;
        entry.hashVerifyEnable = GT_TRUE;
        entry.lockHashValueEnable = GT_FALSE;
        entry.protectionLocUpdateEnable = GT_FALSE;
        entry.flowHash = 0;
        entry.excessKeepaliveDetectionEnable = GT_TRUE;
        entry.excessKeepalivePeriodCounter = 0;
        entry.excessKeepalivePeriodThreshold = 0;
        entry.excessKeepaliveMessageCounter = 0;
        entry.excessKeepaliveMessageThreshold = 0;
        entry.rdiCheckEnable = GT_FALSE;
        entry.rdiStatus = 0;
        entry.periodCheckEnable = GT_FALSE;
        entry.keepaliveTxPeriod = 0;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &tmpEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.opcodeParsingEnable,
                                         tmpEntry.opcodeParsingEnable,
                                         "got another opcodeParsingEnable: %d",
                                         tmpEntry.opcodeParsingEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.megLevelCheckEnable,
                                         tmpEntry.megLevelCheckEnable,
                                         "got another megLevelCheckEnable: %d",
                                         tmpEntry.megLevelCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.megLevel,
                                         tmpEntry.megLevel,
                                         "got another megLevel: %d",
                                         tmpEntry.megLevel);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.packetCommandProfile,
                                         tmpEntry.packetCommandProfile,
                                         "got another packetCommandProfile: %d",
                                         tmpEntry.packetCommandProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.cpuCodeOffset,
                                         tmpEntry.cpuCodeOffset,
                                         "got another cpuCodeOffset: %d",
                                         tmpEntry.cpuCodeOffset);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterfaceCheckEnable,
                                         tmpEntry.sourceInterfaceCheckEnable,
                                         "got another sourceInterfaceCheckEnable: %d",
                                         tmpEntry.sourceInterfaceCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterfaceCheckMode,
                                         tmpEntry.sourceInterfaceCheckMode,
                                         "got another sourceInterfaceCheckMode: %d",
                                         tmpEntry.sourceInterfaceCheckMode);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.type,
                                         tmpEntry.sourceInterface.type,
                                         "got another sourceInterface.type: %d",
                                         tmpEntry.sourceInterface.type);
            UTF_VERIFY_EQUAL1_STRING_MAC(0,/* there is no such parameter in the HW ... cpss just 'memset' it to 0 */
                                         tmpEntry.sourceInterface.devPort.hwDevNum,
                                         "(should be always 0) got another sourceInterface.devPort.hwDevNum: %d",
                                         tmpEntry.sourceInterface.devPort.hwDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.devPort.portNum,
                                         tmpEntry.sourceInterface.devPort.portNum,
                                         "got another sourceInterface.devPort.portNum: %d",
                                         tmpEntry.sourceInterface.devPort.portNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lmCounterCaptureEnable,
                                         tmpEntry.lmCounterCaptureEnable,
                                         "got another lmCounterCaptureEnable: %d",
                                         tmpEntry.lmCounterCaptureEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.dualEndedLmEnable,
                                         tmpEntry.dualEndedLmEnable,
                                         "got another dualEndedLmEnable: %d",
                                         tmpEntry.dualEndedLmEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lmCountingMode,
                                         tmpEntry.lmCountingMode,
                                         "got another lmCountingMode: %d",
                                         tmpEntry.lmCountingMode);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.oamPtpOffsetIndex,
                                         tmpEntry.oamPtpOffsetIndex,
                                         "got another oamPtpOffsetIndex: %d",
                                         tmpEntry.oamPtpOffsetIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.timestampEnable,
                                         tmpEntry.timestampEnable,
                                         "got another timestampEnable: %d",
                                         tmpEntry.timestampEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.keepaliveAgingEnable,
                                         tmpEntry.keepaliveAgingEnable,
                                         "got another keepaliveAgingEnable: %d",
                                         tmpEntry.keepaliveAgingEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.ageState,
                                         tmpEntry.ageState,
                                         "got another ageState: %d",
                                         tmpEntry.ageState);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.agingPeriodIndex,
                                         tmpEntry.agingPeriodIndex,
                                         "got another agingPeriodIndex: %d",
                                         tmpEntry.agingPeriodIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.agingThreshold,
                                         tmpEntry.agingThreshold,
                                         "got another agingThreshold: %d",
                                         tmpEntry.agingThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.hashVerifyEnable,
                                         tmpEntry.hashVerifyEnable,
                                         "got another hashVerifyEnable: %d",
                                         tmpEntry.hashVerifyEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lockHashValueEnable,
                                         tmpEntry.lockHashValueEnable,
                                         "got another lockHashValueEnable: %d",
                                         tmpEntry.lockHashValueEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.protectionLocUpdateEnable,
                                         tmpEntry.protectionLocUpdateEnable,
                                         "got another protectionLocUpdateEnable: %d",
                                         tmpEntry.protectionLocUpdateEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.flowHash,
                                         tmpEntry.flowHash,
                                         "got another flowHash: %d",
                                         tmpEntry.flowHash);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveDetectionEnable,
                                         tmpEntry.excessKeepaliveDetectionEnable,
                                         "got another excessKeepaliveDetectionEnable: %d",
                                         tmpEntry.excessKeepaliveDetectionEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepalivePeriodCounter,
                                         tmpEntry.excessKeepalivePeriodCounter,
                                         "got another excessKeepalivePeriodCounter: %d",
                                         tmpEntry.excessKeepalivePeriodCounter);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepalivePeriodThreshold,
                                         tmpEntry.excessKeepalivePeriodThreshold,
                                         "got another excessKeepalivePeriodThreshold: %d",
                                         tmpEntry.excessKeepalivePeriodThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveMessageCounter,
                                         tmpEntry.excessKeepaliveMessageCounter,
                                         "got another excessKeepaliveMessageCounter: %d",
                                         tmpEntry.excessKeepaliveMessageCounter);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveMessageThreshold,
                                         tmpEntry.excessKeepaliveMessageThreshold,
                                         "got another excessKeepaliveMessageThreshold: %d",
                                         tmpEntry.excessKeepaliveMessageThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.rdiCheckEnable,
                                         tmpEntry.rdiCheckEnable,
                                         "got another rdiCheckEnable: %d",
                                         tmpEntry.rdiCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.rdiStatus,
                                         tmpEntry.rdiStatus,
                                         "got another rdiStatus: %d",
                                         tmpEntry.rdiStatus);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.periodCheckEnable,
                                         tmpEntry.periodCheckEnable,
                                         "got another periodCheckEnable: %d",
                                         tmpEntry.periodCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.keepaliveTxPeriod,
                                         tmpEntry.keepaliveTxPeriod,
                                         "got another keepaliveTxPeriod: %d",
                                         tmpEntry.keepaliveTxPeriod);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 0xFF;

        entry.opcodeParsingEnable = GT_TRUE;
        entry.megLevelCheckEnable = GT_TRUE;
        entry.megLevel = 3;
        entry.packetCommandProfile = 3;
        entry.cpuCodeOffset = 1;
        entry.sourceInterfaceCheckEnable = GT_FALSE;
        entry.sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
        entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

        entry.lmCounterCaptureEnable = GT_TRUE;
        entry.dualEndedLmEnable = GT_TRUE;
        entry.lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_DISABLE_E;
        entry.oamPtpOffsetIndex = 57;
        entry.timestampEnable = GT_FALSE;
        entry.keepaliveAgingEnable = GT_TRUE;
        entry.ageState = 7;
        entry.agingPeriodIndex = 3;
        entry.agingThreshold = 11;
        entry.hashVerifyEnable = GT_FALSE;
        entry.lockHashValueEnable = GT_TRUE;
        entry.protectionLocUpdateEnable = GT_TRUE;
        entry.flowHash = 1024;
        entry.excessKeepaliveDetectionEnable = GT_FALSE;
        entry.excessKeepalivePeriodCounter = 3;
        entry.excessKeepalivePeriodThreshold = 7;
        entry.excessKeepaliveMessageCounter = 3;
        entry.excessKeepaliveMessageThreshold = 5;
        entry.rdiCheckEnable = GT_TRUE;
        entry.rdiStatus = 1;
        entry.periodCheckEnable = GT_TRUE;
        entry.keepaliveTxPeriod = 5;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &tmpEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.opcodeParsingEnable,
                                         tmpEntry.opcodeParsingEnable,
                                         "got another opcodeParsingEnable: %d",
                                         tmpEntry.opcodeParsingEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.megLevelCheckEnable,
                                         tmpEntry.megLevelCheckEnable,
                                         "got another megLevelCheckEnable: %d",
                                         tmpEntry.megLevelCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.megLevel,
                                         tmpEntry.megLevel,
                                         "got another megLevel: %d",
                                         tmpEntry.megLevel);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.packetCommandProfile,
                                         tmpEntry.packetCommandProfile,
                                         "got another packetCommandProfile: %d",
                                         tmpEntry.packetCommandProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.cpuCodeOffset,
                                         tmpEntry.cpuCodeOffset,
                                         "got another cpuCodeOffset: %d",
                                         tmpEntry.cpuCodeOffset);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterfaceCheckEnable,
                                         tmpEntry.sourceInterfaceCheckEnable,
                                         "got another sourceInterfaceCheckEnable: %d",
                                         tmpEntry.sourceInterfaceCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterfaceCheckMode,
                                         tmpEntry.sourceInterfaceCheckMode,
                                         "got another sourceInterfaceCheckMode: %d",
                                         tmpEntry.sourceInterfaceCheckMode);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.type,
                                         tmpEntry.sourceInterface.type,
                                         "got another sourceInterface.type: %d",
                                         tmpEntry.sourceInterface.type);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.trunkId,
                                         tmpEntry.sourceInterface.trunkId,
                                         "got another sourceInterface.trunkId: %d",
                                         tmpEntry.sourceInterface.trunkId);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lmCounterCaptureEnable,
                                         tmpEntry.lmCounterCaptureEnable,
                                         "got another lmCounterCaptureEnable: %d",
                                         tmpEntry.lmCounterCaptureEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.dualEndedLmEnable,
                                         tmpEntry.dualEndedLmEnable,
                                         "got another dualEndedLmEnable: %d",
                                         tmpEntry.dualEndedLmEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lmCountingMode,
                                         tmpEntry.lmCountingMode,
                                         "got another lmCountingMode: %d",
                                         tmpEntry.lmCountingMode);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.oamPtpOffsetIndex,
                                         tmpEntry.oamPtpOffsetIndex,
                                         "got another oamPtpOffsetIndex: %d",
                                         tmpEntry.oamPtpOffsetIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.timestampEnable,
                                         tmpEntry.timestampEnable,
                                         "got another timestampEnable: %d",
                                         tmpEntry.timestampEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.keepaliveAgingEnable,
                                         tmpEntry.keepaliveAgingEnable,
                                         "got another keepaliveAgingEnable: %d",
                                         tmpEntry.keepaliveAgingEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.ageState,
                                         tmpEntry.ageState,
                                         "got another ageState: %d",
                                         tmpEntry.ageState);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.agingPeriodIndex,
                                         tmpEntry.agingPeriodIndex,
                                         "got another agingPeriodIndex: %d",
                                         tmpEntry.agingPeriodIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.agingThreshold,
                                         tmpEntry.agingThreshold,
                                         "got another agingThreshold: %d",
                                         tmpEntry.agingThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.hashVerifyEnable,
                                         tmpEntry.hashVerifyEnable,
                                         "got another hashVerifyEnable: %d",
                                         tmpEntry.hashVerifyEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lockHashValueEnable,
                                         tmpEntry.lockHashValueEnable,
                                         "got another lockHashValueEnable: %d",
                                         tmpEntry.lockHashValueEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.protectionLocUpdateEnable,
                                         tmpEntry.protectionLocUpdateEnable,
                                         "got another protectionLocUpdateEnable: %d",
                                         tmpEntry.protectionLocUpdateEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.flowHash,
                                         tmpEntry.flowHash,
                                         "got another flowHash: %d",
                                         tmpEntry.flowHash);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveDetectionEnable,
                                         tmpEntry.excessKeepaliveDetectionEnable,
                                         "got another excessKeepaliveDetectionEnable: %d",
                                         tmpEntry.excessKeepaliveDetectionEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepalivePeriodCounter,
                                         tmpEntry.excessKeepalivePeriodCounter,
                                         "got another excessKeepalivePeriodCounter: %d",
                                         tmpEntry.excessKeepalivePeriodCounter);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepalivePeriodThreshold,
                                         tmpEntry.excessKeepalivePeriodThreshold,
                                         "got another excessKeepalivePeriodThreshold: %d",
                                         tmpEntry.excessKeepalivePeriodThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveMessageCounter,
                                         tmpEntry.excessKeepaliveMessageCounter,
                                         "got another excessKeepaliveMessageCounter: %d",
                                         tmpEntry.excessKeepaliveMessageCounter);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveMessageThreshold,
                                         tmpEntry.excessKeepaliveMessageThreshold,
                                         "got another excessKeepaliveMessageThreshold: %d",
                                         tmpEntry.excessKeepaliveMessageThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.rdiCheckEnable,
                                         tmpEntry.rdiCheckEnable,
                                         "got another rdiCheckEnable: %d",
                                         tmpEntry.rdiCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.rdiStatus,
                                         tmpEntry.rdiStatus,
                                         "got another rdiStatus: %d",
                                         tmpEntry.rdiStatus);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.periodCheckEnable,
                                         tmpEntry.periodCheckEnable,
                                         "got another periodCheckEnable: %d",
                                         tmpEntry.periodCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.keepaliveTxPeriod,
                                         tmpEntry.keepaliveTxPeriod,
                                         "got another keepaliveTxPeriod: %d",
                                         tmpEntry.keepaliveTxPeriod);
        }


        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        entryIndex = 0x7FF;

        entry.opcodeParsingEnable = GT_TRUE;
        entry.megLevelCheckEnable = GT_TRUE;
        entry.megLevel = 7;
        entry.packetCommandProfile = 7;
        entry.cpuCodeOffset = 3;
        entry.sourceInterfaceCheckEnable = GT_TRUE;
        entry.sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
        entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) - 1);

        entry.lmCounterCaptureEnable = GT_FALSE;
        entry.dualEndedLmEnable = GT_TRUE;
        entry.lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_ENABLE_E;
        entry.oamPtpOffsetIndex = 127;
        entry.timestampEnable = GT_TRUE;
        entry.keepaliveAgingEnable = GT_TRUE;
        entry.ageState = 15;
        entry.agingPeriodIndex = 7;
        entry.agingThreshold = 15;
        entry.hashVerifyEnable = GT_TRUE;
        entry.lockHashValueEnable = GT_TRUE;
        entry.protectionLocUpdateEnable = GT_FALSE;
        entry.flowHash = 4095;
        entry.excessKeepaliveDetectionEnable = GT_TRUE;
        entry.excessKeepalivePeriodCounter = 15;
        entry.excessKeepalivePeriodThreshold = 15;
        entry.excessKeepaliveMessageCounter = 7;
        entry.excessKeepaliveMessageThreshold = 7;
        entry.rdiCheckEnable = GT_TRUE;
        entry.rdiStatus = 1;
        entry.periodCheckEnable = GT_FALSE;
        entry.keepaliveTxPeriod = 7;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamTableBaseFlowIdGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &tmpEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.opcodeParsingEnable,
                                         tmpEntry.opcodeParsingEnable,
                                         "got another opcodeParsingEnable: %d",
                                         tmpEntry.opcodeParsingEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.megLevelCheckEnable,
                                         tmpEntry.megLevelCheckEnable,
                                         "got another megLevelCheckEnable: %d",
                                         tmpEntry.megLevelCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.megLevel,
                                         tmpEntry.megLevel,
                                         "got another megLevel: %d",
                                         tmpEntry.megLevel);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.packetCommandProfile,
                                         tmpEntry.packetCommandProfile,
                                         "got another packetCommandProfile: %d",
                                         tmpEntry.packetCommandProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.cpuCodeOffset,
                                         tmpEntry.cpuCodeOffset,
                                         "got another cpuCodeOffset: %d",
                                         tmpEntry.cpuCodeOffset);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterfaceCheckEnable,
                                         tmpEntry.sourceInterfaceCheckEnable,
                                         "got another sourceInterfaceCheckEnable: %d",
                                         tmpEntry.sourceInterfaceCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterfaceCheckMode,
                                         tmpEntry.sourceInterfaceCheckMode,
                                         "got another sourceInterfaceCheckMode: %d",
                                         tmpEntry.sourceInterfaceCheckMode);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.type,
                                         tmpEntry.sourceInterface.type,
                                         "got another sourceInterface.type: %d",
                                         tmpEntry.sourceInterface.type);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.trunkId,
                                         tmpEntry.sourceInterface.trunkId,
                                         "got another sourceInterface.trunkId: %d",
                                         tmpEntry.sourceInterface.trunkId);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lmCounterCaptureEnable,
                                         tmpEntry.lmCounterCaptureEnable,
                                         "got another lmCounterCaptureEnable: %d",
                                         tmpEntry.lmCounterCaptureEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.dualEndedLmEnable,
                                         tmpEntry.dualEndedLmEnable,
                                         "got another dualEndedLmEnable: %d",
                                         tmpEntry.dualEndedLmEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lmCountingMode,
                                         tmpEntry.lmCountingMode,
                                         "got another lmCountingMode: %d",
                                         tmpEntry.lmCountingMode);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.oamPtpOffsetIndex,
                                         tmpEntry.oamPtpOffsetIndex,
                                         "got another oamPtpOffsetIndex: %d",
                                         tmpEntry.oamPtpOffsetIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.timestampEnable,
                                         tmpEntry.timestampEnable,
                                         "got another timestampEnable: %d",
                                         tmpEntry.timestampEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.keepaliveAgingEnable,
                                         tmpEntry.keepaliveAgingEnable,
                                         "got another keepaliveAgingEnable: %d",
                                         tmpEntry.keepaliveAgingEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.ageState,
                                         tmpEntry.ageState,
                                         "got another ageState: %d",
                                         tmpEntry.ageState);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.agingPeriodIndex,
                                         tmpEntry.agingPeriodIndex,
                                         "got another agingPeriodIndex: %d",
                                         tmpEntry.agingPeriodIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.agingThreshold,
                                         tmpEntry.agingThreshold,
                                         "got another agingThreshold: %d",
                                         tmpEntry.agingThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.hashVerifyEnable,
                                         tmpEntry.hashVerifyEnable,
                                         "got another hashVerifyEnable: %d",
                                         tmpEntry.hashVerifyEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.lockHashValueEnable,
                                         tmpEntry.lockHashValueEnable,
                                         "got another lockHashValueEnable: %d",
                                         tmpEntry.lockHashValueEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.protectionLocUpdateEnable,
                                         tmpEntry.protectionLocUpdateEnable,
                                         "got another protectionLocUpdateEnable: %d",
                                         tmpEntry.protectionLocUpdateEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.flowHash,
                                         tmpEntry.flowHash,
                                         "got another flowHash: %d",
                                         tmpEntry.flowHash);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveDetectionEnable,
                                         tmpEntry.excessKeepaliveDetectionEnable,
                                         "got another excessKeepaliveDetectionEnable: %d",
                                         tmpEntry.excessKeepaliveDetectionEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepalivePeriodCounter,
                                         tmpEntry.excessKeepalivePeriodCounter,
                                         "got another excessKeepalivePeriodCounter: %d",
                                         tmpEntry.excessKeepalivePeriodCounter);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepalivePeriodThreshold,
                                         tmpEntry.excessKeepalivePeriodThreshold,
                                         "got another excessKeepalivePeriodThreshold: %d",
                                         tmpEntry.excessKeepalivePeriodThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveMessageCounter,
                                         tmpEntry.excessKeepaliveMessageCounter,
                                         "got another excessKeepaliveMessageCounter: %d",
                                         tmpEntry.excessKeepaliveMessageCounter);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.excessKeepaliveMessageThreshold,
                                         tmpEntry.excessKeepaliveMessageThreshold,
                                         "got another excessKeepaliveMessageThreshold: %d",
                                         tmpEntry.excessKeepaliveMessageThreshold);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.rdiCheckEnable,
                                         tmpEntry.rdiCheckEnable,
                                         "got another rdiCheckEnable: %d",
                                         tmpEntry.rdiCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.rdiStatus,
                                         tmpEntry.rdiStatus,
                                         "got another rdiStatus: %d",
                                         tmpEntry.rdiStatus);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.periodCheckEnable,
                                         tmpEntry.periodCheckEnable,
                                         "got another periodCheckEnable: %d",
                                         tmpEntry.periodCheckEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.keepaliveTxPeriod,
                                         tmpEntry.keepaliveTxPeriod,
                                         "got another keepaliveTxPeriod: %d",
                                         tmpEntry.keepaliveTxPeriod);
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 0xFF;

        entry.opcodeParsingEnable = GT_TRUE;
        entry.megLevelCheckEnable = GT_TRUE;
        entry.megLevel = 3;
        entry.packetCommandProfile = 3;
        entry.cpuCodeOffset = 1;
        entry.sourceInterfaceCheckEnable = GT_FALSE;
        entry.sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
        entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

        entry.lmCounterCaptureEnable = GT_TRUE;
        entry.dualEndedLmEnable = GT_TRUE;
        entry.lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_DISABLE_E;
        entry.oamPtpOffsetIndex = 27;
        entry.timestampEnable = GT_FALSE;
        entry.keepaliveAgingEnable = GT_TRUE;
        entry.ageState = 7;
        entry.agingPeriodIndex = 3;
        entry.agingThreshold = 11;
        entry.hashVerifyEnable = GT_FALSE;
        entry.lockHashValueEnable = GT_TRUE;
        entry.protectionLocUpdateEnable = GT_TRUE;
        entry.flowHash = 1024;
        entry.excessKeepaliveDetectionEnable = GT_FALSE;
        entry.excessKeepalivePeriodCounter = 3;
        entry.excessKeepalivePeriodThreshold = 7;
        entry.excessKeepaliveMessageCounter = 3;
        entry.excessKeepaliveMessageThreshold = 5;
        entry.rdiCheckEnable = GT_TRUE;
        entry.rdiStatus = 1;
        entry.periodCheckEnable = GT_TRUE;
        entry.keepaliveTxPeriod = 5;

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamEntrySet
                            (dev, stage, entryIndex, &entry),
                            stage);

        /*
            1.4. Call with last entryIndex and other valid params.
            Expected: GT_OK.
        */
        entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev) - 1;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.4. Call with out of range entryIndex and other valid params.
            Expected: NON GT_OK.
        */
        entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev);

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entryIndex = 0xFF;

        /*
            1.5. Call with out of range entry.megLevel
            Expected: NON GT_OK.
        */
        entry.megLevel = BIT_3;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.megLevel = 3;

        /*
            1.6. Call with out of range entry.packetCommandProfile and other
             valid params.
            Expected: NON GT_OK.
        */
        entry.packetCommandProfile = BIT_3;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.packetCommandProfile = 3;

        /*
            1.7. Call with out of range entry.cpuCodeOffset and other valid
             params.
            Expected: NON GT_OK.
        */
        entry.cpuCodeOffset = BIT_2;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.cpuCodeOffset = 1;

        /*
            1.8. Call with out of range entry.sourceInterfaceCheckMode and other
             valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamEntrySet
                            (dev, stage, entryIndex, &entry),
                            entry.sourceInterfaceCheckMode);

        /*
            1.9. Call with entry.sourceInterface.type [CPSS_INTERFACE_TRUNK_E],
             out of range entry.sourceInterface.devPort.portNum (not relevant)
             and others valid params.
            Expected: GT_OK.
        */
        entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
        entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) / 2;

        /*
            1.10. Call with out of range entry.sourceInterface.trunkId and others
             valid params.
            Expected: NOT GT_OK.
        */
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev));

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

        /*
            1.11. Call with entry.sourceInterface.type [CPSS_INTERFACE_PORT_E],
             out of range entry.sourceInterface.trunkId (not relevant) and
             others valid params.
            Expected: GT_OK.
        */
        entry.sourceInterface.type = CPSS_INTERFACE_PORT_E;
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev));

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

        /*
            1.12. Call with out of range entry.sourceInterface.devPort.hwDevNum and
             others valid params.
            Expected: NOT GT_OK.
        */
        entry.sourceInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.sourceInterface.devPort.hwDevNum = dev;

        /*
            1.13. Call with out of range entry.sourceInterface.devPort.portNum
             and others valid params.
            Expected: NOT GT_OK.
        */
        entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) / 2;

        /*
            1.14. Call with wrong entry.sourceInterface.type enum values and
             other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamEntrySet
                            (dev, stage, entryIndex, &entry),
                            entry.sourceInterface.type);

        /* restore values*/
        entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
        entry.sourceInterface.devPort.hwDevNum = dev;

        entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) / 2;
        entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

        /*
            1.15. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E] and
                  out of range entry.oamPtpOffsetIndex (relevant) and other valid params.
            Expected: NON GT_OK.
        */
        entry.oamPtpOffsetIndex = BIT_7;
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.16. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E] and
                  out of range entry.oamPtpOffsetIndex (relevant) and other valid params.
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entry.oamPtpOffsetIndex = 11;

        /*
            1.17. Call with out of range entry.ageState and other valid params.
            Expected: NON GT_OK.
        */
        entry.ageState = BIT_4;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.ageState = 7;

        /*
            1.18. Call with out of range entry.agingPeriodIndex and other valid
            params.
            Expected: NON GT_OK.
        */
        entry.agingPeriodIndex = BIT_3;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.agingPeriodIndex = 3;

        /*
            1.19. Call with out of range entry.agingThreshold and other valid
            params.
            Expected: NON GT_OK.
        */
        entry.agingThreshold = BIT_4;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.agingThreshold = 11;

        /*
            1.20. Call with out of range entry.flowHash and other valid params.
            Expected: NON GT_OK.
        */
        entry.flowHash = BIT_12;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.flowHash = 1024;

        /*
            1.21. Call with out of range entry.excessKeepalivePeriodCounter and
            other valid params.
            Expected: NON GT_OK.
        */
        entry.excessKeepalivePeriodCounter = BIT_4;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.excessKeepalivePeriodCounter = 3;

        /*
            1.22. Call with out of range entry.excessKeepalivePeriodThreshold
            and other valid params.
            Expected: NON GT_OK.
        */
        entry.excessKeepalivePeriodThreshold = BIT_4;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.excessKeepalivePeriodThreshold = 7;

        /*
            1.23. Call with out of range entry.excessKeepaliveMessageCounter
            and other valid params.
            Expected: NON GT_OK.
        */
        entry.excessKeepaliveMessageCounter = BIT_3;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.excessKeepaliveMessageCounter = 3;

        /*
            1.24. Call with out of range entry.excessKeepaliveMessageThreshold
            and other valid params.
            Expected: NON GT_OK.
        */
        entry.excessKeepaliveMessageThreshold = BIT_3;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.excessKeepaliveMessageThreshold = 5;

        /*
            1.25. Call with out of range entry.rdiStatus and other valid params.
            Expected: NON GT_OK.
        */
        entry.rdiStatus = BIT_1;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        entry.rdiStatus = 1;

        /*
            1.26. Call with out of range entry.keepaliveTxPeriod and other valid
            params.
            Expected: NON GT_OK.
        */
        entry.keepaliveTxPeriod = BIT_3;

        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamEntrySet(dev, stage, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_OAM_ENTRY_STC            *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamEntryGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage[CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                         CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                         CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   entryIndex[0, 0xFF, 0x7FF],
                   non-null entryPtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range entryIndex[BIT_11] and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range entryPtr and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                                   st                      = GT_OK;
    GT_U8                                       dev                     = 0;
    GT_U32                                      entryIndex              = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT                stage                   = 0;
    CPSS_DXCH_OAM_ENTRY_STC                     entry                   = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage[CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                 CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                 CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                           entryIndex[0, 0xFF, 0x7FF],
                           non-null entryPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        entryIndex = 0;

        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 0xFF;

        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 0x7FF;

        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamEntryGet
                            (dev, stage, entryIndex, &entry),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 0xFF;

        /*
            1.4. Call with last entryIndex and other valid params.
            Expected: GT_OK.
        */
        entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev) - 1;

        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.3. Call with out of range entryIndex and other valid params.
            Expected: NON GT_OK.
        */
        entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev);

        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChOamEntryGet: %d", dev);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        entryIndex = 0xFF;

        /*
            1.4. Call with out of range entryPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamEntryGet(dev, stage, entryIndex, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChOamEntryGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
    entryIndex = 0xFF;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamEntryGet(dev, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamEntryGet(dev, stage, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamExceptionConfigSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT            stage,
    IN  CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        exceptionType,
    IN  CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC     *exceptionConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamExceptionConfigSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                   exceptionConfig{command[CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                           CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                           CPSS_PACKET_CMD_MIRROR_TO_CPU_E ],
                                   cpuCode[CPSS_NET_CONTROL_E,
                                           CPSS_NET_UNKNOWN_UC_E,
                                           CPSS_NET_UNREG_MC_E],
                                   summaryBitEnable[GT_TRUE, GT_FALSE, GT_TRUE]}.
    Expected: GT_OK.
    1.2. Call cpssDxChOamAgingPeriodEntryGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range exceptionType and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range command and other valid params.
    Expected: GT_BAD_PARAM.
    1.6. Call with command[CPSS_PACKET_CMD_MIRROR_TO_CPU_E] and
        out of range cpuCode(is relevant) and other valid params.
    Expected: GT_BAD_PARAM.
    1.7. Call with command[CPSS_PACKET_CMD_FORWARD_E] and
        out of range cpuCode[0x5AAAAAA5](not relevant) and other valid params.
    Expected: GT_OK.
    1.8. Call with out of range exceptionConfigPtr and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                               st                  = GT_OK;
    GT_U8                                   dev                 = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            stage               = 0;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        exceptionType       = 0;
    CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC      exceptionConfig     = {0};
    CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC      tmpConfig           = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                   exceptionConfig{command[CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                           CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                           CPSS_PACKET_CMD_MIRROR_TO_CPU_E ],
                                   cpuCode[CPSS_NET_CONTROL_E,
                                           CPSS_NET_UNKNOWN_UC_E,
                                           CPSS_NET_UNREG_MC_E],
                                   summaryBitEnable[GT_TRUE, GT_FALSE, GT_TRUE]}.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
        exceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        exceptionConfig.cpuCode = CPSS_NET_CONTROL_E;
        exceptionConfig.summaryBitEnable = GT_TRUE;

        st = cpssDxChOamExceptionConfigSet(dev, stage, exceptionType, &exceptionConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &tmpConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.command,
                                         tmpConfig.command,
                                         "got another command: %d",
                                         tmpConfig.command);
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.cpuCode,
                                         tmpConfig.cpuCode,
                                         "got another cpuCode: %d",
                                         tmpConfig.cpuCode);
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.summaryBitEnable,
                                         tmpConfig.summaryBitEnable,
                                         "got another command: %d",
                                         tmpConfig.summaryBitEnable);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;
        exceptionConfig.command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        exceptionConfig.cpuCode = CPSS_NET_UNKNOWN_UC_E;
        exceptionConfig.summaryBitEnable = GT_FALSE;

        st = cpssDxChOamExceptionConfigSet(dev, stage, exceptionType, &exceptionConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &tmpConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.command,
                                         tmpConfig.command,
                                         "got another command: %d",
                                         tmpConfig.command);
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.cpuCode,
                                         tmpConfig.cpuCode,
                                         "got another cpuCode: %d",
                                         tmpConfig.cpuCode);
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.summaryBitEnable,
                                         tmpConfig.summaryBitEnable,
                                         "got another command: %d",
                                         tmpConfig.summaryBitEnable);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E;
        exceptionConfig.command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        exceptionConfig.cpuCode = CPSS_NET_UNREG_MC_E;
        exceptionConfig.summaryBitEnable = GT_TRUE;

        st = cpssDxChOamExceptionConfigSet(dev, stage, exceptionType, &exceptionConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamExceptionConfigGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &tmpConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.command,
                                         tmpConfig.command,
                                         "got another command: %d",
                                         tmpConfig.command);
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.cpuCode,
                                         tmpConfig.cpuCode,
                                         "got another cpuCode: %d",
                                         tmpConfig.cpuCode);
            UTF_VERIFY_EQUAL1_STRING_MAC(exceptionConfig.summaryBitEnable,
                                         tmpConfig.summaryBitEnable,
                                         "got another command: %d",
                                         tmpConfig.summaryBitEnable);
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;
        exceptionConfig.command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        exceptionConfig.cpuCode = CPSS_NET_UNKNOWN_UC_E;
        exceptionConfig.summaryBitEnable = GT_FALSE;


        /*
            1.3. Call with out of range stage and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionConfigSet
                            (dev, stage, exceptionType, &exceptionConfig),
                            stage);

        /*
            1.4. Call with out of range exceptionType and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionConfigSet
                            (dev, stage, exceptionType, &exceptionConfig),
                            exceptionType);

        /*
            1.5. Call with out of range command and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionConfigSet
                            (dev, stage, exceptionType, &exceptionConfig),
                            exceptionConfig.command);

        /*
            1.6. Call with command[CPSS_PACKET_CMD_MIRROR_TO_CPU_E] and
                out of range cpuCode(is relevant) and other valid params.
            Expected: GT_BAD_PARAM.
        */
        exceptionConfig.command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionConfigSet
                            (dev, stage, exceptionType, &exceptionConfig),
                            exceptionConfig.cpuCode);

        /*
            1.7. Call with command[CPSS_PACKET_CMD_FORWARD_E] and
                out of range cpuCode(not relevant) and other valid params.
            Expected: GT_OK.
        */
        exceptionConfig.command = CPSS_PACKET_CMD_FORWARD_E;
        exceptionConfig.cpuCode = 0x5AAAAAA5;

        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &tmpConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        exceptionConfig.command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        exceptionConfig.cpuCode = CPSS_NET_UNKNOWN_UC_E;

        /*
            1.8. Call with out of range exceptionConfigPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChOamExceptionConfigSet(dev, stage, exceptionType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
    exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;
    exceptionConfig.command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    exceptionConfig.cpuCode = CPSS_NET_UNKNOWN_UC_E;
    exceptionConfig.summaryBitEnable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamExceptionConfigSet(dev, stage, exceptionType,
                                           &exceptionConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamExceptionConfigSet(dev, stage, exceptionType,
                                       &exceptionConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamExceptionConfigGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT            stage,
    IN  CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        exceptionType,
    OUT CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC     *exceptionConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamExceptionConfigGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                   non-null exceptionConfigPtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range exceptionType and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range exceptionConfigPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT    exceptionType;
    CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC  exceptionConfig;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                 exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                 non-null exceptionConfigPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &exceptionConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;

        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &exceptionConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E;

        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &exceptionConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionConfigGet
                            (dev, stage, exceptionType, &exceptionConfig),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        /*
            1.3. Call with out of range exceptionType and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionConfigGet
                            (dev, stage, exceptionType, &exceptionConfig),
                            exceptionType);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        /*
            1.4. Call with out of range exceptionConfigPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamExceptionConfigGet: %d",
                                         dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &exceptionConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamExceptionConfigGet(dev, stage, exceptionType, &exceptionConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamOpcodeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_OAM_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH_OAM_OPCODE_TYPE_ENT            opcodeType,
    IN GT_U32                                   opcodeIndex,
    IN GT_U32                                   opcodeValue
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamOpcodeSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   opcodeType[CPSS_DXCH_OAM_OPCODE_TYPE_LM_COUNTED_E,
                              CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E
                              CPSS_DXCH_OAM_OPCODE_TYPE_KEEPALIVE_E],
                   opcodeIndex[0, 1, 3],
                   opcodeValue[0, 0xA5, 0xFF].
    Expected: GT_OK.
    1.2. Call cpssDxChOamTableBaseFlowIdGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range opcodeType and other valid params.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range opcodeIndex[BIT_2] and other valid params.
    Expected: NON GT_OK.
    1.6. Call with out of range opcodeValue[BIT_8] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_U32                          opcodeIndex = 0;
    GT_U32                          opcodeValue = 0;
    GT_U32                          tmpValue    = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    CPSS_DXCH_OAM_OPCODE_TYPE_ENT   opcodeType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                       opcodeType[CPSS_DXCH_OAM_OPCODE_TYPE_LM_COUNTED_E,
                                  CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E,
                                  CPSS_DXCH_OAM_OPCODE_TYPE_KEEPALIVE_E],
                       opcodeIndex[0, 1, 3],
                       opcodeValue[0, 0xA5, 0xFF].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_COUNTED_E;
        opcodeIndex = 0;
        opcodeValue = 0;

        st = cpssDxChOamOpcodeSet(dev, stage, opcodeType, opcodeIndex, opcodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(opcodeValue, tmpValue,
                                         "got another opcodeValue: %d",
                                         tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 0;
        opcodeValue = 0xA5;

        st = cpssDxChOamOpcodeSet(dev, stage, opcodeType, opcodeIndex, opcodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(opcodeValue, tmpValue,
                                         "got another opcodeValue: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 0;
        opcodeValue = 0xFF;

        st = cpssDxChOamOpcodeSet(dev, stage, opcodeType, opcodeIndex, opcodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamTableBaseFlowIdGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(opcodeValue, tmpValue,
                                         "got another opcodeValue: %d",
                                         tmpValue);
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 0;
        opcodeValue = 0xA5;

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeSet
                            (dev, stage, opcodeType, opcodeIndex, opcodeValue),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 0;
        opcodeValue = 0xA5;

        /*
            1.4. Call with out of range opcodeType and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeSet
                            (dev, stage, opcodeType, opcodeIndex, opcodeValue),
                            opcodeType);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 0;
        opcodeValue = 0xA5;

        /*
            1.5. Call with out of range opcodeIndex and other valid params.
            Expected: NON GT_OK.
        */
        opcodeIndex = BIT_2;

        st = cpssDxChOamOpcodeSet(dev, stage, opcodeType, opcodeIndex, opcodeValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 0;
        opcodeValue = 0xA5;

        /*
            1.6. Call with out of range opcodeValue and other valid params.
            Expected: NON GT_OK.
        */
        opcodeValue = BIT_8;

        st = cpssDxChOamOpcodeSet(dev, stage, opcodeType, opcodeIndex, opcodeValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
    opcodeIndex = 0;
    opcodeValue = 0xA5;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamOpcodeSet(dev, stage, opcodeType, opcodeIndex, opcodeValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamOpcodeSet(dev, stage, opcodeType, opcodeIndex, opcodeValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamOpcodeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT            stage,
    IN  CPSS_DXCH_OAM_OPCODE_TYPE_ENT           opcodeType,
    IN  GT_U32                                  opcodeIndex,
    OUT GT_U32                                 *opcodeValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamOpcodeGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   opcodeType[CPSS_DXCH_OAM_OPCODE_TYPE_LM_COUNTED_E,
                                  CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E,
                                  CPSS_DXCH_OAM_OPCODE_TYPE_KEEPALIVE_E],
                   opcodeIndex[0, 1, 3],
                   non-null opcodeValuePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range opcodeTypeType and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range opcodeIndex[BIT_2] and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range opcodeValuePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_U32                          opcodeIndex = 0;
    GT_U32                          opcodeValue = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    CPSS_DXCH_OAM_OPCODE_TYPE_ENT   opcodeType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                       opcodeType[CPSS_DXCH_OAM_OPCODE_TYPE_LM_COUNTED_E,
                                  CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E,
                                  CPSS_DXCH_OAM_OPCODE_TYPE_KEEPALIVE_E],
                       opcodeIndex[0, 1, 3],
                       non-null opcodeValuePtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_COUNTED_E;
        opcodeIndex = 0;

        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &opcodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 0;

        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &opcodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_KEEPALIVE_E;
        opcodeIndex = 3;

        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &opcodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeGet
                            (dev, stage, opcodeType, opcodeIndex, &opcodeValue),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 1;

        /*
            1.3. Call with out of range opcodeType and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeGet
                            (dev, stage, opcodeType, opcodeIndex, &opcodeValue),
                            opcodeType);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 1;

        /*
            1.4. Call with out of range opcodeIndex and other valid params.
            Expected: NON GT_OK.
        */
        opcodeIndex = BIT_2;

        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &opcodeValue);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamOpcodeGet: %d", dev);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
        opcodeIndex = 1;

        /*
            1.5. Call with out of range opcodeValuePtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamOpcodeGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    opcodeType = CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;
    opcodeIndex = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &opcodeValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamOpcodeGet(dev, stage, opcodeType, opcodeIndex, &opcodeValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamOpcodeProfilePacketCommandEntrySet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  GT_U32                              opcode,
    IN  GT_U32                              profile,
    IN  CPSS_PACKET_CMD_ENT                 command,
    IN  GT_U32                              cpuCodeOffset
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamOpcodeProfilePacketCommandEntrySet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   opcode [ 0, 0xA5, 0xFF ],
                   profile [ 0, 3, 7 ],
                   command [CPSS_PACKET_CMD_FORWARD_E,
                            CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                            CPSS_PACKET_CMD_LOOPBACK_E],
                   cpuCodeOffset[ 0, 1, 3 ].
    Expected: GT_OK.
    1.2. Call cpssDxChOamOpcodeProfilePacketCommandEntryGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range opcode[BIT_8] and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range profile[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.6. Call with out of range command and other valid params.
    Expected: GT_BAD_PARAM.
    1.7. Call with out of range cpuCodeOffset[BIT_2] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_U32                          opcode          = 0;
    GT_U32                          profile         = 0;
    GT_U32                          cpuCodeOffset   = 0;
    GT_U32                          tmpValue        = 0;
    CPSS_PACKET_CMD_ENT             command;
    CPSS_PACKET_CMD_ENT             tmpCommand;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                           opcode[0, 0xA5, 0xFF],
                           profile[0, 3, 7],
                           command[CPSS_PACKET_CMD_FORWARD_E,
                                    CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                    CPSS_PACKET_CMD_LOOPBACK_E],
                        cpuCodeOffset[0, 1, 3].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        opcode = 0;
        profile = 0;
        command = CPSS_PACKET_CMD_FORWARD_E;
        cpuCodeOffset = 0;

        st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                           profile, command,
                                                           cpuCodeOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &tmpCommand, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeOffset, tmpValue,
                                         "got another cpuCodeOffset: %d",
                                         tmpValue);

            UTF_VERIFY_EQUAL1_STRING_MAC(command, tmpCommand,
                                         "got another command: %d",
                                         tmpCommand);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        cpuCodeOffset = 1;

        st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                           profile, command,
                                                           cpuCodeOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &tmpCommand, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeOffset, tmpValue,
                                         "got another cpuCodeOffset: %d",
                                         tmpValue);

            UTF_VERIFY_EQUAL1_STRING_MAC(command, tmpCommand,
                                         "got another command: %d",
                                         tmpCommand);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        opcode = 0xFF;
        profile = 7;
        command = CPSS_PACKET_CMD_LOOPBACK_E;
        cpuCodeOffset = 3;

        st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                           profile, command,
                                                           cpuCodeOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamTableBaseFlowIdGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &tmpCommand, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeOffset, tmpValue,
                                         "got another cpuCodeOffset: %d",
                                         tmpValue);

            UTF_VERIFY_EQUAL1_STRING_MAC(command, tmpCommand,
                                         "got another command: %d",
                                         tmpCommand);
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        cpuCodeOffset = 1;

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeProfilePacketCommandEntrySet
                            (dev, stage, opcode, profile, command, cpuCodeOffset),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        cpuCodeOffset = 1;

        /*
            1.4. Call with out of range opcode and other valid params.
            Expected: NON GT_OK.
        */
        opcode = BIT_8;

        st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                           profile, command,
                                                           cpuCodeOffset);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        cpuCodeOffset = 1;

        /*
            1.5. Call with out of range profile and other valid params.
            Expected: NON GT_OK.
        */
        profile = BIT_3;

        st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                           profile, command,
                                                           cpuCodeOffset);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        cpuCodeOffset = 1;

        /*
            1.6. Call with out of range command and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeProfilePacketCommandEntrySet
                            (dev, stage, opcode, profile, command, cpuCodeOffset),
                            command);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        cpuCodeOffset = 1;

        /*
            1.7. Call with out of range cpuCodeOffset and other valid params.
            Expected: NON GT_OK.
        */
        cpuCodeOffset = BIT_2;

        st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                           profile, command,
                                                           cpuCodeOffset);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
    opcode = 0xA5;
    profile = 3;
    command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    cpuCodeOffset = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                           profile, command,
                                                           cpuCodeOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamOpcodeProfilePacketCommandEntrySet(dev, stage, opcode,
                                                       profile, command,
                                                       cpuCodeOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamOpcodeProfilePacketCommandEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  GT_U32                              opcode,
    IN  GT_U32                              profile,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr,
    OUT GT_U32                              *cpuCodeOffsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamOpcodeProfilePacketCommandEntryGet)
{
/*

    ITERATE_DEVICES (SIP5)
    1.1. Call with stage[CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   opcode[0, 0xA5, 0xFF],
                   profile[0, 3, 7],
                   non-null commandPtr,
                   non-null cpuCodeOffsetPtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range opcode[BIT_8] and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range profile[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range commandPtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.6. Call with out of range cpuCodeOffsetPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_U32                          opcode          = 0;
    GT_U32                          profile         = 0;
    GT_U32                          cpuCodeOffset   = 0;
    CPSS_PACKET_CMD_ENT             command;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                           opcode [ 0, 0xA5, 0xFF ],
                           profile [ 0, 3, 7 ],
                           non-null commandPtr,
                           non-null cpuCodeOffsetPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        opcode = 0;
        profile = 0;

        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &command, &cpuCodeOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);


        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;

        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &command, &cpuCodeOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xFF;
        profile = 7;

        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &command, &cpuCodeOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeProfilePacketCommandEntryGet
                            (dev, stage, opcode, profile, &command, &cpuCodeOffset),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;

        /*
            1.3. Call with out of range opcode and other valid params.
            Expected: NON GT_OK.
        */
        opcode = BIT_8;

        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &command, &cpuCodeOffset);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamOpcodeProfilePacketCommandEntryGet: %d", dev);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;

        /*
            1.4. Call with out of range profile and other valid params.
            Expected: NON GT_OK.
        */
        profile = BIT_3;

        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &command, &cpuCodeOffset);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamOpcodeProfilePacketCommandEntryGet: %d", dev);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        opcode = 0xA5;
        profile = 3;

        /*
            1.5. Call with out of range commandPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           NULL, &cpuCodeOffset);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamOpcodeProfilePacketCommandEntryGet: %d", dev);

        /*
            1.6. Call with out of range cpuCodeOffsetPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &command, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamOpcodeProfilePacketCommandEntryGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
    opcode = 0xA5;
    profile = 3;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                           &command, &cpuCodeOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamOpcodeProfilePacketCommandEntryGet(dev, stage, opcode, profile,
                                                       &command, &cpuCodeOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamTableBaseFlowIdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  GT_U32                              baseFlowId
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamTableBaseFlowIdSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   baseFlowId[0, 0xAA55, 0xFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChOamTableBaseFlowIdGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range baseFlowId[BIT_16] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_U32                          baseFlowId  = 0;
    GT_U32                          tmpValue    = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                           baseFlowId[0, 0xAA55, 0xFFFF].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        baseFlowId = 0;

        st = cpssDxChOamTableBaseFlowIdSet(dev, stage, baseFlowId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamTableBaseFlowIdGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(baseFlowId, tmpValue,
                                         "got another baseFlowId: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        baseFlowId = 0xAA55;

        st = cpssDxChOamTableBaseFlowIdSet(dev, stage, baseFlowId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamTableBaseFlowIdGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(baseFlowId, tmpValue,
                                         "got another baseFlowId: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        baseFlowId = 0xFFFF;

        st = cpssDxChOamTableBaseFlowIdSet(dev, stage, baseFlowId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamTableBaseFlowIdGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamTableBaseFlowIdGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(baseFlowId, tmpValue,
                                         "got another baseFlowId: %d", tmpValue);
        }

        /* restore value */
        baseFlowId = 0xAA55;

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamTableBaseFlowIdSet
                            (dev, stage, baseFlowId),
                            stage);

        /*
            1.4. Call with out of range baseFlowId and other valid params.
            Expected: NON GT_OK.
        */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        baseFlowId = BIT_16;

        st = cpssDxChOamTableBaseFlowIdSet(dev, stage, baseFlowId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        baseFlowId = 0xAA55;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamTableBaseFlowIdSet(dev, stage, baseFlowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamTableBaseFlowIdSet(dev, stage, baseFlowId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamTableBaseFlowIdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    OUT  GT_U32                            *baseFlowIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamTableBaseFlowIdGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   non-null baseFlowIdPtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range baseFlowIdPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_U32                          baseFlowId      = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                           non-null baseFlowIdPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamTableBaseFlowIdGet(dev, stage, &baseFlowId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamTableBaseFlowIdGet(dev, stage, &baseFlowId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamTableBaseFlowIdGet
                            (dev, stage, &baseFlowId),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        /*
            1.3. Call with out of range baseFlowIdPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamTableBaseFlowIdGet(dev, stage, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChOamTableBaseFlowIdGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamTableBaseFlowIdGet(dev, stage, &baseFlowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChOamTableBaseFlowIdGet(dev, stage, &baseFlowId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  GT_BOOL                             enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet)
{
    GT_STATUS                       st;
    GT_U8                           dev;
    GT_BOOL                         enable;
    GT_BOOL                         enableGet;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E ;
            stage <= CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E ;
            stage++)
        {
            for(enable = GT_FALSE ; enable <= GT_TRUE ; enable++)
            {
                st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet(dev, stage, enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

                /* check that get the value that was set */
                st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet(dev, stage, &enableGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                             "in stage %d got another enable: %d",
                                             stage ,enableGet);
            }
        }

        enable = GT_TRUE;
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E + 1;
        st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet(dev, stage, enable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        stage += 1000;
        st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet(dev, stage, enable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);


        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet
                            (dev, stage, enable),
                            stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet(dev, stage, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    OUT GT_BOOL                             *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet)
{
    GT_STATUS                       st;
    GT_U8                           dev;
    GT_BOOL                         enableGet;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E ;
            stage <= CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E ;
            stage++)
        {
            st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet(dev, stage, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* check NULL pointer*/
            st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet(dev, stage, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);
        }

        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E + 1;
        st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet(dev, stage, &enableGet);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        stage += 1000;
        st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet(dev, stage, &enableGet);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet
                            (dev, stage, &enableGet),
                            stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet(dev, stage, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet(dev, stage, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamTimeStampEtherTypeSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      etherType
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamTimeStampEtherTypeSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with etherType[0, 0xAA55, 0xFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChOamTimeStampEtherTypeGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range etherType[BIT_16] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_U32                          etherType   = 0;
    GT_U32                          tmpValue    = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with etherType[0, 0xAA55, 0xFFFF].
            Expected: GT_OK.
        */

        /* call with etherType = 0 */
        etherType = 0;

        st = cpssDxChOamTimeStampEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherType);

        /* check with previous set of parms */
        st = cpssDxChOamTimeStampEtherTypeGet(dev, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(etherType, tmpValue,
                                         "got another etherType: %d", tmpValue);
        }

        /* call with etherType = 0xAA55 */
        etherType = 0xAA55;

        st = cpssDxChOamTimeStampEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherType);

        /* check with previous set of parms */
        st = cpssDxChOamTimeStampEtherTypeGet(dev, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(etherType, tmpValue,
                                         "got another etherType: %d", tmpValue);
        }

        /* call with etherType = 0xFFFF */
        etherType = 0xFFFF;

        st = cpssDxChOamTimeStampEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherType);

        /*
            1.2. Call cpssDxChOamTimeStampEtherTypeGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamTimeStampEtherTypeGet(dev, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(etherType, tmpValue,
                                         "got another etherType: %d", tmpValue);
        }

        /*
            1.3. Call with out of range etherType and other valid params.
            Expected: NON GT_OK.
        */
        etherType = BIT_16;

        st = cpssDxChOamTimeStampEtherTypeSet(dev, etherType);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherType);

        /* restore value */
        etherType = 0xAA55;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamTimeStampEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamTimeStampEtherTypeSet(dev, etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamTimeStampEtherTypeGet
(
    IN  GT_U8        devNum,
    OUT  GT_U32      *etherTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamTimeStampEtherTypeGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null etherTypePtr.
    Expected: GT_OK.
    1.2. Call with out of range etherTypePtr[NULL].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_U32                          etherType       = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null etherTypePtr.
            Expected: GT_OK.
        */
        st = cpssDxChOamTimeStampEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range etherTypePtr.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChOamTimeStampEtherTypeGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamTimeStampEtherTypeGet: %d",
                                         dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamTimeStampEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamTimeStampEtherTypeGet(dev, &etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamTimeStampParsingEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamTimeStampParsingEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with enable[GT_TRUE, GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChOamTimeStampParsingEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    GT_BOOL                         enable      = GT_FALSE;
    GT_BOOL                         tmpValue    = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_TRUE, GT_FALSE].
            Expected: GT_OK.
        */

        /* call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChOamTimeStampParsingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* check with previous set of parms */
        st = cpssDxChOamTimeStampParsingEnableGet(dev, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, tmpValue,
                                         "got another enable: %d", tmpValue);
        }

        /* call with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChOamTimeStampParsingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChOamTimeStampParsingEnableGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamTimeStampParsingEnableGet(dev, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, tmpValue,
                                         "got another enable: %d", tmpValue);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamTimeStampParsingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamTimeStampParsingEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamTimeStampParsingEnableGet
(
    IN  GT_U8        devNum,
    OUT  GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamTimeStampParsingEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null etherTypePtr.
    Expected: GT_OK.
    1.2. Call with out of range etherTypePtr[NULL].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_BOOL                         enable      = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChOamTimeStampParsingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range enablePtr.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChOamTimeStampParsingEnableGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamTimeStampParsingEnableGet:"\
                                         " %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamTimeStampParsingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamTimeStampParsingEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamExceptionCounterGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT            stage,
    IN  CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        exceptionType,
    OUT GT_U32                                  *counterValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamExceptionCounterGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                   non-null counterValuePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range exceptionType and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range counterValuePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    GT_U32                              counterValue    = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT    exceptionType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                 exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                 non-null counterValuePtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        st = cpssDxChOamExceptionCounterGet(dev, stage, exceptionType, &counterValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;

        st = cpssDxChOamExceptionCounterGet(dev, stage, exceptionType, &counterValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E;

        st = cpssDxChOamExceptionCounterGet(dev, stage, exceptionType, &counterValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionCounterGet
                            (dev, stage, exceptionType, &counterValue),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        /*
            1.3. Call with out of range exceptionType and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionCounterGet
                            (dev, stage, exceptionType, &counterValue),
                            exceptionType);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        /*
            1.4. Call with out of range counterValuePtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamExceptionCounterGet(dev, stage, exceptionType, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamExceptionCounterGet: %d", dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamExceptionCounterGet(dev, stage, exceptionType, &counterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamExceptionCounterGet(dev, stage, exceptionType, &counterValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamExceptionGroupStatusGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT            stage,
    IN  CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        exceptionType,
    OUT GT_U32                                  groupStatusArr[CPSS_DXCH_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamExceptionGroupStatusGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                   non-null groupStatusArrPtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range exceptionType and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range groupStatusArrPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT    exceptionType;
    GT_U32      groupStatusArr[CPSS_DXCH_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS];

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                 exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E],
                 non-null groupStatusArrPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        st = cpssDxChOamExceptionGroupStatusGet(dev, stage, exceptionType, &groupStatusArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;

        st = cpssDxChOamExceptionGroupStatusGet(dev, stage, exceptionType, &groupStatusArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E;

        st = cpssDxChOamExceptionGroupStatusGet(dev, stage, exceptionType, &groupStatusArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_AGING_E;

        st = cpssDxChOamExceptionGroupStatusGet(dev, stage, exceptionType, &groupStatusArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionGroupStatusGet
                            (dev, stage, exceptionType, &groupStatusArr[0]),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        /*
            1.3. Call with out of range exceptionType and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionGroupStatusGet
                            (dev, stage, exceptionType, &groupStatusArr[0]),
                            exceptionType);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

        /*
            1.4. Call with out of range groupStatusArrPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamExceptionGroupStatusGet(dev, stage, exceptionType, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamExceptionGroupStatusGet: %d", dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamExceptionGroupStatusGet(dev, stage, exceptionType, &groupStatusArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChOamExceptionGroupStatusGet(dev, stage, exceptionType, &groupStatusArr[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamExceptionStatusGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT                stage,
    IN  CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT            exceptionType,
    IN  GT_U32                                      entryIndex,
    OUT GT_U32                                      *entryBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamExceptionStatusGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                                 CPSS_DXCH_OAM_EXCEPTION_TYPE_SUMMARY_E],
                   entryIndex[0, 32, 63],
                   non-null groupStatusArrPtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range exceptionType and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range entryIndex[BIT_6] and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range entryBmp[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                           st              = GT_OK;
    GT_U8                               dev             = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT    exceptionType;
    GT_U32                              entryIndex      = 0;
    GT_U32                              entryBmp[1]     = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                 exceptionType[CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                               CPSS_DXCH_OAM_EXCEPTION_TYPE_SUMMARY_E],
                 entryIndex[0, 32, 63],
                 non-null groupStatusArrPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
        entryIndex = 0;

        st = cpssDxChOamExceptionStatusGet(dev, stage,
                                           exceptionType, entryIndex, &entryBmp[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;
        entryIndex = 32;

        st = cpssDxChOamExceptionStatusGet(dev, stage,
                                           exceptionType, entryIndex, &entryBmp[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_SUMMARY_E;
        entryIndex = 63;

        st = cpssDxChOamExceptionStatusGet(dev, stage,
                                           exceptionType, entryIndex, &entryBmp[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionStatusGet
                            (dev, stage, exceptionType, entryIndex, &entryBmp[0]),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
        entryIndex = 32;

        /*
            1.3. Call with out of range exceptionType and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamExceptionStatusGet
                            (dev, stage, exceptionType, entryIndex, &entryBmp[0]),
                            exceptionType);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
        entryIndex = 32;

        /*
            1.4. Call with last entryIndex and other valid params.
            Expected: GT_OK.
        */
        entryIndex = OAM_AGING_TABLE_NUM_ENTRIES_CNS(dev) - 1;

        st = cpssDxChOamExceptionStatusGet(dev, stage,
                                           exceptionType, entryIndex, &entryBmp[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamExceptionStatusGet: %d",
                                         dev);

        /*
            1.4. Call with out of range entryIndex and other valid params.
            Expected: NON GT_OK.
        */
        entryIndex = OAM_AGING_TABLE_NUM_ENTRIES_CNS(dev);

        st = cpssDxChOamExceptionStatusGet(dev, stage,
                                           exceptionType, entryIndex, &entryBmp[0]);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamExceptionStatusGet: %d",
                                         dev);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
        entryIndex = 32;

        /*
            1.5. Call with out of range groupStatusArrPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamExceptionStatusGet(dev, stage,
                                           exceptionType, entryIndex, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamExceptionStatusGet: %d",
                                         dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    exceptionType = CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
    entryIndex = 32;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamExceptionStatusGet(dev, stage, exceptionType,
                                            entryIndex, &entryBmp[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChOamExceptionStatusGet(dev, stage, exceptionType,
                                        entryIndex, &entryBmp[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamLmOffsetTableSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    entryIndex,
    IN  GT_U32    offset
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamLmOffsetTableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with entryIndex[0, 7, 15],
                   offset[0, 0xA5, 0xFF].
    Expected: GT_OK.
    1.2. Call cpssDxChOamLmOffsetTableGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range entryIndex [16] and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range offset [256] and
        other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st                  = GT_OK;
    GT_U8                           dev                 = 0;
    GT_U32                          entryIndex          = 0;
    GT_U32                          offset              = 0;
    GT_U32                          tmpValue            = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex[0, 7, 15],
                           offset[0, 0xA5, 0xFF].
            Expected: GT_OK.
        */

        /* call with entryIndex[0] */
        entryIndex = 0;
        offset = 0;

        st = cpssDxChOamLmOffsetTableSet(dev, entryIndex, offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check with previous set of parms */
        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(offset, tmpValue,
                                         "got another offset: %d", tmpValue);
        }

        /* call with entryIndex[127] */
        entryIndex = BIT_7 - 1;
        offset = BIT_7 - 1;

        st = cpssDxChOamLmOffsetTableSet(dev, entryIndex, offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check with previous set of parms */
        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(offset, tmpValue,
                                         "got another offset: %d", tmpValue);
        }

        /* call with entryIndex[115] */
        entryIndex = 115;
        offset = BIT_6-1;

        st = cpssDxChOamLmOffsetTableSet(dev, entryIndex, offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChOamTableBaseFlowIdGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &tmpValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(offset, tmpValue,
                                         "got another offset: %d", tmpValue);
        }

        /*
            1.4. Call with out of range entryIndex and other valid params.
            Expected: NON GT_OK.
        */
        entryIndex = BIT_7;

        st = cpssDxChOamLmOffsetTableSet(dev, entryIndex, offset);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        entryIndex = BIT_7 - 1;

        /*
            1.5. Call with out of range offset and other valid params.
            Expected: NON GT_OK.
        */
        offset = BIT_7;

        st = cpssDxChOamLmOffsetTableSet(dev, entryIndex, offset);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    entryIndex = BIT_7 - 1;
    offset = BIT_7 - 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamLmOffsetTableSet(dev, entryIndex, offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChOamLmOffsetTableSet(dev, entryIndex, offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamLmOffsetTableGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      entryIndex,
    OUT GT_U32                                      *offsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamLmOffsetTableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with entryIndex[0, 7, 15],
                   non-null agingPeriodValuePtr.
    Expected: GT_OK.
    1.2. Call with out of range entryIndex[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range offsetPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st                  = GT_OK;
    GT_U8                           dev                 = 0;
    GT_U32                          entryIndex          = 0;
    GT_U32                          offset              = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex[0, 7, 15],
                        non-null offsetPtr.
            Expected: GT_OK.
        */

        /* call with entryIndex[0] */
        entryIndex = 0;

        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with entryIndex[63] */
        entryIndex = BIT_6 - 1;

        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with entryIndex[127] */
        entryIndex = BIT_7 - 1;

        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range entryIndex[BIT_7] and other valid params.
            Expected: NON GT_OK.
        */
        entryIndex = BIT_7;

        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &offset);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamLmOffsetTableGet: %d", dev);

        /* restore value */
        entryIndex = BIT_7 - 1;

        /*
            1.3. Call with out of range offsetPtr[NULL] and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamLmOffsetTableGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    entryIndex = BIT_7 - 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChOamLmOffsetTableGet(dev, entryIndex, &offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E \
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   enable [GT_FALSE \ GT_TRUE].
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range stage.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_OAM_STAGE_TYPE_ENT stage     = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    GT_BOOL                      enable    = GT_FALSE;
    GT_BOOL                      enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        stage  = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        enable = GT_FALSE;

        st = cpssDxChOamEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /* 1.2. */
        st = cpssDxChOamEnableGet(dev, stage, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enableGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", dev);

        /* 1.1. */
        stage  = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        enable = GT_TRUE;

        st = cpssDxChOamEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /* 1.2. */
        st = cpssDxChOamEnableGet(dev, stage, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enableGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", dev);

        /* 1.3. */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamEnableSet
                            (dev, stage, enable),
                            stage);
    }

    stage  = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamEnableSet(dev, stage, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamEnableGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E \
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_OAM_STAGE_TYPE_ENT stage  = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    GT_BOOL                      enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        stage  = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* 1.1. */
        stage  = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* 1.2. */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamEnableGet
                            (dev, stage, &enable),
                            stage);

        /* 1.3. */
        st = cpssDxChOamEnableGet(dev, stage, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);;
    }

    stage  = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamEnableGet(dev, stage, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamPortGroupEntrySet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS
    1.1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                   entryIndex[0, 0xFF, 0x7FF],
                   entry{
                        opcodeParsingEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        megLevelCheckEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        megLevel[0, 3, 7],
                        packetCommandProfile[0, 3, 7],
                        cpuCodeOffset[0, 1, 3],
                        sourceInterfaceCheckEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        sourceInterfaceCheckMode[CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E,
                                                 CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E,
                                                 CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E],
                        sourceInterface{type [CPSS_INTERFACE_PORT_E,
                                              CPSS_INTERFACE_TRUNK_E,
                                              CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum  [devNum],
                                                 portNum [0, max/2, max]},
                                        trunkId [0, max/2, max]},
                        lmCounterCaptureEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        dualEndedLmEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        lmCountingEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        oamPtpOffsetIndex[0, 11, 15],
                        timestampEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        keepaliveAgingEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        ageState[0, 7, 15],
                        agingPeriodIndex[0, 3, 7],
                        agingThreshold[0, 11, 15],
                        hashVerifyEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        lockHashValueEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        protectionLocUpdateEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        flowHash[0, 1024, 4095],
                        excessKeepaliveDetectionEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        excessKeepalivePeriodCounter[0, 3, 15],
                        excessKeepalivePeriodThreshold[0, 7, 15],
                        excessKeepaliveMessageCounter[0, 3, 7],
                        excessKeepaliveMessageThreshold[0, 5, 7],
                        rdiCheckEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        rdiStatus[0, 1, 1],
                        periodCheckEnable[GT_FALSE, GT_TRUE, GT_FALSE],
                        keepaliveTxPeriod[0, 5, 7]}.
    Expected: GT_OK.
    1.1.2. Call cpssDxChOamAgingPeriodEntryGet.
    Expected: GT_OK and the same values as was set.
    1.1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range entryIndex[BIT_15] and other valid params.
    Expected: NON GT_OK.
    1.1.5. Call with out of range entry.megLevel[BIT_3]
    Expected: NON GT_OK.
    1.1.6. Call with out of range entry.packetCommandProfile[BIT_3]
        and other valid params.
    Expected: NON GT_OK.
    1.1.7. Call with out of range entry.cpuCodeOffset[BIT_2] and other valid params.
    Expected: NON GT_OK.
    1.1.8. Call with out of range entry.sourceInterfaceCheckMode and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.9. Call with entry.sourceInterface.type [CPSS_INTERFACE_TRUNK_E],
         out of range entry.sourceInterface.devPort.portNum (not relevant) and
         others valid params.
    Expected: GT_OK.
    1.1.10. Call with out of range entry.sourceInterface.trunkId and others valid params.
    Expected: NOT GT_OK.
    1.1.11. Call with entry.sourceInterface.type [CPSS_INTERFACE_PORT_E],
          out of range entry.sourceInterface.trunkId (not relevant) and
          others valid params.
    Expected: GT_OK.
    1.1.12. Call with out of range entry.sourceInterface.devPort.hwDevNum and
          others valid params.
    Expected: NOT GT_OK.
    1.1.13. Call with out of range entry.sourceInterface.devPort.portNum and
          others valid params.
    Expected: NOT GT_OK.
    1.1.14. Call with wrong entry.sourceInterface.type enum values and
          other valid params.
    Expected: GT_BAD_PARAM.
    1.1.15. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E] and
          out of range entry.oamPtpOffsetIndex (relevant) and other valid params.
    Expected: NON GT_OK.
    1.1.16. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E] and
          out of range entry.oamPtpOffsetIndex (not relevant) and other valid params.
    Expected: GT_OK.
    1.1.17. Call with out of range entry.ageState[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.1.18. Call with out of range entry.agingPeriodIndex[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.1.19. Call with out of range entry.agingThreshold[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.1.20. Call with out of range entry.flowHash[BIT_12] and other valid params.
    Expected: NON GT_OK.
    1.1.21. Call with out of range entry.excessKeepalivePeriodCounter[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.1.22. Call with out of range entry.excessKeepalivePeriodThreshold[BIT_4] and other valid params.
    Expected: NON GT_OK.
    1.1.23. Call with out of range entry.excessKeepaliveMessageCounter[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.1.24. Call with out of range entry.excessKeepaliveMessageThreshold[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.1.25. Call with out of range entry.rdiStatus[BIT_1] and other valid params.
    Expected: NON GT_OK.
    1.1.26. Call with out of range entry.keepaliveTxPeriod[BIT_3] and other valid params.
    Expected: NON GT_OK.
    1.1.27. Call with out of range entryPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32             portGroupId   = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;

    GT_BOOL                      isEqual    = GT_FALSE;
    GT_U32                       entryIndex = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT stage      = 0;
    CPSS_DXCH_OAM_ENTRY_STC      entry      = {0};
    CPSS_DXCH_OAM_ENTRY_STC      tmpEntry   = {0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /* 1.1.1. */
            stage      = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
            entryIndex = 0;

            cpssOsBzero((GT_VOID*) &entry, sizeof(entry));

            entry.sourceInterfaceCheckEnable = GT_TRUE;
            entry.sourceInterfaceCheckMode   = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E;
            entry.sourceInterface.type = CPSS_INTERFACE_PORT_E;
            entry.sourceInterface.devPort.hwDevNum  = dev;
            entry.sourceInterface.devPort.portNum = 0;

            entry.lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_ENABLE_E;
            entry.timestampEnable  = GT_TRUE;
            entry.hashVerifyEnable = GT_TRUE;
              entry.excessKeepaliveDetectionEnable = GT_TRUE;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, entryIndex);

            /* 1.1.2. */
            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &tmpEntry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, entryIndex);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.type,
                                             tmpEntry.sourceInterface.type,
                                             "got another sourceInterface.type: %d",
                                             tmpEntry.sourceInterface.type);
                UTF_VERIFY_EQUAL1_STRING_MAC(0,/* there is no such parameter in the HW ... cpss just 'memset' it to 0 */
                                             tmpEntry.sourceInterface.devPort.hwDevNum,
                                             "(should be always 0) got another sourceInterface.devPort.hwDevNum: %d",
                                             tmpEntry.sourceInterface.devPort.hwDevNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.devPort.portNum,
                                             tmpEntry.sourceInterface.devPort.portNum,
                                             "got another sourceInterface.devPort.portNum: %d",
                                             tmpEntry.sourceInterface.devPort.portNum);

                cpssOsBzero((GT_VOID*) &(entry.sourceInterface), sizeof(entry.sourceInterface));
                cpssOsBzero((GT_VOID*) &(tmpEntry.sourceInterface), sizeof(tmpEntry.sourceInterface));

                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&entry, (GT_VOID*)&tmpEntry, sizeof(entry))) ?
                           GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                        "get another entry than was set: %d, %d", dev, portGroupsBmp);
            }

            /* 1.1.1. */
            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
            entryIndex = 0xFF;

            cpssOsBzero((GT_VOID*) &entry, sizeof(entry));

            entry.opcodeParsingEnable = GT_TRUE;
            entry.megLevelCheckEnable = GT_TRUE;
            entry.megLevel = 3;
            entry.packetCommandProfile = 3;
            entry.cpuCodeOffset = 1;
            entry.sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
            entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);
            entry.lmCounterCaptureEnable = GT_TRUE;
            entry.dualEndedLmEnable = GT_TRUE;
            entry.oamPtpOffsetIndex = 65;
            entry.keepaliveAgingEnable = GT_TRUE;
            entry.ageState = 7;
            entry.agingPeriodIndex = 3;
            entry.agingThreshold = 11;
            entry.lockHashValueEnable = GT_TRUE;
            entry.protectionLocUpdateEnable = GT_TRUE;
            entry.flowHash = 1024;
            entry.excessKeepalivePeriodCounter = 3;
            entry.excessKeepalivePeriodThreshold = 7;
            entry.excessKeepaliveMessageCounter = 3;
            entry.excessKeepaliveMessageThreshold = 5;
            entry.rdiCheckEnable = GT_TRUE;
            entry.rdiStatus = 1;
            entry.periodCheckEnable = GT_TRUE;
            entry.keepaliveTxPeriod = 5;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, entryIndex);

            /* 1.1.2. */
            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &tmpEntry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, entryIndex);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.type,
                                             tmpEntry.sourceInterface.type,
                                             "got another sourceInterface.type: %d",
                                             tmpEntry.sourceInterface.type);
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.trunkId,
                                             tmpEntry.sourceInterface.trunkId,
                                             "got another sourceInterface.trunkId: %d",
                                             tmpEntry.sourceInterface.trunkId);

                cpssOsBzero((GT_VOID*) &(entry.sourceInterface), sizeof(entry.sourceInterface));
                cpssOsBzero((GT_VOID*) &(tmpEntry.sourceInterface), sizeof(tmpEntry.sourceInterface));

                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&entry, (GT_VOID*)&tmpEntry, sizeof(entry))) ?
                           GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                        "get another entry than was set: %d, %d", dev, portGroupsBmp);
            }


            /* 1.1.1. */
            stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
            entryIndex = 0x7FF;

            cpssOsBzero((GT_VOID*) &entry, sizeof(entry));


            entry.opcodeParsingEnable = GT_TRUE;
            entry.megLevelCheckEnable = GT_TRUE;
            entry.megLevel = 7;
            entry.packetCommandProfile = 7;
            entry.cpuCodeOffset = 3;
            entry.sourceInterfaceCheckEnable = GT_TRUE;
            entry.sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
            entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) - 1);

            entry.dualEndedLmEnable = GT_TRUE;
            entry.lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_ENABLE_E;
            entry.oamPtpOffsetIndex = 127;
            entry.timestampEnable = GT_TRUE;
            entry.keepaliveAgingEnable = GT_TRUE;
            entry.ageState = 15;
            entry.agingPeriodIndex = 7;
            entry.agingThreshold = 15;
            entry.hashVerifyEnable = GT_TRUE;
            entry.lockHashValueEnable = GT_TRUE;
            entry.flowHash = 4095;
            entry.excessKeepaliveDetectionEnable = GT_TRUE;
            entry.excessKeepalivePeriodCounter = 15;
            entry.excessKeepalivePeriodThreshold = 15;
            entry.excessKeepaliveMessageCounter = 7;
            entry.excessKeepaliveMessageThreshold = 7;
            entry.rdiCheckEnable = GT_TRUE;
            entry.rdiStatus = 1;
            entry.keepaliveTxPeriod = 7;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, entryIndex);

            /* 1.1.2. */
            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &tmpEntry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, entryIndex);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.type,
                                             tmpEntry.sourceInterface.type,
                                             "got another sourceInterface.type: %d",
                                             tmpEntry.sourceInterface.type);
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.sourceInterface.trunkId,
                                             tmpEntry.sourceInterface.trunkId,
                                             "got another sourceInterface.trunkId: %d",
                                             tmpEntry.sourceInterface.trunkId);

                cpssOsBzero((GT_VOID*) &(entry.sourceInterface), sizeof(entry.sourceInterface));
                cpssOsBzero((GT_VOID*) &(tmpEntry.sourceInterface), sizeof(tmpEntry.sourceInterface));

                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&entry, (GT_VOID*)&tmpEntry, sizeof(entry))) ?
                           GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                        "get another entry than was set: %d, %d", dev, portGroupsBmp);
            }

            /* restore value */
            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
            entryIndex = 0xFF;

            entry.opcodeParsingEnable = GT_TRUE;
            entry.megLevelCheckEnable = GT_TRUE;
            entry.megLevel = 3;
            entry.packetCommandProfile = 3;
            entry.cpuCodeOffset = 1;
            entry.sourceInterfaceCheckEnable = GT_FALSE;
            entry.sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
            entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

            entry.lmCounterCaptureEnable = GT_TRUE;
            entry.dualEndedLmEnable = GT_TRUE;
            entry.lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_DISABLE_E;
            entry.oamPtpOffsetIndex = 11;
            entry.timestampEnable = GT_FALSE;
            entry.keepaliveAgingEnable = GT_TRUE;
            entry.ageState = 7;
            entry.agingPeriodIndex = 3;
            entry.agingThreshold = 11;
            entry.hashVerifyEnable = GT_FALSE;
            entry.lockHashValueEnable = GT_TRUE;
            entry.protectionLocUpdateEnable = GT_TRUE;
            entry.flowHash = 1024;
            entry.excessKeepaliveDetectionEnable = GT_FALSE;
            entry.excessKeepalivePeriodCounter = 3;
            entry.excessKeepalivePeriodThreshold = 7;
            entry.excessKeepaliveMessageCounter = 3;
            entry.excessKeepaliveMessageThreshold = 5;
            entry.rdiCheckEnable = GT_TRUE;
            entry.rdiStatus = 1;
            entry.periodCheckEnable = GT_TRUE;
            entry.keepaliveTxPeriod = 5;

            /* 1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssDxChOamPortGroupEntrySet
                                (dev, portGroupsBmp, stage, entryIndex, &entry),
                                stage);

            /* 1.1.4 */
            entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev)-1;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* 1.1.4 */
            entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev);

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entryIndex = 0xFF;

            /* 1.1.5 */
            entry.megLevel = BIT_3;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.megLevel = 3;

            /* 1.1.6. */
            entry.packetCommandProfile = BIT_3;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.packetCommandProfile = 3;

            /* 1.1.7. */
            entry.cpuCodeOffset = BIT_2;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.cpuCodeOffset = 1;

            /* 1.1.8. */
            UTF_ENUMS_CHECK_MAC(cpssDxChOamPortGroupEntrySet
                                (dev, portGroupsBmp, stage, entryIndex, &entry),
                                entry.sourceInterfaceCheckMode);

            /* 1.1.9. */
            entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
            entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) / 2;

            /* 1.1.10. */
            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev));

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

            /* 1.1.11. */
            entry.sourceInterface.type = CPSS_INTERFACE_PORT_E;
            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev));

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

            /* 1.1.12. */
            entry.sourceInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.sourceInterface.devPort.hwDevNum = dev;

            /* 1.1.13. */
            entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) / 2;

            /* 1.1.14. */
            UTF_ENUMS_CHECK_MAC(cpssDxChOamPortGroupEntrySet
                                (dev, portGroupsBmp, stage, entryIndex, &entry),
                                entry.sourceInterface.type);

            entry.sourceInterface.type = CPSS_INTERFACE_TRUNK_E;
            entry.sourceInterface.devPort.hwDevNum = dev;

            entry.sourceInterface.devPort.portNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) / 2;
            entry.sourceInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) / 2);

            /* 1.1.15. */
            entry.oamPtpOffsetIndex = BIT_7;
            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* 1.1.16. */
            stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
            entry.oamPtpOffsetIndex = 11;

            /* 1.1.17. */
            entry.ageState = BIT_4;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.ageState = 7;

            /* 1.1.18. */
            entry.agingPeriodIndex = BIT_3;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.agingPeriodIndex = 3;

            /* 1.1.19. */
            entry.agingThreshold = BIT_4;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.agingThreshold = 11;

            /* 1.1.20. */
            entry.flowHash = BIT_12;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.flowHash = 1024;

            /* 1.1.21. */
            entry.excessKeepalivePeriodCounter = BIT_4;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.excessKeepalivePeriodCounter = 3;

            /* 1.1.22. */
            entry.excessKeepalivePeriodThreshold = BIT_4;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.excessKeepalivePeriodThreshold = 7;

            /* 1.1.23. */
            entry.excessKeepaliveMessageCounter = BIT_3;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.excessKeepaliveMessageCounter = 3;

            /* 1.1.24. */
            entry.excessKeepaliveMessageThreshold = BIT_3;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.excessKeepaliveMessageThreshold = 5;

            /* 1.1.25. */
            entry.rdiStatus = BIT_1;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.rdiStatus = 1;

            /* 1.1.26. */
            entry.keepaliveTxPeriod = BIT_3;

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entry.keepaliveTxPeriod = 5;

            /* 1.1.27. */
            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, entryPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    stage      = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    entryIndex = 0;

    cpssOsBzero((GT_VOID*) &entry, sizeof(entry));

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamPortGroupEntryGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS
    1.1.1. Call with stage[CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                           CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                           CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   entryIndex[0, 0xFF, 0x7FF],
                   non-null entryPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with out of range entryIndex[BIT_11] and other valid params.
    Expected: NON GT_OK.
    1.1.4. Call with out of range entryPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32             portGroupId   = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;

    GT_U32                       entryIndex = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT stage      = 0;
    CPSS_DXCH_OAM_ENTRY_STC      entry      = {0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /* 1.1.1. */
            stage      = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
            entryIndex = 0;

            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* 1.1.1. */
            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
            entryIndex = 0xFF;

            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* 1.1.1. */
            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
            entryIndex = 0x7FF;

            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* 1.1.2. */
            UTF_ENUMS_CHECK_MAC(cpssDxChOamPortGroupEntryGet
                                (dev, portGroupsBmp, stage, entryIndex, &entry),
                                stage);

            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
            entryIndex = 0xFF;

            /* 1.1.3. */
            entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev)-1;

            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* 1.1.3. */
            entryIndex = OAM_TABLE_NUM_ENTRIES_CNS(dev);

            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, stage);

            stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
            entryIndex = 0xFF;

            /* 1.1.4. */
            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, entryPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    stage      = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    entryIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamPortGroupEntryGet(dev, portGroupsBmp, stage, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamPduCpuCodeSet)
{
/*
    ITERATE_DEVICE
    1.1. Call cpssDxChOamPduCpuCodeSet with relevant
         cpuCode [CPSS_NET_CONTROL_E/
                  CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E/
                  CPSS_NET_INTERVENTION_PORT_LOCK_E/
                  CPSS_NET_LOCK_PORT_MIRROR_E/
                  CPSS_NET_CONTROL_BPDU_E].
    Expected: GT_OK.
    1.2. Call cpssDxChOamPduCpuCodeGet.
    Expected: GT_OK and the same cpuCode.
    1.3. Call with not relevant cpuCode[CPSS_NET_UDP_BC_MIRROR_TRAP1_E].
    Expected: NOT GT_OK.
    1.4. Call with wrong enum value cpuCode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_CONTROL_E;
    CPSS_NET_RX_CPU_CODE_ENT cpuCodeGet = CPSS_NET_CONTROL_E;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        cpuCode = CPSS_NET_CONTROL_E;

        st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamPduCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;

        st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamPduCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_INTERVENTION_PORT_LOCK_E;

        st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamPduCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_LOCK_PORT_MIRROR_E;

        st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamPduCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.1.  */
        cpuCode = CPSS_NET_CONTROL_BPDU_E;

        st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamPduCpuCodeGet(dev, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                   "get another cpuCode than was set: %d", dev);

        /*  1.3.  */
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP1_E;

        st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamPduCpuCodeSet(dev, cpuCode),
                            cpuCode);
    }
    cpuCode = CPSS_NET_OAM_PDU_TRAP_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamPduCpuCodeSet(dev, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamPduCpuCodeGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChOamPduCpuCodeGet
                with not NULL cpuCodePtr.
    Expected: GT_OK.
    1.2. Call with NULL cpuCodePtr.
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
        /*  1.1.  */
        st = cpssDxChOamPduCpuCodeGet(dev, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamPduCpuCodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamPduCpuCodeGet(dev, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamPduCpuCodeGet(dev, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamMplsCwChannelTypeProfileSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChOamMplsCwChannelTypeProfileSet with relevant value
              profile[1/7/15],
              channelTypeId[0/32766/65535].
    Expected: GT_OK.
    1.2. Call cpssDxChOamMplsCwChannelTypeProfileGet.
    Expected: GT_OK and the same profile.
    1.3. Call with out of range profile[16].
    Expected: NOT GT_OK.
    1.4. Call with out of range channelTypeId[65536].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32   profile = 1;
    GT_U32   channelTypeId = 0;
    GT_U32   channelTypeIdGet = 0;
    GT_BOOL  enable = GT_TRUE;
    GT_BOOL  enableGet = GT_TRUE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        profile       = 1;
        channelTypeId = 0;
        enable        = GT_TRUE;

        st = cpssDxChOamMplsCwChannelTypeProfileSet(
            dev, profile, channelTypeId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeIdGet, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st,
            "cpssDxChOamMplsCwChannelTypeProfileGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(
            channelTypeId, channelTypeIdGet,
            "get another channelTypeId than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            enable, enableGet,
            "get another enable than was set: %d", dev);

        /*  1.1.  */
        profile       = 7;
        channelTypeId = 32766;
        enable        = GT_FALSE;

        st = cpssDxChOamMplsCwChannelTypeProfileSet(
            dev, profile, channelTypeId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeIdGet, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st,
            "cpssDxChOamMplsCwChannelTypeProfileGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(
            channelTypeId, channelTypeIdGet,
            "get another channelTypeId than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            enable, enableGet,
            "get another enable than was set: %d", dev);


        /*  1.1.  */
        profile       = 15;
        channelTypeId = 65535;
        enable        = GT_TRUE;

        st = cpssDxChOamMplsCwChannelTypeProfileSet(
            dev, profile, channelTypeId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeIdGet, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st,
            "cpssDxChOamMplsCwChannelTypeProfileGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(
            channelTypeId, channelTypeIdGet,
            "get another channelTypeId than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            enable, enableGet,
            "get another enable than was set: %d", dev);

        /*  1.3.  */
        profile = 16;

        st = cpssDxChOamMplsCwChannelTypeProfileSet(
            dev, profile, channelTypeId, enable);

        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        profile = 1;

        /*  1.4.  */
        channelTypeId = 65536;

        st = cpssDxChOamMplsCwChannelTypeProfileSet(
            dev, profile, channelTypeId, enable);

        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        channelTypeId = 0;
    }
    profile = 1;
    channelTypeId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamMplsCwChannelTypeProfileSet(
            dev, profile, channelTypeId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamMplsCwChannelTypeProfileSet(
        dev, profile, channelTypeId, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChOamMplsCwChannelTypeProfileGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChOamMplsCwChannelTypeProfileGet with relevant value
              profile channelTypeId[1/7/15],
              and not NULL channelTypeIdPtr.
    Expected: GT_OK.
    1.2. Call with out of range profile[16]
    Expected: NOT GT_OK.
    1.3. Call with NULL channelTypeIdPtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      channelTypeId = 0;
    GT_U32      profile = 1;
    GT_BOOL     enable = GT_TRUE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        profile = 1;

        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeId, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        profile = 7;

        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeId, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        profile = 15;

        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeId, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        profile = 16;
        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeId, &enable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        channelTypeId = 0;

        /*  1.3.  */
        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, NULL /*&channelTypeId*/, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeId, NULL /*&enable*/);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamMplsCwChannelTypeProfileGet(
            dev, profile, &channelTypeId, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamMplsCwChannelTypeProfileGet(
        dev, profile, &channelTypeId, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamAgingBitmapUpdateModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT                stage,
    IN  CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamAgingBitmapUpdateModeSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
              mode[CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ONLY_FAILURES_E,
                    CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ALL_E].
    Expected: GT_OK.
    1.2. Call cpssDxChOamAgingBitmapUpdateModeGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range mode and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT  mode;
    CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT  tmpValue;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E],
                          mode[CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ONLY_FAILURES_E,
                                CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ALL_E].
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        mode = CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ONLY_FAILURES_E;

        st = cpssDxChOamAgingBitmapUpdateModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, tmpValue,
                                         "got another mode: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        mode = CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ONLY_FAILURES_E;

        st = cpssDxChOamAgingBitmapUpdateModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, tmpValue,
                                         "got another mode: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;
        mode = CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ALL_E;

        st = cpssDxChOamAgingBitmapUpdateModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* check with previous set of parms */
        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, tmpValue,
                                         "got another mode: %d", tmpValue);
        }

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        mode = CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ALL_E;

        st = cpssDxChOamAgingBitmapUpdateModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChOamAgingBitmapUpdateModeGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &tmpValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, tmpValue,
                                         "got another mode: %d", tmpValue);
        }

        /* restore value */
        mode = CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ALL_E;

        /*
            1.3. Call with out of range stage and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamAgingBitmapUpdateModeSet
                            (dev, stage, mode),
                            stage);

        /*
            1.4. Call with out of range mode and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamAgingBitmapUpdateModeSet
                            (dev, stage, mode),
                            mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore values */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    mode = CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ALL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamAgingBitmapUpdateModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamAgingBitmapUpdateModeSet(dev, stage, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamAgingBitmapUpdateModeGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT                stage,
    OUT CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamAgingBitmapUpdateModeGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   non-null modePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range modePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT  mode;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                           non-null modePtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamAgingBitmapUpdateModeGet
                            (dev, stage, &mode),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        /*
            1.3. Call with out of range modePtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamAgingBitmapUpdateModeGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamAgingBitmapUpdateModeGet(dev, stage, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamHashBitSelectionSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage,
    IN  GT_U32                          hashFirstBit,
    IN  GT_U32                          hashLastBit
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamHashBitSelectionSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
              hashFirstBit hashLastBit Expected
                   1          31         GT_OK
                   1          32         GT_OUT_OF_RANGE
                   31          1         GT_OK
                   32          1         GT_OUT_OF_RANGE
                   5          28         GT_OK
                   15         11         GT_OK
    1.2. Call cpssDxChOamAgingBitmapUpdateModeGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range mode and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st          = GT_OK;
    GT_U8                           dev         = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    GT_U32                          hashFirstBitArr[] = { 1, 1,31,32, 5,10,30};
    GT_U32                          hashLastBitArr[] =  {31,32, 1, 1,28,11,31};
    GT_STATUS                       stExpct[] = {GT_OK, GT_OUT_OF_RANGE, GT_BAD_PARAM, GT_OUT_OF_RANGE, GT_OK, GT_OK, GT_OK};
    GT_U32                          hashFirstBitGet;
    GT_U32                          hashLastBitGet;
    GT_U32                          ii;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all stages
        */
        for (stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E; stage <= CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E; stage++)
        {
            for (ii = 0; ii < sizeof(hashFirstBitArr) / sizeof(hashFirstBitArr[0]); ii++)
            {
                st = cpssDxChOamHashBitSelectionSet(dev,stage,hashFirstBitArr[ii],hashLastBitArr[ii]);
                UTF_VERIFY_EQUAL4_PARAM_MAC(stExpct[ii], st, dev, stage, hashFirstBitArr[ii], hashLastBitArr[ii]);
                if (st == GT_OK)
                {
                    st = cpssDxChOamHashBitSelectionGet(dev,stage,&hashFirstBitGet,&hashLastBitGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

                    UTF_VERIFY_EQUAL1_STRING_MAC(hashFirstBitArr[ii], hashFirstBitGet,
                                                 "got another hashFirstBit: %d", hashFirstBitGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(hashLastBitArr[ii], hashLastBitGet,
                                                 "got another hashLastBit: %d", hashLastBitGet);
                }
            }
        }

        ii = 0;
        /*
            1.3. Call with out of range stage and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamHashBitSelectionSet
                            (dev,stage,hashFirstBitArr[ii],hashLastBitArr[ii]),
                            stage);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore values */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    ii = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamHashBitSelectionSet(dev,stage,hashFirstBitArr[ii],hashLastBitArr[ii]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamHashBitSelectionSet(dev,stage,hashFirstBitArr[ii],hashLastBitArr[ii]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamHashBitSelectionGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage,
    OUT GT_U32                          *hashFirstBitPtr,
    OUT GT_U32                          *hashLastBitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamHashBitSelectionGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   non-null hashFirstBit and hashLastBit.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range hashFirstBitPtr and hashLastBitPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;
    GT_U32                          hashFirstBit;
    GT_U32                          hashLastBit;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                                  CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                           non-null hashFirstBitPtr and hashLastBitPtr.
            Expected: GT_OK.
        */

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        st = cpssDxChOamHashBitSelectionGet(dev, stage, &hashFirstBit, &hashLastBit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E;

        st = cpssDxChOamHashBitSelectionGet(dev, stage, &hashFirstBit, &hashLastBit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamHashBitSelectionGet
                            (dev, stage, &hashFirstBit, &hashLastBit),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        /*
            1.3. Call with NULL pointer for hashFirstBitPtr and hashLastBitPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChOamHashBitSelectionGet(dev, stage, NULL, &hashLastBit);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                         "cpssDxChOamHashBitSelectionGet: %d", dev);

        st = cpssDxChOamHashBitSelectionGet(dev, stage, &hashFirstBit, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                         "cpssDxChOamHashBitSelectionGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamHashBitSelectionGet(dev, stage, &hashFirstBit, &hashLastBit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamHashBitSelectionGet(dev, stage, &hashFirstBit, &hashLastBit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChOamKeepaliveForPacketCommandEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  CPSS_PACKET_CMD_ENT                 command,
    IN  GT_BOOL                             enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChOamKeepaliveForPacketCommandEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   command [CPSS_PACKET_CMD_DROP_SOFT_E,
                            CPSS_PACKET_CMD_DROP_HARD_E],
                   enable [GT_TRUE,
                           GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChOamKeepaliveForPacketCommandEnablGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range command and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_BOOL                         applicCmd;
    GT_BOOL                         enable;
    GT_BOOL                         enableGet;
    CPSS_PACKET_CMD_ENT             command;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
         1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   command [CPSS_PACKET_CMD_DROP_SOFT_E,
                            CPSS_PACKET_CMD_DROP_HARD_E],
                   enable [GT_TRUE,
                           GT_FALSE].
            Expected: GT_OK.
        */

        /* iterate on all stages */
        for (stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E; stage <= CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E; stage++)
        {
             /* iterate on all commands */
            for (command = CPSS_PACKET_CMD_FORWARD_E; command <= CPSS_PACKET_CMD_LOOPBACK_E; command++)
            {
                applicCmd = GT_FALSE;
                if (stage == CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E)
                {
                    if ((command == CPSS_PACKET_CMD_DROP_HARD_E) ||
                        (command == CPSS_PACKET_CMD_DROP_SOFT_E))
                    {
                        applicCmd = GT_TRUE;
                    }
                }
                else
                {
                    if (command == CPSS_PACKET_CMD_DROP_HARD_E)
                    {
                        applicCmd = GT_TRUE;
                    }
                }

                for (enable = GT_FALSE; enable <= GT_TRUE; enable++)
                {
                    st = cpssDxChOamKeepaliveForPacketCommandEnableSet(dev, stage, command, enable);
                    if (applicCmd)
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, stage, command, enable);

                        /* check with previous set of parms */
                        st = cpssDxChOamKeepaliveForPacketCommandEnableGet(dev, stage, command, &enableGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, stage, command, enable);
                        if (GT_OK == st)
                        {
                            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                                         "got another enable: %d",
                                                         enableGet);
                        }
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, stage, command, enable);
                    }
                }
            }
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        command = CPSS_PACKET_CMD_DROP_HARD_E;
        enable = GT_FALSE;

        /*
            1.3. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamKeepaliveForPacketCommandEnableSet
                            (dev, stage, command, enable),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        /*
            1.4. Call with out of range command and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamKeepaliveForPacketCommandEnableSet
                            (dev, stage, command, enable),
                            command);

        /* restore value */
        command = CPSS_PACKET_CMD_DROP_HARD_E;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    command = CPSS_PACKET_CMD_DROP_HARD_E;
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamKeepaliveForPacketCommandEnableSet(dev, stage, command, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamKeepaliveForPacketCommandEnableSet(dev, stage, command, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChOamKeepaliveForPacketCommandEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_STAGE_TYPE_ENT        stage,
    IN  CPSS_PACKET_CMD_ENT                 command,
    OUT GT_BOOL                            *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChOamKeepaliveForPacketCommandEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   command [CPSS_PACKET_CMD_DROP_SOFT_E,
                            CPSS_PACKET_CMD_DROP_HARD_E],
                   non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range stage and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range command and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_BOOL                         applicCmd;
    GT_BOOL                         enableGet;
    CPSS_PACKET_CMD_ENT             command;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT    stage;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
         1.1. Call with stage [CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
                          CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E],
                   command [CPSS_PACKET_CMD_DROP_SOFT_E,
                            CPSS_PACKET_CMD_DROP_HARD_E],
                           non-null enablePtr.
            Expected: GT_OK.
        */
        /* iterate on all stages */
        for (stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E; stage <= CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E; stage++)
        {
             /* iterate on all commands */
            for (command = CPSS_PACKET_CMD_FORWARD_E; command <= CPSS_PACKET_CMD_LOOPBACK_E; command++)
            {
                applicCmd = GT_FALSE;
                if (stage == CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E)
                {
                    if ((command == CPSS_PACKET_CMD_DROP_HARD_E) ||
                        (command == CPSS_PACKET_CMD_DROP_SOFT_E))
                    {
                        applicCmd = GT_TRUE;
                    }
                }
                else
                {
                    if (command == CPSS_PACKET_CMD_DROP_HARD_E)
                    {
                        applicCmd = GT_TRUE;
                    }
                }

                st = cpssDxChOamKeepaliveForPacketCommandEnableGet(dev, stage, command, &enableGet);
                if (applicCmd)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, command);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, stage, command);
                }
            }
        }

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
        command = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.2. Call with out of range stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamKeepaliveForPacketCommandEnableGet
                            (dev, stage, command, &enableGet),
                            stage);

        /* restore value */
        stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;

        /*
            1.3. Call with out of range command and other valid params.
            Expected: NON GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChOamKeepaliveForPacketCommandEnableGet
                            (dev, stage, command, &enableGet),
                            command);

        /* restore value */
        command = CPSS_PACKET_CMD_DROP_HARD_E;
        /*
            1.4. Call with out of range cpuCodeLsBitsPtr and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChOamKeepaliveForPacketCommandEnableGet(dev, stage, command, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChOamKeepaliveForPacketCommandEnablGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    stage = CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E;
    command = CPSS_PACKET_CMD_DROP_HARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChOamKeepaliveForPacketCommandEnableGet(dev, stage, command, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChOamKeepaliveForPacketCommandEnableGet(dev, stage, command, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChOam suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChOam)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamAgingDaemonEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamAgingDaemonEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamAgingPeriodEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamAgingPeriodEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamCpuCodeBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamCpuCodeBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamDualEndedLmPacketCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamDualEndedLmPacketCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamExceptionConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamExceptionConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamOpcodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamOpcodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamOpcodeProfilePacketCommandEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamOpcodeProfilePacketCommandEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamTableBaseFlowIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamTableBaseFlowIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamTimeStampEtherTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamTimeStampEtherTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamTimeStampParsingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamTimeStampParsingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamExceptionCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamExceptionGroupStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamExceptionStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamLmOffsetTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamLmOffsetTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamPortGroupEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamPortGroupEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamPduCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamPduCpuCodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamMplsCwChannelTypeProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamMplsCwChannelTypeProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamAgingBitmapUpdateModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamAgingBitmapUpdateModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamHashBitSelectionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamHashBitSelectionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamKeepaliveForPacketCommandEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChOamKeepaliveForPacketCommandEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChOam)


