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
* @file cpssDxChPolicerUT.c
*
* @brief Unit tests for cpssDxChPolicer, that provides
* Ingress Policing Engine function implementations.
*
* @version   17
********************************************************************************
*/
/* includes */
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerPacketSizeModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT        mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerPacketSizeModeSet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with mode [CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E /
                                  CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E /
                                  CPSS_POLICER_PACKET_SIZE_L3_ONLY_E].
    Expected: GT_OK.
    1.2. Call with non-supported mode [CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E].
    Expected: NON GT_OK.
    1.3. Call function with wrong enum values enum mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                         st   = GT_OK;

    GT_U8                             dev;
    CPSS_POLICER_PACKET_SIZE_MODE_ENT mode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E /
                                          CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E /
                                          CPSS_POLICER_PACKET_SIZE_L3_ONLY_E].
            Expected: GT_OK.
        */

        /* Call function with mode [CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E] */
        mode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;

        st = cpssDxChPolicerPacketSizeModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* Call function with mode [CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E] */
        mode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

        st = cpssDxChPolicerPacketSizeModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* Call function with mode [CPSS_POLICER_PACKET_SIZE_L3_ONLY_E] */
        mode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;

        st = cpssDxChPolicerPacketSizeModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call with non-supported mode [CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E].
            Expected: NON GT_OK.
        */
        mode = CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E;

        st = cpssDxChPolicerPacketSizeModeSet(dev, mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerPacketSizeModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerPacketSizeModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerPacketSizeModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerDropRedModeSet
(
    IN  GT_U8                      devNum,
    IN  CPSS_DROP_MODE_TYPE_ENT    dropRedMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerDropRedModeSet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with dropRedMode [CPSS_DROP_MODE_SOFT_E /
                                         CPSS_DROP_MODE_HARD_E].
    Expected: GT_OK.
    1.2. Call function with wrong enum values enum dropRedMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st   = GT_OK;

    GT_U8                   dev;
    CPSS_DROP_MODE_TYPE_ENT mode = CPSS_DROP_MODE_SOFT_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with dropRedMode [CPSS_DROP_MODE_SOFT_E /
                                                 CPSS_DROP_MODE_HARD_E].
        */

        /* Call function with mode [CPSS_DROP_MODE_SOFT_E] */
        mode = CPSS_DROP_MODE_SOFT_E;

        st = cpssDxChPolicerDropRedModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* Call function with mode [CPSS_DROP_MODE_HARD_E] */
        mode = CPSS_DROP_MODE_HARD_E;

        st = cpssDxChPolicerDropRedModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call function with wrong enum values dropRedMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerDropRedModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DROP_MODE_SOFT_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerDropRedModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerDropRedModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerPacketSizeModeForTunnelTermSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_POLICER_PACKET_SIZE_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerPacketSizeModeForTunnelTermSet)
{
/*
    ITERATE_DEVICES (DxCh2)
    1.1. Call function with mode [CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E /
                                  CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E /
                                  CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E].
    Expected: GT_OK.
    1.2. Call with non-supported mode [CPSS_POLICER_PACKET_SIZE_L3_ONLY_E].
    Expected: NON GT_OK.
    1.3. Call function with wrong enum values enum mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                         st   = GT_OK;

    GT_U8                             dev;
    CPSS_POLICER_PACKET_SIZE_MODE_ENT mode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E /
                                          CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E /
                                          CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E].
            Expected: GT_OK.
        */

        /* Call function with mode [CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E] */
        mode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;

        st = cpssDxChPolicerPacketSizeModeForTunnelTermSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* Call function with mode [CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E] */
        mode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

        st = cpssDxChPolicerPacketSizeModeForTunnelTermSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* Call function with mode [CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E] */
        mode = CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E;

        st = cpssDxChPolicerPacketSizeModeForTunnelTermSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call with non-supported mode [CPSS_POLICER_PACKET_SIZE_L3_ONLY_E].
            Expected: NON GT_OK.
        */
        mode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;

        st = cpssDxChPolicerPacketSizeModeForTunnelTermSet(dev, mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call function with wrong enum values enum mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerPacketSizeModeForTunnelTermSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerPacketSizeModeForTunnelTermSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerPacketSizeModeForTunnelTermSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerTokenBucketModeSet
(
    IN GT_U8                      devNum,
    IN CPSS_POLICER_TB_MODE_ENT   mode,
    IN CPSS_POLICER_MRU_ENT       mru
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerTokenBucketModeSet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with mode [CPSS_POLICER_TB_STRICT_E/
                                  CPSS_POLICER_TB_LOOSE_E]
                            and mru [CPSS_POLICER_MRU_1536_E /
                                     CPSS_POLICER_MRU_10K_E].
    Expected: GT_OK.
    1.2. Call function with wrong enum values enum mode.
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values enum mru.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                st   = GT_OK;

    GT_U8                    dev;
    CPSS_POLICER_TB_MODE_ENT mode = CPSS_POLICER_TB_STRICT_E;
    CPSS_POLICER_MRU_ENT     mru  = CPSS_POLICER_MRU_1536_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_POLICER_TB_STRICT_E/
                                          CPSS_POLICER_TB_LOOSE_E]
                            and mru [CPSS_POLICER_MRU_1536_E /
                                     CPSS_POLICER_MRU_10K_E].
            Expected: GT_OK.
        */

        /* Call function with mode [CPSS_POLICER_TB_STRICT_E], mru [CPSS_POLICER_MRU_1536_E] */
        mode = CPSS_POLICER_TB_STRICT_E;
        mru = CPSS_POLICER_MRU_1536_E;

        st = cpssDxChPolicerTokenBucketModeSet(dev, mode, mru);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mode, mru);

        /* Call function with mode [CPSS_POLICER_TB_LOOSE_E], mru [CPSS_POLICER_MRU_10K_E] */
        mode = CPSS_POLICER_TB_LOOSE_E;
        mru = CPSS_POLICER_MRU_10K_E;

        st = cpssDxChPolicerTokenBucketModeSet(dev, mode, mru);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mode, mru);

        /*
            1.2. Call function with wrong enum values enum mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerTokenBucketModeSet
                            (dev, mode, mru),
                            mode);

        /*
            1.3. Call function with wrong enum values enum mru.
            Expected: GT_BAD_PARAM.
        */
        mode = CPSS_POLICER_TB_STRICT_E;
        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerTokenBucketModeSet
                            (dev, mode, mru),
                            mru);
    }

    mode = CPSS_POLICER_TB_STRICT_E;
    mru = CPSS_POLICER_MRU_1536_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerTokenBucketModeSet(dev, mode, mru);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerTokenBucketModeSet(dev, mode, mru);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerEntrySet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            entryIndex,
    IN  CPSS_DXCH_POLICER_ENTRY_STC       *entryPtr,
    OUT CPSS_DXCH_POLICER_TB_PARAMS_STC   *tbParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerEntrySet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with entryIndex [0],
                            entryPtr [{policerEnable = GT_TRUE,
                                       meterColorMode = PSS_POLICER_COLOR_AWARE_E,
                                       tbParams = {100, 1000},
                                       counterEnable = GT_TRUE,
                                       counterSetIndex = 10,
                                       cmd = CPSS_DXCH_POLICER_CMD_BY_ENTRY_E,
                                       qosProfile = 10,
                                       modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                       modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E }],
                            and non-NULL tbParamsPtr.
    Expected: GT_OK.
    1.2. Call cpssDxChPolicerEntryGet with entryIndex [0]
                                           and entryPtr is non-NULL.
    Expected: GT_OK and parameters the entry the same as were written (by fields - exclude tbParams, and take into attention that one fields depend on another).
    1.3. Call with entryIndex [0xFFFF] (no constraints in function's contract)
                   and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.

    Check fields of entry
    1.4. Call with wrong enum values enum meterColorMode
                   and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with tbParams.cir [0xFFFF] (no constraints in function's contract)
                   and other parameters the same as in 1.1.
    Expected: GT_OK.
    1.6. Call with tbParams.cbs [0xFFFF] (no constraints in function's contract)
                   and other parameters the same as in 1.1.
    Expected: GT_OK.
    1.7. Call with out of range counterEnable [GT_FALSE]
                   and counterSetIndex [100] (relevant when counterEnable == GT_TRUE)
                   and other parameters the same as in 1.1.
    Expected: GT_OK.
    1.8. Call with out of range counterSetIndex [100],
                   counterEnable [GT_TRUE]
                   and other parameters the same as in 1.1.
    Expected: NON GT_OK.
    1.9. Call with wrong enum values enum cmd
                   and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call with cmd [CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E]
                    and qosProfile [0xFFFF]  and  qosProfile[50] (no constraints in function's contract)
                    and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM for qosProfile [0xFFFF] and GT_OK for qosProfile[50]
    1.11. Call with cmd [CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E]
                    and wrong enum values enum modifyUp
                    and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.12. Call with cmd [CPSS_DXCH_POLICER_CMD_NONE_E]
                    and wrong enum values enum modifyUp
                    and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.13. Call with cmd [CPSS_DXCH_POLICER_CMD_NONE_E]
                    and wrong enum values enum modifyDscp
                    and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.14. Call with cmd [CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E]
                    and wrong enum values enum modifyDscp
                    and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.15. Call with entryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.16. Call with tbParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.17. Call cpssDxChPolicerEntryInvalidate with entryIndex [0] to invalidate changes.
    Expect: GT_OK.
*/
    GT_STATUS                       st    = GT_OK;

    GT_U8                           dev;
    GT_U32                          index = 0;
    CPSS_DXCH_POLICER_ENTRY_STC     entry;
    CPSS_DXCH_POLICER_TB_PARAMS_STC params;
    CPSS_DXCH_POLICER_ENTRY_STC     entryGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with entryIndex [0],
                            entryPtr [{policerEnable = GT_TRUE,
                                       meterColorMode = PSS_POLICER_COLOR_AWARE_E,
                                       tbParams = {100, 1000},
                                       counterEnable = GT_TRUE,
                                       counterSetIndex = 10,
                                       cmd = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E,
                                       qosProfile = 10,
                                       modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                       modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E }],
                            and non-NULL tbParamsPtr.
            Expected: GT_OK.
        */
        index                 = 0;
        entry.policerEnable   = GT_TRUE;
        entry.meterColorMode  = CPSS_POLICER_COLOR_AWARE_E;
        entry.tbParams.cir    = 100;
        entry.tbParams.cbs    = 1000;
        entry.counterEnable   = GT_TRUE;
        entry.counterSetIndex = 10;
        entry.cmd             = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
        entry.qosProfile      = 10;
        entry.modifyDscp      = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        entry.modifyUp        = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call cpssDxChPolicerEntryGet with entryIndex [0]
                                              and entryPtr is non-NULL.
            Expected: GT_OK and parameters the entry the same as were written
            (by fields - exclude tbParams, and take into attention that one fields depend on another).
        */
        st = cpssDxChPolicerEntryGet(dev, index, &entryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPolicerEntryGet: %d, %d", dev, index);

        /* Verifying entryPtr */
        UTF_VERIFY_EQUAL1_STRING_MAC(entry.policerEnable, entryGet.policerEnable,
                       "get another entryPtr->policerEnable than was set: %d", dev);
        if (GT_TRUE == entry.policerEnable)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.meterColorMode, entryGet.meterColorMode,
                           "get another entryPtr->meterColorMode than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.counterEnable, entryGet.counterEnable,
                           "get another entryPtr->counterEnable than was set: %d", dev);
            if (GT_TRUE == entry.counterEnable)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.counterSetIndex, entryGet.counterSetIndex,
                               "get another entryPtr->counterSetIndex than was set: %d", dev);
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.cmd, entryGet.cmd,
                           "get another entryPtr->cmd than was set: %d", dev);
            if (CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E == entry.cmd)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.qosProfile, entryGet.qosProfile,
                               "get another entryPtr->qosProfile than was set: %d", dev);
            }
            if ((CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E == entry.cmd) ||
                (CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E == entry.cmd))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.modifyDscp, entryGet.modifyDscp,
                               "get another entryPtr->modifyDscp than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.modifyUp, entryGet.modifyUp,
                               "get another entryPtr->modifyUp than was set: %d", dev);
            }
        }
        /*
            1.3. Call with entryIndex [0xFFFF] and 255 (no constraints in function's contract)
                           and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM for index [0xFFFF] and GT_OK for 255
        */
        index = 0xFFFF;

        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);


        index = 255;
        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);


        /*
            1.4. Call with wrong enum values enum meterColorMode
                           and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        index = 0;
        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerEntrySet
                            (dev, index, &entry, &params),
                            entry.meterColorMode);

        /*
            1.5. Call with tbParams.cir [0xFFFF] (no constraints in function's contract)
                   and other parameters the same as in 1.1.
            Expected: GT_OK.
        */
        entry.meterColorMode = CPSS_POLICER_COLOR_AWARE_E;
        entry.tbParams.cir   = 0xFFFF;

        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, entryPtr->tbParams->cir = %d",
                                     dev, entry.tbParams.cir);

        /*
            1.6. Call with tbParams.cbs [0xFFFF] (no constraints in function's contract)
                           and other parameters the same as in 1.1.
            Expected: GT_OK.
        */
        entry.tbParams.cir = 100;
        entry.tbParams.cbs = 0xFFFF;

        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, entryPtr->tbParams->cbs = %d",
                                     dev, entry.tbParams.cbs);

        /*
            1.7. Call with out of range counterEnable [GT_FALSE]
                           and counterSetIndex [100] (relevant when counterEnable == GT_TRUE)
                           and other parameters the same as in 1.1.
            Expected: NON GT_OK.
        */
        entry.tbParams.cbs    = 1000;
        entry.counterEnable   = GT_FALSE;
        entry.counterSetIndex = 100;

        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, entryPtr->counterEnable = %d, "
                                                    "entry->counterSetIndex = %d",
                                         dev, entry.counterEnable, entry.counterSetIndex);

        /*
            1.8. Call with out of range counterSetIndex [100],
                           counterEnable [GT_TRUE]
                           and other parameters the same as in 1.1.
            Expected: NON GT_OK.
        */
        entry.counterEnable   = GT_TRUE;
        entry.counterSetIndex = 100;

        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, entryPtr->counterEnable = %d, "
                                                    "entry->counterSetIndex = %d",
                                         dev, entry.counterEnable, entry.counterSetIndex);

        /*
            1.9. Call with wrong enum values enum cmd
                           and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        entry.counterSetIndex = 10;
        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerEntrySet
                            (dev, index, &entry, &params),
                            entry.cmd);

        /*
            1.10. Call with cmd [CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E]
                    and qosProfile [0xFFFF] (no constraints in function's contract)
                    and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        entry.cmd        = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
        entry.qosProfile = 0xFFFF;

        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "%d, entryPtr->cmd = %d, "
                                                "entryPtr->qosProfile = %d",
                                     dev, entry.cmd, entry.qosProfile);

        entry.qosProfile = 50;
        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, entryPtr->cmd = %d, "
                                                "entryPtr->qosProfile = %d",
                                     dev, entry.cmd, entry.qosProfile);
        /*
            1.11. Call with cmd [CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E]
                    and wrong enum values enum modifyUp
                    and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        entry.qosProfile = 10;
        entry.cmd        = CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerEntrySet
                            (dev, index, &entry, &params),
                            entry.modifyUp);

        /*
            1.12. Call with cmd [CPSS_DXCH_POLICER_CMD_NONE_E]
                            and wrong enum values enum modifyUp
                            and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        entry.cmd = CPSS_DXCH_POLICER_CMD_NONE_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerEntrySet
                            (dev, index, &entry, &params),
                            entry.modifyUp);

        /*
            1.13. Call with cmd [CPSS_DXCH_POLICER_CMD_NONE_E]
                            and wrong enum values enum modifyDscp
                            and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        entry.cmd        = CPSS_DXCH_POLICER_CMD_NONE_E;
        entry.modifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerEntrySet
                            (dev, index, &entry, &params),
                            entry.modifyDscp);

        /*
            1.14. Call with cmd [CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E]
                            and wrong enum values enum modifyDscp
                            and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        entry.cmd  = CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPolicerEntrySet
                            (dev, index, &entry, &params),
                            entry.modifyDscp);

        /*
            1.15. Call with entryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        entry.cmd        = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
        entry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

        st = cpssDxChPolicerEntrySet(dev, index, NULL, &params);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, entryPtr = NULL", dev);

        /*
            1.16. Call with tbParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPolicerEntrySet(dev, index, &entry, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tbParamsPtr = NULL", dev);

        /*
            1.17. Call cpssDxChPolicerEntryInvalidate with entryIndex [0] to invalidate changes.
            Expect: GT_OK.
        */
        st = cpssDxChPolicerEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChPolicerEntryInvalidate: %d, %d", dev, index);
    }

    index                 = 0;
    entry.policerEnable   = GT_TRUE;
    entry.meterColorMode  = CPSS_POLICER_COLOR_AWARE_E;
    entry.tbParams.cir    = 100;
    entry.tbParams.cbs    = 1000;
    entry.counterEnable   = GT_TRUE;
    entry.counterSetIndex = 10;
    entry.cmd             = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
    entry.qosProfile      = 10;
    entry.modifyDscp      = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    entry.modifyUp        = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerEntrySet(dev, index, &entry, &params);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_DXCH_POLICER_ENTRY_STC     *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerEntryGet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function cpssDxChPolicerEntrySet with
                       entryIndex [0],
                       entryPtr [{policerEnable = GT_TRUE,
                                  meterColorMode = PSS_POLICER_COLOR_AWARE_E,
                                  tbParams = {100, 1000},
                                  counterEnable = GT_TRUE,
                                  counterSetIndex = 10,
                                  cmd = CPSS_DXCH_POLICER_CMD_BY_ENTRY_E,
                                  qosProfile = 10,
                                  modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                  modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E}],
                        and non-NULL tbParamsPtr.
    Expected: GT_OK.
    1.2. Call this function with entryIndex [0]
                                 and non-NULL entryPtr.
    Expect: GT_OK.
    1.3. Call with entryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with entryIndex [0xFFFF]
                   and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call cpssDxChPolicerEntryInvalidate with entryIndex [0] to invalidate changes.
    Expect: GT_OK.
