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
* @file cpssDxChPortSyncEtherUT.c
*
* @brief Unit tests for cpssDxChPortSyncEther, that provides
* CPSS implementation for Sync-E (Synchronious Ethernet)
*
* @version   26
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSyncEther.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/* defines */

/* Default valid value for port id */
#define PORT_SE_VALID_PHY_PORT_CNS  0

/* Default valid value for lane num */
#define PORT_SE_VALID_PHY_LANE_CNS  0

/* Macro to verify XG/GE ports */
#define UTF_VERIFY_XG_GE_PARAM_MAC(rc, devNum, portNum, lionExpectedRc)             \
    if((CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)||         \
       (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)||        \
       (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)||        \
       (PRV_CPSS_SIP_5_CHECK_MAC(dev)))       \
    {                                                                               \
        UTF_VERIFY_EQUAL2_PARAM_MAC(lionExpectedRc, rc, devNum, portNum);           \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        switch (portNum)                                                            \
        {                                                                           \
            case 0:                                                                 \
            case 4:                                                                 \
            case 8:                                                                 \
            case 12:                                                                \
            case 16:                                                                \
            case 20:                                                                \
            case 24:                                                                \
            case 25:                                                                \
            case 26:                                                                \
            case 27:                                                                \
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, portNum);        \
                    break;                                                          \
            default:                                                                \
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, rc, devNum, portNum); \
        }\
    }

static GT_STATUS prvUtGetPortLaneNum(IN GT_U8 devNum, IN GT_PHYSICAL_PORT_NUM portNum,
                                     OUT GT_U32 *maxlaneNumPtr);

