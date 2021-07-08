/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChPortPfcUT.c
*
* DESCRIPTION:
*       Unit tests for cpssDxChPortPfc, that provides
*       CPSS implementation for Priority Flow Control functionality.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 25 $
*******************************************************************************/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* defines */

/* Default valid value for port id */
#define PORT_PFC_VALID_PHY_PORT_CNS  0

/* Invalid profileIndex */
#define PORT_PFC_INVALID_PROFILEINDEX_CNS   8

/* Invalid tcQueue */
#define PORT_PFC_INVALID_TCQUEUE_CNS        8

/* Invalid xoffThreshold */
#define PORT_PFC_INVALID_XOFFTHRESHOLD_CNS  0x7FF+1

/* Invalid xoffThreshold */
#define PORT_PFC_INVALID_XONTHRESHOLD_CNS   0x1

/* Invalid dropThreshold */
#define PORT_PFC_INVALID_DROPTHRESHOLD_CNS  0x7FF+1

#define PORT_PFC_INVALID_XOFFTHRESHOLD_MAC(_devNum)            \
        ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) ? \
        (PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(_devNum) + 1)       : \
        (PORT_PFC_INVALID_XOFFTHRESHOLD_CNS))

#define PORT_PFC_INVALID_XONTHRESHOLD_MAC(_devNum)            \
        ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) ? \
        (PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(_devNum) + 1)       : \
        (PORT_PFC_INVALID_XONTHRESHOLD_CNS))

#define PORT_PFC_INVALID_DROPTHRESHOLD_MAC(_devNum)            \
        ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) ? \
        (PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(_devNum) + 1)       : \
        (PORT_PFC_INVALID_DROPTHRESHOLD_CNS))

#define PORT_PFC_INVALID_XOFFTHRESHOLD_STEP_MAC(_devNum)       \
        ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) ? \
        ((GT_U32)(BIT_24))                                   : \
        (300))

#define PORT_PFC_INVALID_XONTHRESHOLD_STEP_MAC(_devNum)       \
        ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) ? \
        ((GT_U32)(BIT_24))                                   : \
        (1))