*/
    GT_STATUS                       st    = GT_OK;

    GT_U8                           dev;
    GT_U32                          index = 0;
    CPSS_DXCH_POLICER_ENTRY_STC     entry;
    CPSS_DXCH_POLICER_ENTRY_STC     setEntry;
    CPSS_DXCH_POLICER_TB_PARAMS_STC params;

    setEntry.policerEnable   = GT_TRUE;
    setEntry.meterColorMode  = CPSS_POLICER_COLOR_AWARE_E;
    setEntry.tbParams.cir    = 100;
    setEntry.tbParams.cbs    = 1000;
    setEntry.counterEnable   = GT_TRUE;
    setEntry.counterSetIndex = 10;
    setEntry.cmd             = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
    setEntry.qosProfile      = 10;
    setEntry.modifyDscp      = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    setEntry.modifyUp        = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function cpssDxChPolicerEntrySet with
                       entryIndex [0],
                       entryPtr [{policerEnable = GT_TRUE,
                                  meterColorMode = PSS_POLICER_COLOR_AWARE_E,
                                  tbParams = {100, 1000},
                                  counterEnable = GT_TRUE,
                                  counterSetIndex = 10,
                                  cmd = CPSS_DXCH_POLICER_CMD_BY_ENTRY_E,
                                  qosProfile = 10,
                                  modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                  modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E}],
                        and non-NULL tbParamsPtr.
            Expected: GT_OK.
        */
        index                 = 0;

        st = cpssDxChPolicerEntrySet(dev, index, &setEntry, &params);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerEntrySet: %d, %d", dev, index);

        /*
            1.2. Call this function with entryIndex [0]
                                 and non-NULL entryPtr.
            Expect: GT_OK.
        */
        st = cpssDxChPolicerEntryGet(dev, index, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.3. Call with entryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPolicerEntryGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, entryPtr = NULL", dev);

        /*
            1.4. Call with entryIndex [0xFFFF] (no constraints in function's contract)
                   and other parameters the same as in 1.1.
            Expected: GT_OK.
        */
        index = 0xFFFF;

        st = cpssDxChPolicerEntryGet(dev, index, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

        /*
            1.5. Call cpssDxChPolicerEntryInvalidate with entryIndex [0] to invalidate changes.
            Expect: GT_OK.
        */
        index = 0;

        st = cpssDxChPolicerEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerEntryInvalidate: %d, %d", dev, index);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerEntryGet(dev, index, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerEntryGet(dev, index, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerEntryInvalidate
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  entryIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerEntryInvalidate)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with entryIndex [0].
    Expect: GT_OK.
    1.2. Call with entryIndex [0xFFFF] and entryIndex [255]
    Expected: GT_BAD_PARAM for [0xFFFF] and GT_OK for entryIndex [255].
*/
    GT_STATUS  st    = GT_OK;

    GT_U8      dev;
    GT_U32     index = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with entryIndex [0].
            Expect: GT_OK.
        */
        index = 0;

        st = cpssDxChPolicerEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with entryIndex [0xFFFF]  and  entryIndex [255](no constraints in function's contract).
            Expected: GT_BAD_PARAM for entryIndex [0xFFFF] and GT_OK for entryIndex [255].
        */
        index = 0xFFFF;

        st = cpssDxChPolicerEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

        index = 255;
        st = cpssDxChPolicerEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerEntryInvalidate(dev, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_TB_PARAMS_STC        *tbInParamsPtr,
    OUT CPSS_DXCH_POLICER_TB_PARAMS_STC        *tbOutParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerEntryMeterParamsCalculate)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with tbInParamsPtr [{1000, 10000}]
                            and non-NULL tbOutParamsPtr.
    Expected: GT_OK.
    1.2. Call function with tbInParamsPtr [{0xFFFF, 10000}] (no constraints in function's contract)
                            and non-NULL tbOutParamsPtr.
    Expected: GT_OK.
    1.3. Call function with tbInParamsPtr [{1000, 0xFFFF}] (no constraints in function's contract)
                            and non-NULL tbOutParamsPtr.
    Expected: GT_OK.
    1.4. Call function with tbInParamsPtr [{1000, 10000}]
                            and tbOutParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call function with tbInParamsPtr [NULL]
                             and non-NULL tbOutParamsPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st    = GT_OK;

    GT_U8                           dev;
    CPSS_DXCH_POLICER_TB_PARAMS_STC inParams;
    CPSS_DXCH_POLICER_TB_PARAMS_STC outParams;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with tbInParamsPtr [{1000, 10000}]
                            and non-NULL tbOutParamsPtr.
            Expected: GT_OK.
        */
        inParams.cir = 1000;
        inParams.cbs = 10000;

        st = cpssDxChPolicerEntryMeterParamsCalculate(dev, &inParams, &outParams);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, tbInParamsPtr->cir = %d, "
                                                "tbInParamsPtr->cbs = %d",
                                     dev, inParams.cir, inParams.cbs);

        /*
            1.2. Call function with tbInParamsPtr [{0xFFFF, 10000}] (no constraints in function's contract)
                            and non-NULL tbOutParamsPtr.
            Expected: GT_OK.
        */
        inParams.cir = 0xFFFF;

        st = cpssDxChPolicerEntryMeterParamsCalculate(dev, &inParams, &outParams);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, tbInParamsPtr->cir = %d, "
                                                "tbInParamsPtr->cbs = %d",
                                     dev, inParams.cir, inParams.cbs);

        /*
            1.3. Call function with tbInParamsPtr [{1000, 0xFFFF}] (no constraints in function's contract)
                            and non-NULL tbOutParamsPtr.
            Expected: GT_OK.
        */
        inParams.cir = 1000;
        inParams.cbs = 0xFFFF;

        st = cpssDxChPolicerEntryMeterParamsCalculate(dev, &inParams, &outParams);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, tbInParamsPtr->cir = %d, "
                                                "tbInParamsPtr->cbs = %d",
                                     dev, inParams.cir, inParams.cbs);

        /*
            1.4. Call function with tbInParamsPtr [{1000, 10000}]
                            and tbOutParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        inParams.cbs = 10000;

        st = cpssDxChPolicerEntryMeterParamsCalculate(dev, &inParams, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tbOutParamsPtr = NULL", dev);

        /*
            1.5. Call function with tbInParamsPtr [NULL]
                                    and non-NULL tbOutParamsPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPolicerEntryMeterParamsCalculate(dev, NULL, &outParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tbInParamsPtr = NULL", dev);
    }

    inParams.cir = 1000;
    inParams.cbs = 10000;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerEntryMeterParamsCalculate(dev, &inParams, &outParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerEntryMeterParamsCalculate(dev, &inParams, &outParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerCountersGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            counterSetIndex,
    OUT CPSS_DXCH_POLICER_COUNTERS_STC    *countersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerCountersGet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with counterSetIndex [15]
                            and non-NULL countersPtr.
    Expected: GT_OK.
    1.2. Call function with out of range counterSetIndex [16]
                            and non-NULL countersPtr.
    Expected: NON GT_OK.
    1.3. Call function with counterSetIndex [0]
                            and countersPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                      st    = GT_OK;

    GT_U8                          dev;
    GT_U32                         index = 0;
    CPSS_DXCH_POLICER_COUNTERS_STC counters;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with counterSetIndex [15]
                            and non-NULL countersPtr.
            Expected: GT_OK.
        */
        index = 15;

        st = cpssDxChPolicerCountersGet(dev, index, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);


        /*
            1.2. Call function with out of range counterSetIndex [16]
                            and non-NULL countersPtr.
            Expected: NON GT_OK.
        */
        index = 16;

        st = cpssDxChPolicerCountersGet(dev, index, &counters);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.3. Call function with counterSetIndex [0]
                            and countersPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        index = 0;

        st = cpssDxChPolicerCountersGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, countersPtr = NULL", dev);
    }

    index = 15;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerCountersGet(dev, index, &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerCountersGet(dev, index, &counters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerCountersSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          counterSetIndex,
    IN  CPSS_DXCH_POLICER_COUNTERS_STC  *countersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerCountersSet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with counterSetIndex [15]
                            and countersPtr [{10000, 20000}].
    Expected: GT_OK.
    1.2. Call function cpssDxChPolicerCountersGet with counterSetIndex [15]
                                                  and non-NULL countersPtr.
    Expected: GT_OK and counters the same as were just set.
    1.3. Call function with out of range counterSetIndex [16]
                            and countersPtr [{10000, 20000}].
    Expected: NON GT_OK.
    1.4. Call function with counterSetIndex [0]
                            and countersPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call function with counterSetIndex [15]
                            and countersPtr [{0xFFFF, 20000}] (no constraints in contract).
    Expected: GT_OK.
    1.6. Call function with counterSetIndex [15]
                            and countersPtr [{10000, 0xFFFF}] (no constraints in contract).
    Expected: GT_OK.
*/
    GT_STATUS                      st    = GT_OK;

    GT_U8                          dev;
    GT_U32                         index = 0;
    CPSS_DXCH_POLICER_COUNTERS_STC counters;
    CPSS_DXCH_POLICER_COUNTERS_STC countersGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with counterSetIndex [15]
                            and countersPtr [{10000, 20000}].
            Expected: GT_OK.
        */
        index = 15;
        counters.outOfProfileBytesCnt = 10000;
        counters.inProfileBytesCnt    = 20000;

        st = cpssDxChPolicerCountersSet(dev, index, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call function cpssDxChPolicerCountersGet with counterSetIndex [15]
                                                               and non-NULL countersPtr.
            Expected: GT_OK and counters the same as were just set.
        */
        st = cpssDxChPolicerCountersGet(dev, index, &countersGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerCountersGet: %d, %d", dev, index);

        /* Verifying entryPtr */
        UTF_VERIFY_EQUAL1_STRING_MAC(counters.outOfProfileBytesCnt, countersGet.outOfProfileBytesCnt,
                       "get another counterSetIndex->outOfProfileBytesCnt than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(counters.inProfileBytesCnt, countersGet.inProfileBytesCnt,
                       "get another counterSetIndex->inProfileBytesCnt than was set: %d", dev);

        /*
            1.3. Call function with out of range counterSetIndex [16]
                            and countersPtr [{10000, 20000}].
            Expected: NON GT_OK.
        */
        index = 16;
        counters.outOfProfileBytesCnt = 10000;
        counters.inProfileBytesCnt = 20000;

        st = cpssDxChPolicerCountersSet(dev, index, &counters);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.4. Call function with counterSetIndex [0]
                                    and countersPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        index = 0;

        st = cpssDxChPolicerCountersSet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, countersPtr = NULL", dev);

        /*
            1.5. Call function with counterSetIndex [15]
                                    and countersPtr [{0xFFFF, 20000}] (no constraints in contract).
            Expected: GT_OK.
        */
        index = 15;
        counters.outOfProfileBytesCnt = 0xFFFF;

        st = cpssDxChPolicerCountersSet(dev, index, &counters);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, counterSetIndex->outOfProfileBytesCnt = %d, "
                                                "counterSetIndex->inOfProfileBytesCnt = %d",
                                     dev, counters.outOfProfileBytesCnt, counters.inProfileBytesCnt);

        /*
            1.6. Call function with counterSetIndex [15]
                            and countersPtr [{10000, 0xFFFF}] (no constraints in contract).
            Expected: GT_OK.
        */
        counters.outOfProfileBytesCnt = 10000;
        counters.inProfileBytesCnt    = 0xFFFF;

        st = cpssDxChPolicerCountersSet(dev, index, &counters);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, counterSetIndex->outOfProfileBytesCnt = %d, "
                                                "counterSetIndex->inOfProfileBytesCnt = %d",
                                     dev, counters.outOfProfileBytesCnt, counters.inProfileBytesCnt);
    }

    index = 15;
    counters.outOfProfileBytesCnt = 10000;
    counters.inProfileBytesCnt    = 20000;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerCountersSet(dev, index, &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerCountersSet(dev, index, &counters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill Policer table.
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerFillPolicerTable)
{
/*
    ITERATE_DEVICE (DxCh1, DxCh2)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_POLICER_METERS_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in Policer table.
         Call cpssDxChPolicerEntrySet with entryIndex [0... numEntries-1],
                                    entryPtr [{policerEnable = GT_TRUE,
                                               meterColorMode = PSS_POLICER_COLOR_AWARE_E,
                                               tbParams = {100, 1000},
                                               counterEnable = GT_TRUE,
                                               counterSetIndex = 10,
                                               cmd = CPSS_DXCH_POLICER_CMD_BY_ENTRY_E,
                                               qosProfile = 10,
                                               modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                               modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E }],
                                    and non-NULL tbParamsPtr.
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChPolicerEntrySet with entryIndex [numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in Policer table and compare with original.
         Call cpssDxChPolicerEntryGet with entryIndex
                                           and entryPtr is non-NULL.
    Expected: GT_OK and parameters the entry the same as were written (by fields - exclude tbParams, and take into attention that one fields depend on another).
    1.5. Try to read entry with index out of range.
         Call cpssDxChPolicerEntryGet with entryIndex [numEntries] and entryPtr is non-NULL.
    Expected: NOT GT_OK.
    1.6. Delete all entries in Policer table.
         Call cpssDxChPolicerEntryInvalidate with entryIndex as in 1.2 to invalidate entries.
    Expect: GT_OK.
    1.7. Try to delete entry with index out of range.
         Call cpssDxChPolicerEntryInvalidate with entryIndex [numEntries].
    Expect: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      numEntries = 0;
    GT_U16      iTemp      = 0;

    CPSS_DXCH_POLICER_ENTRY_STC     entry;
    CPSS_DXCH_POLICER_TB_PARAMS_STC params;
    CPSS_DXCH_POLICER_ENTRY_STC     entryGet;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &entry, sizeof(entry));
    cpssOsBzero((GT_VOID*) &entryGet, sizeof(entryGet));

    /* Fill the entry for Policer table */
    entry.policerEnable   = GT_TRUE;
    entry.meterColorMode  = CPSS_POLICER_COLOR_AWARE_E;
    entry.tbParams.cir    = 100;
    entry.tbParams.cbs    = 1000;
    entry.counterEnable   = GT_TRUE;
    entry.counterSetIndex = 10;
    entry.cmd             = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
    entry.qosProfile      = 10;
    entry.modifyDscp      = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    entry.modifyUp        = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_POLICER_METERS_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        /* 1.2. Fill all entries in Policer table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            /* make every entry unique */
            entry.counterSetIndex = iTemp % 15;

            st = cpssDxChPolicerEntrySet(dev, iTemp, &entry, &params);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerEntrySet: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChPolicerEntrySet(dev, numEntries, &entry, &params);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerEntrySet: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in vlan table and compare with original */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            /* restore unique entry before compare */
            entry.counterSetIndex = iTemp % 15;

            st = cpssDxChPolicerEntryGet(dev, iTemp, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChPolicerEntryGet: %d, %d", dev, iTemp);

            /* Verifying entryPtr */
            UTF_VERIFY_EQUAL1_STRING_MAC(entry.policerEnable, entryGet.policerEnable,
                           "get another entryPtr->policerEnable than was set: %d", dev);
            if (GT_TRUE == entry.policerEnable)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.meterColorMode, entryGet.meterColorMode,
                               "get another entryPtr->meterColorMode than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.counterEnable, entryGet.counterEnable,
                               "get another entryPtr->counterEnable than was set: %d", dev);
                if (GT_TRUE == entry.counterEnable)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(entry.counterSetIndex, entryGet.counterSetIndex,
                                   "get another entryPtr->counterSetIndex than was set: %d", dev);
                }
                UTF_VERIFY_EQUAL1_STRING_MAC(entry.cmd, entryGet.cmd,
                               "get another entryPtr->cmd than was set: %d", dev);
                if (CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E == entry.cmd)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(entry.qosProfile, entryGet.qosProfile,
                                   "get another entryPtr->qosProfile than was set: %d", dev);
                }
                if ((CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E == entry.cmd) ||
                    (CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E == entry.cmd))
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(entry.modifyDscp, entryGet.modifyDscp,
                                   "get another entryPtr->modifyDscp than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(entry.modifyUp, entryGet.modifyUp,
                                   "get another entryPtr->modifyUp than was set: %d", dev);
                }
            }
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChPolicerEntryGet(dev, numEntries, &entryGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerEntryGet: %d, %d", dev, numEntries);

        /* 1.6. Delete all entries in vlan table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxChPolicerEntryInvalidate(dev, iTemp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerEntryInvalidate: %d, %d", dev, iTemp);
        }

        /* 1.7. Try to delete entry with index out of range. */
        st = cpssDxChPolicerEntryInvalidate(dev, numEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPolicerEntryInvalidate: %d, %d", dev, numEntries);

    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerInit
(
    IN GT_U8                                    devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerInit)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with not null dev.
    Expected: GT_OK.
*/
    GT_STATUS                         st   = GT_OK;
    GT_U8                             dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not null dev.
            Expected: GT_OK.
        */

        st = cpssDxChPolicerInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPolicer suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPolicer)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerPacketSizeModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerDropRedModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerPacketSizeModeForTunnelTermSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerTokenBucketModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerEntryInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerEntryMeterParamsCalculate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerCountersSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerInit)
    /* Test filling Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerFillPolicerTable)
UTF_SUIT_END_TESTS_MAC(cpssDxChPolicer)