/* DUE to wrong port identification go only over pair ports */
/* and stop SERDES up\down status set fail problem!!!       */
/* NOTE : QSGMII ports are valid on 0,4,8... */
#define DEBUG_SKIP_ODD_PORT(_port, _ifMode) \
        if((0 != (_port%4)) && (_ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)) \
            _ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;                         \
        if( 0 != _port%2 )                                                   \
            _ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherSecondRecoveryClkEnableSet(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherSecondRecoveryClkEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1. Call with enable [GT_FALSE/ GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChPortSyncEtherSecondRecoveryClkEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    CPSS_TBD_BOOKMARK /* UTF_XCAT2_E should be removed from the check, temporarily checked due to problems in prvCpssDxChPortInterfaceModeGet and similars */
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable = GT_FALSE and GT_TRUE.   */
        /* Expected: GT_OK.                                             */

        /*call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call cpssDxChPortSyncEtherSecondRecoveryClkEnableGet.   */
        /* Expected: GT_OK and the same value.                          */
        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*checking value */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                  "got another enable than was set: %d", dev);

        /*call with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*checking value */
        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                  "got another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE    */

    st = cpssDxChPortSyncEtherSecondRecoveryClkEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherSecondRecoveryClkEnableGet(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherSecondRecoveryClkEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1. Call function for with non null enable pointer.
    Expected: GT_OK.
    1.2. Call function for with null enable pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with non null enable pointer.   */
        /* Expected: GT_OK.                                             */

        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableGet(dev, &enable);
        if( GT_BAD_STATE == st )
        {
            st = GT_OK;
        }
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call function for with null enable pointer [NULL].      */
        /* Expected: GT_BAD_PTR.                                        */
        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherSecondRecoveryClkEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherSecondRecoveryClkEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherPllRefClkSelectSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_TYPE_ENT pllClkSelect
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherPllRefClkSelectSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with pllClkSelect
                     [CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_OSCILLATOR_E /
                      CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_RECOVERED_E]
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSyncEtherPllRefClkSelectGet.
    Expected: GT_OK and the same value.
    1.1.3. Call with wrong enum values pllClkSelect.
    Expected: GT_OK and the same value.
*/
    GT_STATUS                                      st       = GT_OK;
    GT_U8                                          dev;
    GT_PHYSICAL_PORT_NUM                           port;
    CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_TYPE_ENT pllClkSelect;
    CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_TYPE_ENT pllClkSelectGet;
    CPSS_PORT_INTERFACE_MODE_ENT                   ifMode;

    /*UTF_LION_NOT_SUPPORT_TEST_MAC(prvTgfDevNum);*/

    pllClkSelect    = CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_OSCILLATOR_E;
    pllClkSelectGet = CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_OSCILLATOR_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_MII_E == ifMode) )
            {
                continue;
            }

            /*
                1.1.1. Call with pllClkSelect =
                         [CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_OSCILLATOR_E /
                          CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_RECOVERED_E]
                Expected: GT_OK.
            */

            /*Call with pllClkSelect [CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_OSCILLATOR_E]*/
            pllClkSelect = CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_OSCILLATOR_E;

            st = cpssDxChPortSyncEtherPllRefClkSelectSet(dev, port, pllClkSelect);
            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChPortSyncEtherPllRefClkSelectGet.
                    Expected: GT_OK and the same value.
                */

                st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, &pllClkSelectGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, pllClkSelect);
                UTF_VERIFY_EQUAL1_STRING_MAC(pllClkSelect, pllClkSelectGet,
                                        "got another pllClkSelect than was set: %d", dev);
            }

            /*Call with pllClkSelect [CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_RECOVERED_E]*/
            pllClkSelect = CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_RECOVERED_E;

            st = cpssDxChPortSyncEtherPllRefClkSelectSet(dev, port, pllClkSelect);
            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChPortSyncEtherPllRefClkSelectGet.
                    Expected: GT_OK and the same value.
                */

                st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, &pllClkSelectGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, pllClkSelect);
                UTF_VERIFY_EQUAL1_STRING_MAC(pllClkSelect, pllClkSelectGet,
                                        "got another pllClkSelect than was set: %d", dev);
            }

            /*
                1.1.3. Call with wrong enum values pllClkSelect.
                Expected: GT_OK and the same value.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortSyncEtherPllRefClkSelectSet
                                (dev, port, pllClkSelect),
                                pllClkSelect);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */

            st = cpssDxChPortSyncEtherPllRefClkSelectSet(dev, port, pllClkSelect);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSyncEtherPllRefClkSelectSet(dev, port, pllClkSelect);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    pllClkSelect = CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_RECOVERED_E;
    port         = PORT_SE_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherPllRefClkSelectSet(dev, port, pllClkSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherPllRefClkSelectSet(dev, port, pllClkSelect);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherPllRefClkSelectGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_TYPE_ENT *pllClkSelectPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherPllRefClkSelectGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call function for with non null pllClkSelect pointer.
    Expected: GT_OK.
    1.1.2. Call function for with null pllClkSelect pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                      st       = GT_OK;

    GT_U8                                          dev;
    GT_PHYSICAL_PORT_NUM                           port;
    CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_TYPE_ENT pllClkSelect;
    CPSS_PORT_INTERFACE_MODE_ENT                   ifMode;

    /*UTF_LION_NOT_SUPPORT_TEST_MAC(prvTgfDevNum);*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_MII_E == ifMode) )
            {
                continue;
            }

            /*
                1.1.1. Call function for with non null pllClkSelect pointer.
                Expected: GT_OK.
            */
            st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, &pllClkSelect);
            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            /*
                1.1.2. Call function for with null pllClkSelect pointer [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, &pllClkSelect);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, &pllClkSelect);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_SE_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, &pllClkSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherPllRefClkSelectGet(dev, port, &pllClkSelect);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_U8   portNum,
    IN  GT_U32  laneNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveryClkConfigSet)
{
/*
    ITERATE_DEVICES (xCat):
    1.1.1. Call with recoveryClkType = [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E /
                                        CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E],
                          and enable = [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSyncEtherRecoveryClkConfigGet.
    Expected: GT_OK and the same value.
    1.1.3. Call wrong wnum values recoveryClkType.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with recoveryClkType = [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E /
                                        CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E].
    Expected: GT_BAD_PARAM.
    ITERATE_DEVICES (Lion):
    1.1.1. Call with recoveryClkType according to portGroup for port
                           and lane = [0 / maxLanePerPortIfMode ]
                           and enable = [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSyncEtherRecoveryClkConfigGet.
    Expected: GT_OK and the same value.
    1.1.3 While port and lane are enabled in 1.1.1. call another enabling in the same portGroup
    Expected: GT_BAD_STATE.
    1.1.4 Call with maxLanePerPortIfMode+1
    Expected: GT_BAD_PARAM.
    1.1.5. Call wrong wnum values recoveryClkType.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with recoveryClkType which does not fit to the portGroup of the port.
    Expected: GT_BAD_PARAM.

    ITERATE_DEVICES (Lion2):
    1.1.1. Call with recoveryClkType according to portGroup for port
                           and lane = [0 / maxLanePerPortIfMode ]
                           and enable = [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSyncEtherRecoveryClkConfigGet.
    Expected: GT_OK and the same value.
    1.1.3 While port and lane are enabled in 1.1.1. call another enabling in the same portGroup
    Expected: GT_BAD_STATE.
    1.1.4 Call with maxLanePerPortIfMode+1
    Expected: GT_BAD_PARAM.
    1.1.5. Call wrong wnum values recoveryClkType.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with recoveryClkType which does not fit to the portGroup of the port.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS  st        = GT_OK;
    GT_U8      dev;

    GT_PHYSICAL_PORT_NUM port      = PORT_SE_VALID_PHY_PORT_CNS ;
    GT_PHYSICAL_PORT_NUM testPort  = PORT_SE_VALID_PHY_PORT_CNS ;
    GT_PHYSICAL_PORT_NUM      portGet   = 1;
    GT_U32     lane      = PORT_SE_VALID_PHY_LANE_CNS;
    GT_U32     testLane  = PORT_SE_VALID_PHY_LANE_CNS;
    GT_U32     laneGet   = 0;
    GT_BOOL    enable    = GT_TRUE;
    GT_BOOL    enableGet = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT            speed;
    GT_U32     portGroup;  /* port group */

    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT recoveryClkType =
                                                        CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;

    /*UTF_LION_NOT_SUPPORT_TEST_MAC(prvTgfDevNum);*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E | UTF_CPSS_PP_E_ARCH_CNS);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E , CPSS-6062);
    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_CAELUM_E, CPSS-6064);
    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6065);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with recoveryClkType =
                                    [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E /
                                     CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E],
                       and enable = [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */

            /*Call with recoveryClkType [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E]
                                         and enable [GT_TRUE] */
            recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;
            enable          = GT_TRUE;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                                 recoveryClkType, enable, port, lane);
                UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_BAD_PARAM);
                continue;
            }

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChPortSyncEtherRecoveryClkConfigGet.
                    Expected: GT_OK and the same value.
                */

                    st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);

                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port,
                                                           enable, recoveryClkType);
                    UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                        "got another enable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(port, portGet,
                                          "got another port than was set: %d", dev);
            }

            /*Call with recoveryClkType [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E]
                                         and enable [GT_FALSE] */
            recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E;
            enable          = GT_FALSE;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);

            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChPortSyncEtherRecoveryClkConfigGet.
                    Expected: GT_OK and the same value.
                */

                    st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);

                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port,
                                                           enable, recoveryClkType);
                    UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                        "got another enable than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(port, portGet,
                                          "got another port than was set: %d", dev);
            }

            /*
                1.1.3. Call wrong enum values recoveryClkType.
                Expected: GT_BAD_PARAM.
            */
            enable = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChPortSyncEtherRecoveryClkConfigSet
                                (dev, recoveryClkType, enable, port, 0),
                                recoveryClkType);

            /*
                1.1.4. Call with recoveryClkType = [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E /
                                                    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E].
                Expected: GT_BAD_PARAM.
            */
            recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                     recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                     recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
    }

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode) )
            {
                continue;
            }

            st = cpssDxChPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with recoveryClkType according to portGroup for port
                           and lane = [0 / maxLanePerPortIfMode ]
                           and enable = [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */

            portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(dev, port);
            switch(portGroup)
            {
                case 0: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;
                        break;
                case 1: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E;
                        break;
                case 2: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E;
                        break;
                case 3: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E;
                        break;
                case 4: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK4_E;
                        break;
                case 5: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK5_E;
                        break;
                case 6: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK6_E;
                        break;
                case 7: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK7_E;
                        break;
                default: UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
                         break;
            }

            lane   = PORT_SE_VALID_PHY_LANE_CNS;
            enable = GT_TRUE;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                                 recoveryClkType, enable, port, lane);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, port, lane);
                continue;
            }

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            /*
                1.1.2. Call cpssDxChPortSyncEtherRecoveryClkConfigGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            if(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC((port&0xFFFFFFFC), portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(port, portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            UTF_VERIFY_EQUAL5_STRING_MAC(lane, laneGet, "got another lane than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            /*
                1.1.3 While port and lane are enabled in 1.1.1. call another enabling in the same portGroup
                Expected: GT_BAD_STATE.
            */

            switch(ifMode)
            {
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:

                    switch(speed)
                    {
                        case CPSS_PORT_SPEED_10000_E:
                            testPort = port + 1;
                            testLane = lane;
                            break;

                        default:
                            testPort = port;
                            testLane = lane + 1;
                            break;
                    }
                    break;
                case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
                case CPSS_PORT_INTERFACE_MODE_SGMII_E:
                    testPort = port + 1;
                    testLane = lane;
                    break;

                case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
                    if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(dev)->devFamily)
                    {
                        testPort = (port + 4) % 24;
                    }
                    else
                    {
                        testPort = (port + 4) % 48;
                    }
                    testLane = lane;
                    break;

                case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
                case CPSS_PORT_INTERFACE_MODE_HX_E:
                case CPSS_PORT_INTERFACE_MODE_XGMII_E:
                    testPort = port;
                    testLane = lane + 1;
                    break;
                default:
                    break;
            }
            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, testPort, (GT_U8)(testLane));
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                GT_U32 portMacNum;

                st = prvCpssDxChPortPhysicalPortMapCheckAndConvert(dev, testPort, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
                if(GT_OK != st)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, testPort, testLane);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_STATE, st, dev, recoveryClkType, enable, testPort, testLane);
            }

            enable = GT_FALSE;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            st = prvUtGetPortLaneNum(dev, port, &lane);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);

            enable = GT_TRUE;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            if(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC((port&0xFFFFFFFC), portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(port, portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            UTF_VERIFY_EQUAL5_STRING_MAC(lane, laneGet, "got another lane than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            enable = GT_FALSE;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            /*
                1.1.4 Call with maxLanePerPortIfMode+1
                Expected: GT_BAD_PARAM.
            */

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, (GT_U8)(lane+1));
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, port, lane+1);

            /*
                1.1.5. Call wrong enum values recoveryClkType.
                Expected: GT_BAD_PARAM.
            */
            enable = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChPortSyncEtherRecoveryClkConfigSet
                                (dev, recoveryClkType, enable, port, lane),
                                recoveryClkType);

            /*
                1.1.6. Call with recoveryClkType which does not fit to the portGroup of the port.
                Expected: GT_BAD_PARAM.
            */

            recoveryClkType = (CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT)((recoveryClkType+2)%8);

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, port, lane);
        }
    }

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode) )
            {
                continue;
            }

            st = cpssDxChPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with recoveryClkType that is NOT EQUAL to portGroup
                Expected: GT_OK.
            */

            portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(dev, port);
            switch(portGroup)
            {
                case 0: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E;
                        break;
                case 1: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;
                        break;
                case 2: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E;
                        break;
                case 3: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E;
                        break;
                case 4: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK5_E;
                        break;
                case 5: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK4_E;
                        break;
                case 6: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK7_E;
                        break;
                case 7: recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK6_E;
                        break;
                default: UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
                         break;
            }

            lane   = PORT_SE_VALID_PHY_LANE_CNS;
            enable = GT_TRUE;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                                 recoveryClkType, enable, port, lane);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, port, lane);
                continue;
            }

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            /*
                1.1.2. Call cpssDxChPortSyncEtherRecoveryClkConfigGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            if(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC((port&0xFFFFFFFC), portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(port, portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            UTF_VERIFY_EQUAL5_STRING_MAC(lane, laneGet, "got another lane than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            /*
                1.1.3 While port and lane are enabled in 1.1.1. call another enabling in the same portGroup
                Expected: GT_BAD_STATE.
            */

            switch(ifMode)
            {
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:

                    switch(speed)
                    {
                        case CPSS_PORT_SPEED_10000_E:
                            testPort = port + 1;
                            testLane = lane;
                            break;

                        default:
                            testPort = port;
                            testLane = lane + 1;
                            break;
                    }
                    break;
                case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
                case CPSS_PORT_INTERFACE_MODE_SGMII_E:
                    testPort = port + 1;
                    testLane = lane;
                    break;

                case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
                    if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(dev)->devFamily)
                    {
                        testPort = (port + 4) % 24;
                    }
                    else
                    {
                        testPort = (port + 4) % 48;
                    }
                    testLane = lane;
                    break;

                case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
                case CPSS_PORT_INTERFACE_MODE_HX_E:
                case CPSS_PORT_INTERFACE_MODE_XGMII_E:
                    testPort = port;
                    testLane = lane + 1;
                    break;
                default:
                    break;
            }
            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, testPort, (GT_U8)(testLane));
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                GT_U32 portMacNum;

                st = prvCpssDxChPortPhysicalPortMapCheckAndConvert(dev, testPort, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
                if(GT_OK != st)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, testPort, testLane);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_STATE, st, dev, recoveryClkType, enable, testPort, testLane);
            }


            enable = GT_FALSE;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            st = prvUtGetPortLaneNum(dev, port, &lane);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);

            enable = GT_TRUE;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            if(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC((port&0xFFFFFFFC), portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(port, portGet, "got another port than was set: %d, %d, %d, %d, %d",
                                             dev, recoveryClkType, enable, port, lane);
            }
            UTF_VERIFY_EQUAL5_STRING_MAC(lane, laneGet, "got another lane than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            enable = GT_FALSE;

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, recoveryClkType, enable, port, lane);

            st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                             recoveryClkType, &enableGet, &portGet, &laneGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkConfigGet: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_STRING_MAC(enable, enableGet, "got another enable than was set: %d, %d, %d, %d, %d",
                                         dev, recoveryClkType, enable, port, lane);

            /*
                1.1.4 Call with maxLanePerPortIfMode+1
                Expected: GT_BAD_PARAM.
            */

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, (GT_U8)(lane+1));
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, port, lane+1);

            /*
                1.1.5. Call wrong enum values recoveryClkType.
                Expected: GT_BAD_PARAM.
            */
            enable = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChPortSyncEtherRecoveryClkConfigSet
                                (dev, recoveryClkType, enable, port, lane),
                                recoveryClkType);

            /*
                1.1.6. Call with recoveryClkType which does not fit to the portGroup of the port.
                Expected: GT_BAD_PARAM.
            */

            recoveryClkType = (CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT)((recoveryClkType+2)%8);

            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, recoveryClkType, enable, port, lane);
        }
    }

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;
    lane            = PORT_SE_VALID_PHY_LANE_CNS;

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                             recoveryClkType, enable, port, lane);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                         recoveryClkType, enable, port, lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_SE_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                         recoveryClkType, enable, port, lane);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherRecoveryClkConfigSet(dev,
                                     recoveryClkType, enable, port, lane);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U8    *portNumPtr,
    OUT GT_U32   *laneNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveryClkConfigGet)
{
/*
    1.1.1. Call function for with non null enable and portNum pointer.
    Expected: GT_OK.
    1.1.2. Call function for with wrong enum values recoveryClkType.
    Expected: GT_BAD_PARAM.
    1.1.3. Call function for with null enable pointer [NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call function for with null portNum pointer [NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call function for with null laneNum pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st       = GT_OK;
    GT_U8                                           dev;
    GT_PHYSICAL_PORT_NUM                            port;
    GT_U32                                          lane;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT recoveryClkType;
    GT_BOOL                                         enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function for with non null enable and portNum pointer.
            Expected: GT_OK.
        */

        /*Call with
          recoveryClkType [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E]*/
        recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;

        st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, recoveryClkType);

        /*Call with
          recoveryClkType [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E]*/
        recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E;

        st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, recoveryClkType);

        /*
            1.1.2. Call function for with wrong enum values recoveryClkType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortSyncEtherRecoveryClkConfigGet
                            (dev, recoveryClkType, &enable, &port, &lane),
                            recoveryClkType);

        /*
            1.1.3. Call function for with null enable pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, NULL, &port, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, recoveryClkType);

        /*
            1.1.4. Call function for with null port pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, NULL, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, recoveryClkType);

        /*
            1.1.5. Call function for with null lane pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, recoveryClkType);
    }

    recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, &lane);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherRecoveryClkConfigGet(dev,
                             recoveryClkType, &enable, &port, &lane);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1. Call with enable [GT_FALSE/ GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable = GT_FALSE and GT_TRUE.   */
        /* Expected: GT_OK.                                             */
        enable = GT_FALSE;

        /*call with enable = GT_FALSE */
        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet.   */
        /* Expected: GT_OK and the same value.                          */

        enable = GT_FALSE;

        /*call with enable = GT_FALSE */
        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*checking value */
        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet(dev, &enableGet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                  "got another enable than was set: %d", dev);

        /*call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*checking value */
        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet(dev, &enableGet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                  "got another enable than was set: %d", dev);
        enable = GT_TRUE;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE    */

    st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1. Call function for with non null enable pointer.
    Expected: GT_OK.
    1.2. Call function for with null enable pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with non null enable pointer.   */
        /* Expected: GT_OK.                                             */

        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call function for with null enable pointer [NULL].      */
        /* Expected: GT_BAD_PTR.                                        */
        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveryClkDividerValueSet)
{
    /*
    ITERATE_DEVICES (Lion and above)
    1.1.1. Call with lane [0 \ maxLaneOnPort ]
                and value [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E \
                           CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E]
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSyncEtherRecoveryClkDividerValueGet
                `       with non-NULL valueGet and other parameters from 1.1.1
    Expected: GT_OK and same valueGet as written by value
    1.1.3. Call with lane [maxLaneOnPort+1]
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong enum values value.
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8                                               dev;
    GT_PHYSICAL_PORT_NUM                                port = PORT_SE_VALID_PHY_PORT_CNS;
    GT_U32                                              lane = PORT_SE_VALID_PHY_LANE_CNS;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value =
                                                            CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  valueGet;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect =
                                                            CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E;
    CPSS_PORT_INTERFACE_MODE_ENT                        ifMode;
    CPSS_PP_FAMILY_TYPE_ENT                             devFamily;

    /* there is no Sync Ethernet in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode) )
            {
                continue;
            }

            /*
               1.1.1. Call with lane [0 \ maxLaneOnPort ]
                and value [CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E \
                           CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E]
               Expected: GT_OK.
            */

            lane = PORT_SE_VALID_PHY_LANE_CNS;
            value = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
                UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_BAD_PARAM);
                continue;
            }

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
            if(CPSS_PORT_INTERFACE_MODE_MII_E == ifMode)
            {
                /* the FE port is not supported */
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, lane, value);
                continue;
            }

            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if( GT_BAD_PARAM == st )
                continue;

            /*
                1.1.2. Call cpssDxChPortSyncEtherRecoveryClkDividerValueGet
                `       with non-NULL valueGet and other parameters from 1.1.1
                Expected: GT_OK and same valueGet as written by value
            */

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &valueGet);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkDividerValueGet: %d %d %d %d",
                                         dev, port, lane, clockSelect);
            UTF_VERIFY_EQUAL4_STRING_MAC(value, valueGet, "got another value than was set: %d %d %d %d",
                                         dev, port, lane, clockSelect);

            st = prvUtGetPortLaneNum(dev, port, &lane);
            if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E) ||
               (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);
            }

            value = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E;

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, lane, clockSelect, value);

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &valueGet);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveryClkDividerValueGet: %d %d %d %d",
                                         dev, port, lane, clockSelect);
            UTF_VERIFY_EQUAL4_STRING_MAC(value, valueGet, "got another value than was set: %d %d %d %d",
                                         dev, port, lane, clockSelect);


            /*
               1.1.3. with lane [maxLaneOnPort+1]
               Expected: GT_BAD_PARAM.
            */

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, (GT_U8)(lane+1), clockSelect, value);
            if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
            {
                /* device ignores laneNum parameter */
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane+1, value);
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane+1, value);
            }


            /*
               1.1.4. Call with wrong enum values value.
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortSyncEtherRecoveryClkDividerValueSet
                                (dev, port, lane, clockSelect, value),
                                value);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect, value);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect, value);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_SE_VALID_PHY_PORT_CNS;
    lane = PORT_SE_VALID_PHY_LANE_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveryClkDividerValueGet)
{
    /*
    ITERATE_DEVICES (Lion and above)
    1.1.1. Call with lane [0 \ maxLaneOnPort ] and non-NULL valuePtr.
    Expected: GT_OK.
    1.1.2. Call with lane [maxLaneOnPort+1] and non-NULL valuePtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with NULL valuePtr.
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8                                               dev;
    GT_PHYSICAL_PORT_NUM                                port = PORT_SE_VALID_PHY_PORT_CNS;
    GT_U32                                              lane = PORT_SE_VALID_PHY_LANE_CNS;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value;
    CPSS_PORT_INTERFACE_MODE_ENT                        ifMode;
    CPSS_PP_FAMILY_TYPE_ENT                             devFamily;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E;

    /* there is no Sync Ethernet in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode) )
            {
                continue;
            }

            /*
                1.1.1. Call with lane [0 \ maxLaneOnPort ] and non-NULL valuePtr.
                Expected: GT_OK.
            */

            lane = PORT_SE_VALID_PHY_LANE_CNS;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
                UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_BAD_PARAM);
                continue;
            }

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
            if(CPSS_PORT_INTERFACE_MODE_MII_E == ifMode)
            {
                /* the FE port is not supported */
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, lane, clockSelect, value);
                continue;
            }

            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if( GT_BAD_PARAM == st )
                continue;

            st = prvUtGetPortLaneNum(dev, port, &lane);
            if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
               (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);
            }

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, clockSelect);

            /*
                1.1.2. Call with lane [maxLaneOnPort+1] and non-NULL valuePtr.
                Expected: GT_BAD_PARAM.
            */

            st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, (GT_U8)(lane+1), clockSelect, &value);
            if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
            {
                /* device ignores laneNum parameter */
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, clockSelect);
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);
            }

            /*
                1.1.3. Call with NULL valuePtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, lane);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_SE_VALID_PHY_PORT_CNS;
    lane = PORT_SE_VALID_PHY_LANE_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet)
{
    /*
    ITERATE_DEVICES (Lion and above)
    1.1.1. Call with lane [0 \ maxLaneOnPort ]
                and enable [GT_TRUE \ GT_FALSE]
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet
                `       with non-NULL enableGet and other parameters from 1.1.1
    Expected: GT_OK and same enableGet as written by enable
    1.1.3. Call with lane [maxLaneOnPort+1]
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8                           dev;
    GT_PHYSICAL_PORT_NUM            port = PORT_SE_VALID_PHY_PORT_CNS;
    GT_U32                          lane = PORT_SE_VALID_PHY_LANE_CNS;
    GT_BOOL                         enable = GT_TRUE;
    GT_BOOL                         enableGet;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E;

    /* there is no Sync Ethernet in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode) )
            {
                continue;
            }

            /*
                1.1.1. Call with lane [0 \ maxLaneOnPort ]
                and enable [GT_TRUE \ GT_FALSE]
                Expected: GT_OK.
            */

            lane = PORT_SE_VALID_PHY_LANE_CNS;
            enable = GT_TRUE;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, lane, clockSelect, enable);
                UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_BAD_PARAM);
                continue;
            }

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, lane, clockSelect, enable);
            if(CPSS_PORT_INTERFACE_MODE_MII_E == ifMode)
            {
                /* the FE port is not supported */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, lane);
                continue;
            }

            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if( GT_BAD_PARAM == st )
                continue;

            /*
                1.1.2. Call cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet
                `       with non-NULL enableGet and other parameters from 1.1.1
                Expected: GT_OK and same enableGet as written by enable
            */

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enableGet);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet: %d %d %d %d",
                                         dev, port, lane, clockSelect);
            UTF_VERIFY_EQUAL4_STRING_MAC(enable, enableGet, "got another value than was set: %d %d %d %d",
                                         dev, port, lane, clockSelect);

            st = prvUtGetPortLaneNum(dev, port, &lane);
            if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
               (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);
            }

            enable = GT_FALSE;

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, lane, clockSelect, enable);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, lane, clockSelect, enable);

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enableGet);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet: %d %d %d %d",
                                         dev, port, lane, clockSelect);
            UTF_VERIFY_EQUAL4_STRING_MAC(enable, enableGet, "got another value than was set: %d %d %d %d",
                                         dev, port, lane, clockSelect);

            /*
                1.1.3. Call with lane [maxLaneOnPort+1]
                Expected: GT_BAD_PARAM.
            */

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, (GT_U8)(lane+1), clockSelect, enable);
            if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
            {
                /* device ignores laneNum parameter */
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, lane+1, clockSelect, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane+1, clockSelect, enable);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, lane, clockSelect, enable);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect, enable);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, lane, clockSelect, enable);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_SE_VALID_PHY_PORT_CNS;
    lane = PORT_SE_VALID_PHY_LANE_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, lane, clockSelect, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(dev, port, lane, clockSelect, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet)
{
    /*
    ITERATE_DEVICES (Lion and above)
    1.1.1. Call with lane [0 \ maxLaneOnPort ] and non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with lane [maxLaneOnPort+1] and non-NULL enablePtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8                           dev;
    GT_PHYSICAL_PORT_NUM            port = PORT_SE_VALID_PHY_PORT_CNS;
    GT_U32                          lane = PORT_SE_VALID_PHY_LANE_CNS;
    GT_BOOL                         enable;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E;

    /* there is no Sync Ethernet in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            DEBUG_SKIP_ODD_PORT(port,ifMode)
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode) )
            {
                continue;
            }

            /*
                1.1.1. Call with lane [0 \ maxLaneOnPort ] and non-NULL enablePtr.
                Expected: GT_OK.
            */

            lane = PORT_SE_VALID_PHY_LANE_CNS;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enable);
                UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_BAD_PARAM);
                continue;
            }

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enable);
            if(CPSS_PORT_INTERFACE_MODE_MII_E == ifMode)
            {
                /* the FE port is not supported */
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, lane, enable);
                continue;
            }

            UTF_VERIFY_XG_GE_PARAM_MAC(st, dev, port, GT_OK);

            if( GT_BAD_PARAM == st )
                continue;

            st = prvUtGetPortLaneNum(dev, port, &lane);
            if((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
               (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);
            }

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, clockSelect);

            /*
                1.1.2. Call with lane [maxLaneOnPort+1] and non-NULL enablePtr.
                Expected: GT_BAD_PARAM.
            */

            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, (GT_U8)(lane+1), clockSelect, &enable);
            if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
            {
                /* device ignores laneNum parameter */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            }

            /*
                1.1.3. Call with NULL enablePtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, NULL);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, port, lane, clockSelect);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_SE_VALID_PHY_PORT_CNS;
    lane = PORT_SE_VALID_PHY_LANE_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(dev, port, lane, clockSelect, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortSyncEther suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortSyncEther)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherSecondRecoveryClkEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherSecondRecoveryClkEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherPllRefClkSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherPllRefClkSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveryClkConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveryClkConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveryClkDividerBypassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveryClkDividerValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveryClkDividerValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortSyncEther)

/**
* @internal prvUtGetPortLaneNum function
* @endinternal
*
* @brief   Gets the nuber of lanes for port (based on port interface mode).
*
* @note   APPLICABLE DEVICES:      Lion
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum or no lanes on port
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvUtGetPortLaneNum
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32  *maxLaneNumPtr
)
{
    GT_STATUS                       rc;     /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* port interface mode */
    CPSS_PORT_SPEED_ENT             speed; /* current port speed */
    GT_U32 startSerdes;
    GT_U32 maxLane;

    rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
    if( GT_OK != rc )
    {
        return rc;
    }

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ||
       (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes, &maxLane);
        *maxLaneNumPtr = maxLane-1;
        return rc;
    }
    else
    {
        switch(ifMode)
        {
            case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            case CPSS_PORT_INTERFACE_MODE_SGMII_E:  *maxLaneNumPtr = 0;
                                                    break;
            case CPSS_PORT_INTERFACE_MODE_HX_E:
            case CPSS_PORT_INTERFACE_MODE_RXAUI_E:  *maxLaneNumPtr = 1;
                                                    break;
            case CPSS_PORT_INTERFACE_MODE_XGMII_E:  *maxLaneNumPtr = 3;
                                                    break;
            case CPSS_PORT_INTERFACE_MODE_XLG_E:    *maxLaneNumPtr = 7;
                                                    break;
            case CPSS_PORT_INTERFACE_MODE_KR_E:
            case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                                                    switch(speed)
                                                    {
                                                        case CPSS_PORT_SPEED_10000_E:   *maxLaneNumPtr = 0; break;
                                                        case CPSS_PORT_SPEED_20000_E:   *maxLaneNumPtr = 1; break;
                                                        case CPSS_PORT_SPEED_40000_E:   *maxLaneNumPtr = 3; break;
                                                        default:                        return GT_BAD_PARAM;
                                                    }
                                                    break;
            default: return GT_BAD_PARAM;
        }

        return GT_OK;
    }
}