#define PORT_PFC_INVALID_DROPTHRESHOLD__STEP_MAC(_devNum)      \
        ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) ? \
        ((GT_U32)(BIT_24))                                   : \
        (500))

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcCountingModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcCountingModeSet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call withall correct pfcCountMode [CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E /
                                            CPSS_DXCH_PORT_PFC_COUNT_PACKETS_E].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcCountingModeGet with not-NULL pfcCountModePtr.
    Expected: GT_OK and the same pfcCountMode as was set.
    1.3. Call with wrong enum values pfcCountMode.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  pfcCountMode = CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E;
    CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  pfcCountModeGet;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call withall correct pfcCountMode [CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E /
                                                    CPSS_DXCH_PORT_PFC_COUNT_PACKETS_E].
            Expected: GT_OK.
        */
        pfcCountMode = CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E;

        st = cpssDxChPortPfcCountingModeSet(dev, pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortPfcCountingModeGet with not-NULL pfcCountModePtr.
            Expected: GT_OK and the same pfcCountMode as was set.
        */
        st = cpssDxChPortPfcCountingModeGet(dev, &pfcCountModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcCountingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(pfcCountMode, pfcCountModeGet,
                         "got another pfcCountMode then was set: %d", dev);

        /*
            1.1. Call withall correct pfcCountMode [CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E /
                                                    CPSS_DXCH_PORT_PFC_COUNT_PACKETS_E].
            Expected: GT_OK.
        */
        pfcCountMode = CPSS_DXCH_PORT_PFC_COUNT_PACKETS_E;

        st = cpssDxChPortPfcCountingModeSet(dev, pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortPfcCountingModeGet with not-NULL pfcCountModePtr.
            Expected: GT_OK and the same pfcCountMode as was set.
        */
        st = cpssDxChPortPfcCountingModeGet(dev, &pfcCountModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcCountingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(pfcCountMode, pfcCountModeGet,
                         "got another pfcCountMode then was set: %d", dev);

        /*
            1.3. Call with wrong enum values pfcCountMode.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPfcCountingModeSet
                            (dev, pfcCountMode),
                            pfcCountMode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcCountingModeSet(dev, pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcCountingModeSet(dev, pfcCountMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcCountingModeGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcCountingModeGet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with not null pfcCountModePtr.
    Expected: GT_OK.
    1.2. Call with wrong pfcCountModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  pfcCountMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pfcCountModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortPfcCountingModeGet(dev, &pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong pfcCountModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcCountingModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcCountingModeGet(dev, &pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcCountingModeGet(dev, &pfcCountMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcEnableSet
(
    IN  GT_U8     devNum,
    IN  CPSS_DXCH_PORT_PFC_ENABLE_ENT pfcEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcEnableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with state [CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_ONLY_E/
                          CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PFC_ENABLE_ENT enable;
    CPSS_DXCH_PORT_PFC_ENABLE_ENT state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        for (enable = CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_ONLY_E;
              enable <=CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E;
              enable++)
        {
             /*
               1.1. Call with state.
               Expected: GT_OK.
            */

            st = cpssDxChPortPfcEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /*
               1.2. Call cpssDxChPortPfcEnableGet.
               Expected: GT_OK and the same enable options.
            */
            st = cpssDxChPortPfcEnableGet(dev, &state);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortPfcEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                            "get another enable value than was set: %d, %d", dev);
        }
        /*
           1.3. Call with wrong enable option.
           Expected: GT_BAD_PARAM.
        */

        enable = CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E + 1;
        st = cpssDxChPortPfcEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, enable);
    }


    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E;

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcEnableGet
(
    IN   GT_U8     devNum,
    OUT CPSS_DXCH_PORT_PFC_ENABLE_ENT *pfcEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PFC_ENABLE_ENT state;

    /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortPfcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with statePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssDxChPortPfcEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcGlobalDropEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcGlobalDropEnableSet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcGlobalDropEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with state [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
        */

        /* Call function with enable = GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChPortPfcGlobalDropEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssDxChPortPfcGlobalDropEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortPfcGlobalDropEnableGet(dev, &state);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssDxChPortPfcGlobalDropEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                        "get another enable value than was set: %d, %d", dev);

        /*
           1.1. Call with state [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
        */

        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChPortPfcGlobalDropEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssDxChPortPfcGlobalDropEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortPfcGlobalDropEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "[cpssDxChPortPfcGlobalDropEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                      "get another enable value than was set: %d, %d", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcGlobalDropEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcGlobalDropEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcGlobalDropEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcGlobalDropEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion)
    1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortPfcGlobalDropEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with statePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssDxChPortPfcGlobalDropEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcGlobalDropEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcGlobalDropEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcGlobalQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcGlobalQueueConfigSet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with tcQueue [0 - 7],
                   xoffThreshold [0 - 0x7FF],
                   dropThreshold [0 - 0x7FF].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcGlobalQueueConfigGet with not-NULL pointers.
    Expected: GT_OK and the same values as was set.
    1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong xoffThreshold [PORT_PFC_INVALID_XOFFTHRESHOLD_CNS].
    Expected: NOT GT_OK.
    1.5. Call with wrong dropThreshold [PORT_PFC_INVALID_DROPTHRESHOLD_CNS].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8   tcQueue = 0;
    GT_U32  xoffThreshold = 0;
    GT_U32  xoffThresholdGet = 1;
    GT_U32  dropThreshold = 0;
    GT_U32  dropThresholdGet = 1;
    GT_U32  xonThreshold = 0;
    GT_U32  xonThresholdGet = 1;
    GT_U32  xoffStep;
    GT_U32  xonStep;
    GT_U32  dropStep;
    GT_U8   queueStep;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tcQueue [0 - 7], xoffThreshold [0 - 0x7FF], dropThreshold [0 - 0x7FF].
            Expected: GT_OK.
        */

        queueStep = 1;
        xoffStep  = PORT_PFC_INVALID_XOFFTHRESHOLD_STEP_MAC(dev);
        xonStep   = PORT_PFC_INVALID_XONTHRESHOLD_STEP_MAC(dev);
        dropStep  = PORT_PFC_INVALID_DROPTHRESHOLD__STEP_MAC(dev);

        if (GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            queueStep *= 4;
            xoffStep  *= 4;
            xonStep   *= 4;
            dropStep  *= 4;
        }

        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue += queueStep)
            for(xoffThreshold = 0;
                xoffThreshold < PORT_PFC_INVALID_XOFFTHRESHOLD_MAC(dev);
                xoffThreshold += xoffStep)
                for(dropThreshold = 0;
                    dropThreshold < PORT_PFC_INVALID_DROPTHRESHOLD_MAC(dev);
                    dropThreshold += dropStep)
                    for(xonThreshold = 0;
                        xonThreshold < PORT_PFC_INVALID_XONTHRESHOLD_MAC(dev);
                        xonThreshold += xonStep)

                    {
                        st = cpssDxChPortPfcGlobalQueueConfigSet(dev, tcQueue,
                                                                 xoffThreshold, dropThreshold, xonThreshold);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                        if(GT_OK == st)
                        {
                            /*
                                1.2. Call cpssDxChPortPfcGlobalQueueConfigGet with not-NULL tcQueuePtr.
                                Expected: GT_OK and the same tcQueue as was set.
                            */
                            st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue,
                                                                     &xoffThresholdGet, &dropThresholdGet, &xonThresholdGet);
                            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChPortPfcGlobalQueueConfigGet: %d", dev);
                            UTF_VERIFY_EQUAL1_STRING_MAC(xoffThreshold, xoffThresholdGet,
                                                         "got another xoffThreshold then was set: %d", dev);
                            UTF_VERIFY_EQUAL1_STRING_MAC(dropThreshold, dropThresholdGet,
                                                         "got another dropThreshold then was set: %d", dev);

                            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
                            {
                                UTF_VERIFY_EQUAL1_STRING_MAC(xonThreshold, xonThresholdGet,
                                                             "got another xoffThreshold then was set: %d", dev);
                            }
                        }
                    }

        /*
            1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 0;

        /*
            1.4. Call with wrong xoffThreshold [PORT_PFC_INVALID_XOFFTHRESHOLD_CNS].
            Expected: NOT GT_OK.
        */
        xoffThreshold = PORT_PFC_INVALID_XOFFTHRESHOLD_MAC(dev);

        st = cpssDxChPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        xoffThreshold = 0;

        /*
            1.5. Call with wrong dropThreshold [PORT_PFC_INVALID_DROPTHRESHOLD_CNS].
            Expected: NOT GT_OK.
        */
        dropThreshold = PORT_PFC_INVALID_DROPTHRESHOLD_MAC(dev);

        st = cpssDxChPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        dropThreshold = 0;

        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /*
                1.5. Call with wrong dropThreshold [PORT_PFC_INVALID_DROPTHRESHOLD_CNS].
                Expected: NOT GT_OK.
            */
            xonThreshold = PORT_PFC_INVALID_XONTHRESHOLD_MAC(dev);

            st = cpssDxChPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            xonThreshold = 0;
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcGlobalQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcGlobalQueueConfigGet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with tcQueue [0 - 7] and  not-NULL pointers.
    Expected: GT_OK.
    1.2. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.3. Call with wrong  xoffThresholdPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong  dropThresholdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8      tcQueue = 0;
    GT_U32  xoffThreshold = 0;
    GT_U32  dropThreshold = 0;
    GT_U32  xonThreshold = 0;

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL tcQueue.
            Expected: GT_OK.
        */
        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue++)
        {
            st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, tcQueue = NULL", dev);

        tcQueue = 0;

        /*
            1.3. Call with wrong xoffThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue, NULL, &dropThreshold, &xonThreshold);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xoffThresholdPtr = NULL", dev);

        /*
            1.4. Call with wrong dropThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, NULL, &xonThreshold);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropThresholdPtr = NULL", dev);

        /*
            1.5. Call with wrong dropThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xonThresholdPtr = NULL", dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcProfileIndexSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_U32   profileIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcProfileIndexSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion)
    1.1.1. Call with profileIndex [0 / 7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortPfcProfileIndexGet with the same params.
    Expected: GT_OK and the same profileIndex.
    1.1.3. Call with profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
    Expected: NOT GT_OK.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_PFC_VALID_PHY_PORT_CNS;
    GT_U32      profileIndex = 0;
    GT_U32      profileIndexGet = 0;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                         UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with profileIndex [0 / 7].
                Expected: GT_OK.
            */

            for(profileIndex = 0;
                profileIndex < PORT_PFC_INVALID_PROFILEINDEX_CNS;
                profileIndex++)
            {
                st = cpssDxChPortPfcProfileIndexSet(dev, port, profileIndex);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

                if(GT_OK == st)
                {
                    /*
                       1.1.2. Call cpssDxChPortPfcProfileIndexGet.
                       Expected: GT_OK and the same profileIndex.
                    */
                    st = cpssDxChPortPfcProfileIndexGet(dev, port, &profileIndexGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                    "[cpssDxChPortPfcProfileIndexGet]: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(profileIndex, profileIndexGet,
                                    "get another profileIndex value than was set: %d, %d", dev, port);
                }
            }

            /*
                1.1.3. Call with profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
                Expected: NOT GT_OK.
            */

            profileIndex = PORT_PFC_INVALID_PROFILEINDEX_CNS;

            st = cpssDxChPortPfcProfileIndexSet(dev, port, profileIndex);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

            profileIndex = 0;
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each out of range non-active port */
            /* profileIndex == GT_TRUE    */
            if(port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
            {
                st = cpssDxChPortPfcProfileIndexSet(dev, port, profileIndex);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPfcProfileIndexSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    profileIndex = GT_TRUE;
    port = PORT_PFC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                         UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcProfileIndexSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcProfileIndexSet(dev, port, profileIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U32  *profileIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcProfileIndexGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion)
    1.1.1. Call with non-null profileIndexGetPtr.
    Expected: GT_OK.
    1.1.2. Call with profileIndexGetPtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_PFC_VALID_PHY_PORT_CNS;
    GT_U32      profileIndexGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null profileIndexGetPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortPfcProfileIndexGet(dev, port, &profileIndexGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with profileIndexGetPtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortPfcProfileIndexGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all out of range non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            if(port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
            {
                st = cpssDxChPortPfcProfileIndexGet(dev, port, &profileIndexGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPfcProfileIndexGet(dev, port, &profileIndexGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_PFC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcProfileIndexGet(dev, port, &profileIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcProfileIndexGet(dev, port, &profileIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcProfileQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcProfileQueueConfigSet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7],
                        pfcProfileCfg.xonThreshold = 10,
                        pfcProfileCfg.xoffThreshold = 10,
                        pfcProfileCfg.xonAlpha [1 / 2 / 0],
                        pfcProfileCfg.xoffAlpha [1 / 4 / 3].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcProfileQueueConfigGet with not-NULL pfcProfileCfgPtr.
    Expected: GT_OK and the same pfcProfileCfg as was set.
    1.3. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.5. Call with wrong pfcProfileCfg [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   profileIndex = 0;
    GT_U8    tcQueue = 0;
    CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC   pfcProfileCfg;
    CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC   pfcProfileCfgGet;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7]
                 and pfcProfileCfg [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [0], tcQueue [0] pfcProfileCfg [0].*/
        profileIndex = 0;
        tcQueue = 0;
        pfcProfileCfg.xonThreshold = 10;
        pfcProfileCfg.xoffThreshold = 10;

        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            pfcProfileCfg.xonAlpha = 2;
            pfcProfileCfg.xoffAlpha = 2;
        }

        st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortPfcProfileQueueConfigGet with not-NULL pfcProfileCfgPtr.
            Expected: GT_OK and the same pfcProfileCfg as was set.
        */
        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortPfcProfileQueueConfigGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonThreshold,
                                     pfcProfileCfgGet.xonThreshold,
               "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffThreshold,
                                     pfcProfileCfgGet.xoffThreshold,
               "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonAlpha,
                                         pfcProfileCfgGet.xonAlpha,
                   "got another pfcProfileCfg.xonAlpha then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffAlpha,
                                         pfcProfileCfgGet.xoffAlpha,
                   "got another pfcProfileCfg.xonAlpha then was set: %d", dev);
        }

        /*
            1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7]
                 and pfcProfileCfg [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [3], tcQueue [3] pfcProfileCfg [0x1F00].*/
        profileIndex = 3;
        tcQueue = 3;

        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            pfcProfileCfg.xonAlpha = 2;
            pfcProfileCfg.xoffAlpha = 4;
        }

        st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortPfcProfileQueueConfigGet with not-NULL pfcProfileCfgPtr.
            Expected: GT_OK and the same pfcProfileCfg as was set.
        */
        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortPfcProfileQueueConfigGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonThreshold,
                                     pfcProfileCfgGet.xonThreshold,
               "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffThreshold,
                                     pfcProfileCfgGet.xoffThreshold,
               "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonAlpha,
                                         pfcProfileCfgGet.xonAlpha,
                   "got another pfcProfileCfg.xonAlpha then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffAlpha,
                                         pfcProfileCfgGet.xoffAlpha,
                   "got another pfcProfileCfg.xonAlpha then was set: %d", dev);
        }

        /*
            1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7]
                 and pfcProfileCfg [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [7], tcQueue [7] pfcProfileCfg [0x1FFF].*/
        profileIndex = 7;
        tcQueue = 7;
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            pfcProfileCfg.xonAlpha = 0;
            pfcProfileCfg.xoffAlpha = 3;
        }

        st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortPfcProfileQueueConfigGet with not-NULL pfcProfileCfgPtr.
            Expected: GT_OK and the same pfcProfileCfg as was set.
        */
        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortPfcProfileQueueConfigGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonThreshold,
                                     pfcProfileCfgGet.xonThreshold,
               "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffThreshold,
                                     pfcProfileCfgGet.xoffThreshold,
               "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonAlpha,
                                         pfcProfileCfgGet.xonAlpha,
                   "got another pfcProfileCfg.xonAlpha then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffAlpha,
                                         pfcProfileCfgGet.xoffAlpha,
                   "got another pfcProfileCfg.xonAlpha then was set: %d", dev);
        }

        /*
            1.3. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
            Expected: NOT GT_OK.
        */
        profileIndex = PORT_PFC_INVALID_PROFILEINDEX_CNS;

        st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        profileIndex = 7;

        /*
            1.4. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 7;

        /*
            1.5. Call with wrong pfcProfileCfg [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Aldrin2 has introduced alpha param for PFC DBA */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            /*
               1.7. Call with wrong pfcProfileCfg.xonAlpha [out of range].
               Expected: GT_BAD_PARAM.
            */
            pfcProfileCfg.xonAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;
            pfcProfileCfg.xoffAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;

            st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            pfcProfileCfg.xonAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        }

        /* Aldrin2 has introduced alpha param for PFC DBA */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            /*
               1.7. Call with wrong pfcProfileCfg.xoffAlpha [out of range].
               Expected: GT_BAD_PARAM.
            */
            pfcProfileCfg.xoffAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;
            pfcProfileCfg.xonAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

            st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            pfcProfileCfg.xoffAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        }

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcProfileQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcProfileQueueConfigGet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null pfcProfileCfg.
    Expected: GT_OK.
    1.2. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
    Expected: NOT GT_OK.
    1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong  pfcProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      profileIndex = 0;
    GT_U8       tcQueue = 0;
    CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC pfcProfileCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null pfcProfileCfg.
            Expected: GT_OK.
        */
        profileIndex = 0;
        tcQueue = 0;

        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null pfcProfileCfg.
            Expected: GT_OK.
        */
        profileIndex = 7;
        tcQueue = 7;

        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
            Expected: NOT GT_OK.
        */
        profileIndex = PORT_PFC_INVALID_PROFILEINDEX_CNS;

        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        profileIndex = 7;

        /*
            1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 7;

        /*
            1.4. Call with wrong  pfcProfileCfg pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pfcProfileCfg = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcTimerMapEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_BOOL                                 enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcTimerMapEnableSet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                   enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcTimerMapEnableGet with the same profileSet.
    Expected: GT_OK and the same enable.
    1.3. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK for DxCh and GT_OK for others.
    1.4. Call with wrong enum values profileSet
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet;
    GT_BOOL                                 enable;
    GT_BOOL                                 enableGet;


    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                           enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
        enable     = GT_FALSE;

        st = cpssDxChPortPfcTimerMapEnableSet(dev, profileSet, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, enable);

        /*
            1.2. Call cpssDxChPortPfcTimerMapEnableGet with the same profileSet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcTimerMapEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                   "get another value than was set: %d, %d", dev, enableGet);

        /*
            1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                           enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_4_E;
        enable     = GT_TRUE;

        st = cpssDxChPortPfcTimerMapEnableSet(dev, profileSet, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, enable);

        /*
            1.2. Call cpssDxChPortPfcTimerMapEnableGet with the same profileSet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcTimerMapEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                   "get another value than was set: %d, %d", dev, enableGet);

        /*
            1.3. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK for DxCh and GT_OK for others.
        */
        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E;

        st = cpssDxChPortPfcTimerMapEnableSet(dev, profileSet, enable);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, enable);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, enable);
        }

        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E;

        st = cpssDxChPortPfcTimerMapEnableSet(dev, profileSet, enable);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, enable);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, enable);
        }

        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;

        /*
            1.4. Call with wrong enum values profileSet
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPfcTimerMapEnableSet
                            (dev, profileSet, enable),
                            profileSet);
    }

    profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
    enable     = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcTimerMapEnableSet(dev, profileSet, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcTimerMapEnableSet(dev, profileSet, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcTimerMapEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_BOOL                                *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcTimerMapEnableGet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                   non NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK for DxCh and GT_OK for others.
    1.3. Call with wrong enum values profileSet
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with NULL enablePtr
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet;
    GT_BOOL                                 enable;


    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                           non NULL enablePtr.
            Expected: GT_OK.
        */
        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;

        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileSet);

        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_4_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_4_E;

        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileSet);

        /*
            1.2. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK for DxCh and GT_OK for others.
        */
        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E;

        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enable);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileSet);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileSet);
        }

        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E;

        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enable);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileSet);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileSet);
        }

        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;

        /*
            1.3. Call with wrong enum values profileSet
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPfcTimerMapEnableGet
                            (dev, profileSet, &enable),
                            profileSet);

        /*
            1.4. Call with NULL enablePtr
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcTimerMapEnableGet(dev, profileSet, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcTimerToQueueMapSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcTimerToQueueMapSet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with pfcTimer [0 / 5 / 7],
                   tcQueue [0 / 5 / 7].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcTimerToQueueMapGet with the same pfcTimer.
    Expected: GT_OK and the same tcQueue.
    1.3. Call with out of range pfcTimer [8]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      pfcTimer   = 0;
    GT_U32      tcQueue    = 0;
    GT_U32      tcQueueGet = 0;


    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with pfcTimer [0 / 5 / 7],
                           tcQueue [0 / 5 / 7].
            Expected: GT_OK.
        */
        pfcTimer = 0;
        tcQueue  = 0;

        st = cpssDxChPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pfcTimer, tcQueue);

        /*
            1.2. Call cpssDxChPortPfcTimerToQueueMapGet with the same pfcTimer.
            Expected: GT_OK and the same tcQueue.
        */
        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcTimerToQueueMapGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(tcQueue, tcQueueGet,
                   "get another value than was set: %d, %d", dev, tcQueueGet);

        /*
            1.1. Call with pfcTimer [0 / 5 / 7],
                           tcQueue [0 / 5 / 7].
            Expected: GT_OK.
        */
        pfcTimer = 5;
        tcQueue  = 5;

        st = cpssDxChPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pfcTimer, tcQueue);

        /*
            1.2. Call cpssDxChPortPfcTimerToQueueMapGet with the same pfcTimer.
            Expected: GT_OK and the same tcQueue.
        */
        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcTimerToQueueMapGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(tcQueue, tcQueueGet,
                   "get another value than was set: %d, %d", dev, tcQueueGet);

        /*
            1.1. Call with pfcTimer [0 / 5 / 7],
                           tcQueue [0 / 5 / 7].
            Expected: GT_OK.
        */
        pfcTimer = 7;
        tcQueue  = 7;

        st = cpssDxChPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pfcTimer, tcQueue);

        /*
            1.2. Call cpssDxChPortPfcTimerToQueueMapGet with the same pfcTimer.
            Expected: GT_OK and the same tcQueue.
        */
        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcTimerToQueueMapGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(tcQueue, tcQueueGet,
                   "get another value than was set: %d, %d", dev, tcQueueGet);

        /*
            1.3. Call with out of range pfcTimer [8]
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        pfcTimer = 8;

        st = cpssDxChPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, pfcTimer);

        pfcTimer = 0;

        /*
            1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS]
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssDxChPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcQueue);
    }

    pfcTimer = 0;
    tcQueue  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcTimerToQueueMapGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcTimerToQueueMapGet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with pfcTimer[0 / 5 / 7],
                   non NULL tcQueuePtr.
    Expected: GT_OK.
    1.2. Call with out of range pfcTimer [8]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with NULL tcQueuePtr and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      pfcTimer = 0;
    GT_U32      tcQueue  = 0;


    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with pfcTimer[0 / 5 / 7],
                           non NULL tcQueuePtr.
            Expected: GT_OK.
        */

        /* call with pfcTimer = 0 */
        pfcTimer = 0;

        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pfcTimer);

        /* call with pfcTimer = 5 */
        pfcTimer = 5;

        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pfcTimer);

        /* call with pfcTimer = 7 */
        pfcTimer = 7;

        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pfcTimer);

        /*
            1.2. Call with out of range pfcTimer [8]
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        pfcTimer = 8;

        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pfcTimer);

        pfcTimer = 0;

        /*
            1.3. Call with NULL tcQueuePtr and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcQueuePtr = NULL", dev);
    }

    pfcTimer = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcShaperToPortRateRatioSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    IN  GT_U32                                  shaperToPortRateRatio
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcShaperToPortRateRatioSet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_3_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                   tcQueue [0 / 5 / 7],
                   shaperToPortRateRatio [0 / 50 / 100].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcShaperToPortRateRatioGet with the same profileSet.
    Expected: GT_OK and the same shaperToPortRateRatio.
    1.3. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK for DxCh and GT_OK for others.
    1.4. Call with wrong enum values profileSet
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with out of range shaperToPortRateRatio [101],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet;
    GT_U8                                   tcQueue;
    GT_U32                                  ratio;
    GT_U32                                  ratioGet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E ],
                           tcQueue [0],
                           shaperToPortRateRatio [0 .. 100].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
        tcQueue    = 0;

        for (ratio = 0; ratio <= 100; ratio++)
        {
            st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);

            /*
                1.2. Call cpssDxChPortPfcShaperToPortRateRatioGet with the same profileSet.
                Expected: GT_OK and the same shaperToPortRateRatio.
            */
            st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratioGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcShaperToPortRateRatioGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(ratio, ratioGet,
                       "get another value than was set: %d, %d", dev, ratioGet);
        }


        /*
            1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_3_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                           tcQueue [0 / 5 / 7],
                           shaperToPortRateRatio [0 / 50 / 100].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_3_E;
        tcQueue    = 5;
        ratio      = 50;

        st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);

        /*
            1.2. Call cpssDxChPortPfcShaperToPortRateRatioGet with the same profileSet.
            Expected: GT_OK and the same shaperToPortRateRatio.
        */
        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratioGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcShaperToPortRateRatioGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(ratio, ratioGet,
                   "get another value than was set: %d, %d", dev, ratioGet);

        /*
            1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_3_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                           tcQueue [0 / 5 / 7],
                           shaperToPortRateRatio [0 / 50 / 100].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_4_E;
        tcQueue    = 7;
        ratio      = 100;

        st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);

        /*
            1.2. Call cpssDxChPortPfcShaperToPortRateRatioGet with the same profileSet.
            Expected: GT_OK and the same shaperToPortRateRatio.
        */
        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratioGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcShaperToPortRateRatioGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(ratio, ratioGet,
                   "get another value than was set: %d, %d", dev, ratioGet);

        /*
            1.3. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK for DxCh and GT_OK for others.
        */
        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E;

        st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);
        }

        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E;

        st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);
        }

        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;

        /*
            1.4. Call with wrong enum values profileSet
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPfcShaperToPortRateRatioSet
                            (dev, profileSet, tcQueue, ratio),
                            profileSet);

        /*
            1.5. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcQueue);

        tcQueue = 0;

        /*
            1.6. Call with out of range shaperToPortRateRatio [101],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        ratio = 101;

        st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);
    }

    profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
    tcQueue    = 0;
    ratio      = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcShaperToPortRateRatioGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    OUT GT_U32                                 *shaperToPortRateRatioPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcShaperToPortRateRatioGet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_3_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                   tcQueue [0 / 5 / 7],
                   not NULL shaperToPortRateRatioPtr.
    Expected: GT_OK.
    1.2. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                               CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK for DxCh and GT_OK for others.
    1.3. Call with wrong enum values profileSet
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with NULL shaperToPortRateRatioPtr,
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet;
    GT_U8                                   tcQueue;
    GT_U32                                  ratio;


    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_1_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_3_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_4_E],
                           tcQueue [0 / 5 / 7],
                           not NULL shaperToPortRateRatioPtr.
            Expected: GT_OK.
        */
        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
        tcQueue    = 0;

        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);

        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_3_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_3_E;
        tcQueue    = 5;

        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);

        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_4_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_4_E;
        tcQueue    = 7;

        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);

        /*
            1.2. Call with profileSet [CPSS_PORT_TX_SCHEDULER_PROFILE_5_E /
                                       CPSS_PORT_TX_SCHEDULER_PROFILE_8_E],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK for DxCh and GT_OK for others.
        */
        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_5_E;

        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);
        }

        /* call with  profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E */
        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_8_E;

        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);
        }

        profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;

        /*
            1.3. Call with wrong enum values profileSet
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPfcShaperToPortRateRatioGet
                            (dev, profileSet, tcQueue, &ratio),
                            profileSet);

        /*
            1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcQueue);

        tcQueue = 0;

        /*
            1.5. Call with NULL shaperToPortRateRatioPtr,
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, shaperToPortRateRatioPtr = NULL", dev);
    }

    profileSet = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
    tcQueue    = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcForwardEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcForwardEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat2)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_U8                   dev;
    GT_STATUS               st      = GT_OK;
    GT_PHYSICAL_PORT_NUM    port    = PORT_PFC_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable  = GT_FALSE;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortPfcForwardEnableGet(dev, port, &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /*
               1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortPfcForwardEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortPfcForwardEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPfcForwardEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPfcForwardEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPfcForwardEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_PFC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcForwardEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcForwardEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcForwardEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcForwardEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat2)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortPfcForwardEnableGet.
    Expected: GT_OK and the same enable value as was set.
*/
    GT_U8                   dev;
    GT_STATUS               st          = GT_OK;
    GT_PHYSICAL_PORT_NUM    port        = PORT_PFC_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable      = GT_FALSE;
    GT_BOOL                 enableGet   = GT_TRUE;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with enable [GT_FALSE] */
            enable = GT_FALSE;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssDxChPortPfcForwardEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChPortPfcForwardEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);

            /* Call function with enable [GT_TRUE] */
            enable = GT_TRUE;
            st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssDxChPortPfcForwardEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChPortPfcForwardEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_PFC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcForwardEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcLossyDropConfigSet)
{
/*
    1.1. Call with valid params.
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcLossyDropConfigGet.
    Expected: GT_OK and the same lossyDropConfigPtr value as was set.
    1.3. Call with NULL lossyDropConfigPtr.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC  lossyDropConfig;
    CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC  lossyDropConfigGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        lossyDropConfig.fromCpuLossyDropEnable = GT_FALSE;
        lossyDropConfig.toCpuLossyDropEnable = GT_FALSE;
        lossyDropConfig.toTargetSnifferLossyDropEnable = GT_FALSE;

        st = cpssDxChPortPfcLossyDropConfigSet(dev, &lossyDropConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropConfigGet(dev, &lossyDropConfigGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.fromCpuLossyDropEnable,
                                     lossyDropConfigGet.fromCpuLossyDropEnable,
            "get another lossyDropConfig.fromCpuLossyDropEnable - %d ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.toCpuLossyDropEnable,
                                     lossyDropConfigGet.toCpuLossyDropEnable,
            "get another lossyDropConfig.toCpuLossyDropEnable - %d ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.toTargetSnifferLossyDropEnable,
                                     lossyDropConfigGet.toTargetSnifferLossyDropEnable,
            "get another lossyDropConfig.toTargetSnifferLossyDropEnable - %d ", dev);

        /* 1.1 */
        lossyDropConfig.fromCpuLossyDropEnable = GT_TRUE;
        lossyDropConfig.toCpuLossyDropEnable = GT_FALSE;
        lossyDropConfig.toTargetSnifferLossyDropEnable = GT_TRUE;

        st = cpssDxChPortPfcLossyDropConfigSet(dev, &lossyDropConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropConfigGet(dev, &lossyDropConfigGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.fromCpuLossyDropEnable,
                                     lossyDropConfigGet.fromCpuLossyDropEnable,
            "get another lossyDropConfig.fromCpuLossyDropEnable - %d ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.toCpuLossyDropEnable,
                                     lossyDropConfigGet.toCpuLossyDropEnable,
            "get another lossyDropConfig.toCpuLossyDropEnable - %d ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.toTargetSnifferLossyDropEnable,
                                     lossyDropConfigGet.toTargetSnifferLossyDropEnable,
            "get another lossyDropConfig.toTargetSnifferLossyDropEnable - %d ", dev);

        /* 1.1 */
        lossyDropConfig.fromCpuLossyDropEnable = GT_TRUE;
        lossyDropConfig.toCpuLossyDropEnable = GT_TRUE;
        lossyDropConfig.toTargetSnifferLossyDropEnable = GT_TRUE;

        st = cpssDxChPortPfcLossyDropConfigSet(dev, &lossyDropConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropConfigGet(dev, &lossyDropConfigGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.fromCpuLossyDropEnable,
                                     lossyDropConfigGet.fromCpuLossyDropEnable,
            "get another lossyDropConfig.fromCpuLossyDropEnable - %d ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.toCpuLossyDropEnable,
                                     lossyDropConfigGet.toCpuLossyDropEnable,
            "get another lossyDropConfig.toCpuLossyDropEnable - %d ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(lossyDropConfig.toTargetSnifferLossyDropEnable,
                                     lossyDropConfigGet.toTargetSnifferLossyDropEnable,
            "get another lossyDropConfig.toTargetSnifferLossyDropEnable - %d ", dev);

        /* 1.3 */
        st = cpssDxChPortPfcLossyDropConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcLossyDropConfigSet(dev, &lossyDropConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcLossyDropConfigSet(dev, &lossyDropConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcLossyDropConfigGet)
{
/*
    1.1.1. Call with not NULL lossyDropConfigPtr.
    Expected: GT_OK.
    1.1.2. Call with NULL lossyDropConfigPtr.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC  lossyDropConfig;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        st = cpssDxChPortPfcLossyDropConfigGet(dev, &lossyDropConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcLossyDropConfigGet(dev, &lossyDropConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcLossyDropConfigGet(dev, &lossyDropConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcLossyDropQueueEnableSet)
{
/*
    1.1. Call with valid parameters
                tcQueue [0/3/7]
                enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcLossyDropQueueEnableGet.
    Expected: GT_OK and the same lossyDropConfigPtr value as was set.
    1.3. Call with out of range tcQueue [8].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;

    GT_U8     tcQueue    = 0;
    GT_BOOL   enable     = GT_FALSE;
    GT_BOOL   enableGet  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        enable  = GT_TRUE;

        /* 1.1 */
        tcQueue = 0;

        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev,  tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);

        /* 1.1 */
        tcQueue = 3;

        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev,  tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);

        /* 1.1 */
        tcQueue = 7;

        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev,  tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);

        enable  = GT_FALSE;

        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev,  tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);

        /* 1.1 */
        tcQueue = 3;

        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev,  tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);

        /* 1.1 */
        tcQueue = 7;

        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev,  tcQueue, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);

        /* 1.3 */
        tcQueue = 8;
        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcLossyDropQueueEnableSet(dev, tcQueue, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcLossyDropQueueEnableGet)
{
/*
    1.1.1. Call with not NULL aneblePtr and valid parameters
            tcQueue [0/3/7].
    Expected: GT_OK.
    1.1.2. Call with NULL lossyDropConfigPtr.
    Expected: GT_OK.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;

    GT_U8     tcQueue    = 0;
    GT_BOOL   enable     = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        tcQueue    = 0;
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue    = 3;
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue    = 7;
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcLossyDropQueueEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcLossyDropQueueEnableGet(dev, tcQueue, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcTerminateFramesEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcTerminateFramesEnableGet
           with same enablePtr.
    Expected: GT_OK and the same enable than was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_BOOL enable = GT_FALSE;
    GT_BOOL enableGet = GT_FALSE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChPortPfcTerminateFramesEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortPfcTerminateFramesEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChPortPfcTerminateFramesEnableGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);

        /*  1.1.  */
        enable = GT_FALSE;

        st = cpssDxChPortPfcTerminateFramesEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        st = cpssDxChPortPfcTerminateFramesEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChPclPortListPortGroupEnableGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);
    }

    /* restore correct values */
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcTerminateFramesEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcTerminateFramesEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcTerminateFramesEnableGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with not null enablePtr.
    Expected: GT_OK.
    1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8    dev;
    GT_BOOL  enable;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChPortPfcTerminateFramesEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChPortPfcTerminateFramesEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcTerminateFramesEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcTerminateFramesEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcQueueCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    OUT GT_U32  *cntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcQueueCounterGet)
{
/*
    ITERATE_DEVICES (Lion)
    1.1. Call with tcQueue [0 - 7] and  not-NULL pointers.
    Expected: GT_OK.
    1.2. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.3. Call with wrong  cntPtr [NULL].
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U8       tcQueue = 0;
    GT_U32      cnt = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL tcQueue.
            Expected: GT_OK.
        */
        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue++)
        {
            st = cpssDxChPortPfcQueueCounterGet(dev, tcQueue, &cnt);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortPfcQueueCounterGet(dev, tcQueue, &cnt);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, tcQueue = NULL", dev);

        tcQueue = 0;

        /*
            1.3. Call with wrong cnt [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcQueueCounterGet(dev, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cnt = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcQueueCounterGet(dev, tcQueue, &cnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcQueueCounterGet(dev, tcQueue, &cnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcSourcePortToPfcCounterSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcSourcePortToPfcCounterSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with portNum [0 - 255] and valid pfcCounterNum.
    Expected: GT_OK.
    1.2. Call with wrong pfcCounterNum >= BIT_7
    Expected: NOT GT_OK.
*/

    GT_U8       dev;
    GT_STATUS   st  = GT_OK;
    GT_PHYSICAL_PORT_NUM   port = 0;
    GT_U32  pfcCounterNum = 0;

    GT_U32  pfcCounterNumGet;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            pfcCounterNum   = 0;

            st = cpssDxChPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, pfcCounterNum);

            /*
                1.1.2. Call cpssDxChPortPfcSourcePortToPfcCounterSet with not-NULL pfcCounterNumGet
                            and other parameters as in 1.1.1
                Expected: GT_OK and same pfcCounterNum that was written
            */
            st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNumGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPfcSourcePortToPfcCounterSet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(pfcCounterNum, pfcCounterNumGet, "got another counterNum then was set: %d, %d", dev, port);

            pfcCounterNum   = 100;

            st = cpssDxChPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, pfcCounterNum);

            /*
                1.1.2. Call cpssDxChPortPfcSourcePortToPfcCounterSet with not-NULL pfcCounterNumGet
                            and other parameters as in 1.1.1
                Expected: GT_OK and same pfcCounterNum that was written
            */
            st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNumGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortPfcSourcePortToPfcCounterSet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(pfcCounterNum, pfcCounterNumGet, "got another counterNum then was set: %d, %d", dev, port);

            /*
                1.1.3. Call with out of range pfcCounterNum [256]
                          and other parameters as in 1.1.1.
                Expected: NOT GT_OK
            */
            pfcCounterNum   = BIT_7;

            st = cpssDxChPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, limit = %d", dev, port, pfcCounterNum);
        }

        pfcCounterNum   = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    pfcCounterNum   = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfNextPhyPortGet(&port, GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcSourcePortToPfcCounterGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *pfcCounterNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcSourcePortToPfcCounterGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with portNum [0 - 255]
    Expected: GT_OK.
    Expected: NOT GT_OK.
*/
    GT_U8       dev;
    GT_STATUS   st  = GT_OK;
    GT_PHYSICAL_PORT_NUM   port = 0;
    GT_U32  pfcCounterNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {

            st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, pfcCounterNum);

            st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, NULL);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    pfcCounterNum   = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfNextPhyPortGet(&port, GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcCounterGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with tcQueue [0 - 7]
    1.1. Call with pfcCounterNum [0 - 127]
    Expected: GT_OK.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U8   tcQueue = 0;
    GT_U32  pfcCounterNum = 0;
    GT_U32  pfcCounterValue;
    GT_U32  pfcCounterMaxNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            pfcCounterMaxNum = 100;
        }
        else if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            pfcCounterMaxNum = 78;
        }
        else
        {
            pfcCounterMaxNum = 128;
        }

        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue++)
            for(pfcCounterNum = 0;
                pfcCounterNum < pfcCounterMaxNum;
                pfcCounterNum++)
            {
                st = cpssDxChPortPfcCounterGet(dev, tcQueue, pfcCounterNum, &pfcCounterValue);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tcQueue, pfcCounterNum);
            }

        /* call with NULL pointer */
        st = cpssDxChPortPfcCounterGet(dev, tcQueue, pfcCounterNum, NULL);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, tcQueue, pfcCounterNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcCounterGet(dev, tcQueue, pfcCounterNum, &pfcCounterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcCounterGet(dev, tcQueue, pfcCounterNum, &pfcCounterValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcXonMessageFilterEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcXonMessageFilterEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChPortPfcXonMessageFilterEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;

    GT_U8   dev;
    GT_BOOL enable;
    GT_BOOL enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (enable = GT_TRUE; enable <= GT_FALSE; enable++)
        {
             /*
               1.1. Call with enable.
               Expected: GT_OK.
            */

            st = cpssDxChPortPfcXonMessageFilterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /*
               1.2. Call cpssDxChPortPfcXonMessageFilterEnableGet.
               Expected: GT_OK and the same enable options.
            */
            st = cpssDxChPortPfcXonMessageFilterEnableGet(dev, &enableGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortPfcXonMessageFilterEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                            "get another enable value than was set: %d, %d", dev);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E );

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcXonMessageFilterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcXonMessageFilterEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcXonMessageFilterEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL  *enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcXonMessageFilterEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8   dev;
    GT_BOOL enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null enablePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortPfcXonMessageFilterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with enablePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssDxChPortPfcXonMessageFilterEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E );

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcXonMessageFilterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcXonMessageFilterEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcDbaAvailableBuffSet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  availableBuff
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcDbaAvailableBuffSet)
{
/*
    1.1. Call with availableBuff [0x91000 / 0x98000].
    Expected: GT_OK.
    1.2. Call with out of range availableBuff [0x1000000]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_U32      availableBuff = 0;
    GT_U32      availableBuffGet = 0;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. Call with availableBuff [0x91000].
             * Expected: GT_OK.
             */
            availableBuff = 0x91000;
            st = cpssDxChPortPfcDbaAvailableBuffSet(dev,0, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChPortPfcDbaAvailableBuffGet(dev, 0,&availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcAvailableBuffSet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                                         "got another availableBuff then was set: %d", dev);

            /* 1.1. Call with availableBuff [0x98000].
             * Expected: GT_OK.
             */
            availableBuff = 0x98000;
            st = cpssDxChPortPfcDbaAvailableBuffSet(dev,0, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChPortPfcDbaAvailableBuffGet(dev,0, &availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcAvailableBuffSet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                                         "got another availableBuff then was set: %d", dev);

            /* 1.2. Call with availableBuff out of range [0x100000].
             * Expected: GT_OK.
             */
            availableBuff = 0x1000000;
            st = cpssDxChPortPfcDbaAvailableBuffSet(dev,0, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            availableBuff = 0x1000;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcDbaAvailableBuffSet(dev,0, availableBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    availableBuff = 0x0;
    st = cpssDxChPortPfcDbaAvailableBuffSet(dev,0, availableBuff);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcDbaAvailableBuffGet
(
    IN   GT_U8                   devNum,
    OUT  GT_U32                  *availableBuffPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcDbaAvailableBuffGet)
{
/*
    1.1. Call with non-null availableBuffPtr.
    Expected: GT_OK.
    1.2. Call with wrong availableBuffPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_U32      availableBuffGet;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null availableBuffPtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortPfcDbaAvailableBuffGet(dev,0, &availableBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with NULL availableBuffPtr.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcDbaAvailableBuffGet(dev,0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, availableBuffGet = NULL", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcDbaAvailableBuffGet(dev,0, &availableBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcDbaAvailableBuffGet(dev,0, &availableBuffGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcDbaModeEnableSet
(
    IN   GT_U8                   devNum,
    IN   GT_BOOL                 enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcDbaModeEnableSet)
{
/*
    1.1. Call with enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with enable [GT_TRUE].
           Expected: GT_OK.
        */
        enable = GT_TRUE;
        st = cpssDxChPortPfcDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortPfcDbaModeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortPfcDbaModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable then was set: %d", dev);

        /*
         * 1.1. Call with enable [GT_FALSE].
         * Expected: GT_OK.
         */
        enable = GT_FALSE;
        st = cpssDxChPortPfcDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortPfcDbaModeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortDbaModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable then was set: %d", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    enable = GT_FALSE;
    st = cpssDxChPortPfcDbaModeEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPfcDbaModeEnableGet
(
    IN   GT_U8                   devNum,
    OUT  GT_BOOL                 *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPfcDbaModeEnableGet)
{
/*
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_BOOL     enablePtr;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null enablePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortPfcDbaModeEnableGet(dev, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with NULL enablePtr
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPfcDbaModeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPfcDbaModeEnableGet(dev, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPfcDbaModeEnableGet(dev, &enablePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortPfc suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortPfc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcCountingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcCountingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcGlobalDropEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcGlobalDropEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcGlobalQueueConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcGlobalQueueConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcProfileIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcProfileIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcProfileQueueConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcProfileQueueConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcTimerMapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcTimerMapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcTimerToQueueMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcTimerToQueueMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcShaperToPortRateRatioSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcShaperToPortRateRatioGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcForwardEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcForwardEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcLossyDropConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcLossyDropConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcLossyDropQueueEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcLossyDropQueueEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcTerminateFramesEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcTerminateFramesEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcQueueCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcSourcePortToPfcCounterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcSourcePortToPfcCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcXonMessageFilterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcXonMessageFilterEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcDbaAvailableBuffSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcDbaAvailableBuffGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcDbaModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPfcDbaModeEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortPfc)
