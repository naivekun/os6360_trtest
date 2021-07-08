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
* @file cpssDxChPortBufMgUT.c
*
* @brief Unit tests for cpssDxChPortBufMg, that provides
* CPSS implementation for Port configuration and control facility.
*
* @version   32
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define PORT_BUF_MG_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGlobalXonLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xonLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChGlobalXonLimitSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call cpssDxChGlobalXoffLimitSet with xoffLimit [0x00FF].
Expected: GT_OK.
1.2. Call with xonLimit [0x00EF].
Expected: GT_OK.
1.3. Call cpssDxChGlobalXonLimitGet with non-NULL pointer.
Expected: GT_OK and the same xonLimit.
1.4. Call with out of range xonLimit[0x0FFE].
Expected: NON GT_OK.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    GT_U32                          limit    = 0;
    GT_U32                          limitGet = 0;
    GT_U32                          xoffLimit    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChGlobalXoffLimitSet with xoffLimit [0x00FF].
            Expected: GT_OK.
        */
        xoffLimit = 0xFF;

        st = cpssDxChGlobalXoffLimitSet(dev, xoffLimit);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChGlobalXoffLimitSet: %d, %d", dev, xoffLimit);

        /*
           1.2. Call with xonLimit [0x00FE].
           Expected: GT_OK.
        */
        limit = 0x00FE;

        st = cpssDxChGlobalXonLimitSet(dev, limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, limit);

        /*
            1.3. Call cpssDxChGlobalXonLimitGet.
            Expected: GT_OK and the same xonLimit.
        */
        st = cpssDxChGlobalXonLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChGlobalXonLimitGet: %d", dev);
/*        UTF_VERIFY_EQUAL1_STRING_MAC(limit, limitGet,
                       "get another xonLimit than was set: %d", dev);*/

        /* 1.4. Call with xonLimit [0x0FFE].
            Expected: GT_BAD_PARAM (The limit's length is 10 bits).
        */
        limit = 0x0FFE;

        st = cpssDxChGlobalXonLimitSet(dev, limit);
        /*UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, limit);*/
    }

    limit = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGlobalXonLimitSet(dev, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChGlobalXonLimitSet(dev, limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGlobalXonLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xonLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChGlobalXonLimitGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with non-null xonLimitPtr.
Expected: GT_OK.
1.2. Call with xonLimitPtr [NULL].
Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    GT_U32                          limitGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL xonLimitPtr.
            Expected: GT_OK and the same xonLimit.
        */
        st = cpssDxChGlobalXonLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with xonLimitPtr [NULL].
            Expected: GT_OK and the same xonLimit.
        */
        st = cpssDxChGlobalXonLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d , NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGlobalXonLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChGlobalXonLimitGet(dev, &limitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGlobalXoffLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xoffLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChGlobalXoffLimitSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with xoffLimit [0x00EF].
Expected: GT_OK.
1.2. Call cpssDxChGlobalXoffLimitGet with non-NULL pointer.
Expected: GT_OK and the same xoffLimit.
1.3. Call with out of range xoffLimit[0x0FFE].
Expected: NON GT_OK.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    GT_U32                          limit    = 0;
    GT_U32                          limitGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with xoffLimit [0x00FE].
           Expected: GT_OK.
        */
        limit = 0x00FE;

        st = cpssDxChGlobalXoffLimitSet(dev, limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, limit);

        /*
            1.2. Call cpssDxChGlobalXoffLimitGet.
            Expected: GT_OK and the same xoffLimit.
        */
        st = cpssDxChGlobalXoffLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChGlobalXoffLimitGet: %d", dev);
        /*UTF_VERIFY_EQUAL1_STRING_MAC(limit, limitGet,
                       "get another xoffLimit than was set: %d", dev);*/

        /* 1.3. Call with xoffLimit [0x0FFE].
            Expected: GT_BAD_PARAM (The limit's length is 10 bits).
        */
        limit = 0x0FFE;

        st = cpssDxChGlobalXoffLimitSet(dev, limit);
        /*UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, limit);*/
    }

    limit = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGlobalXoffLimitSet(dev, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChGlobalXoffLimitSet(dev, limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGlobalXoffLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xoffLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChGlobalXoffLimitGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with non-null xonLimitPtr.
Expected: GT_OK.
1.2. Call with xonLimitPtr [NULL].
Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    GT_U32                          limitGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL xonLimitPtr.
            Expected: GT_OK and the same xonLimit.
        */
        st = cpssDxChGlobalXoffLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with xonLimitPtr [NULL].
            Expected: GT_OK and the same xonLimit.
        */
        st = cpssDxChGlobalXoffLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d , NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGlobalXoffLimitGet(dev, &limitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChGlobalXoffLimitGet(dev, &limitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxFcProfileSet
(
    IN GT_U8                            devNum,
    IN GT_U8                            portNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxFcProfileSet)
{
/*
    ITERATE_DEVICES_RXDMA_PORTS (DxChx)
    1.1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                CPSS_PORT_RX_FC_PROFILE_2_E /
                                CPSS_PORT_RX_FC_PROFILE_3_E /
                                CPSS_PORT_RX_FC_PROFILE_4_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortRxFcProfileGet
    Expected: GT_OK and the same profileSet.
    1.1.3. Call with profileSet[5]
    Expected: GT_BAD_PARAM.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: GT_OK.
*/

    GT_STATUS                       st     = GT_OK;
    GT_U8                           dev;
    GT_PHYSICAL_PORT_NUM            port    = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile = CPSS_PORT_RX_FC_PROFILE_1_E;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT pfGet   = CPSS_PORT_RX_FC_PROFILE_1_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                            CPSS_PORT_RX_FC_PROFILE_2_E /
                                            CPSS_PORT_RX_FC_PROFILE_3_E /
                                            CPSS_PORT_RX_FC_PROFILE_4_E].
               Expected: GT_OK.
            */

            /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
            profile = CPSS_PORT_RX_FC_PROFILE_1_E;

            st = cpssDxChPortRxFcProfileSet(dev, port, profile);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profile);

            /*
                1.1.2. Call cpssDxChPortRxFcProfileGet
                Expected: GT_OK and the same profileSet.
            */
            st = cpssDxChPortRxFcProfileGet(dev, port, &pfGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortRxFcProfileGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(profile, pfGet,
                       "get another state than was set: %d, %d", dev, port);


            /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
            profile = CPSS_PORT_RX_FC_PROFILE_2_E;

            st = cpssDxChPortRxFcProfileSet(dev, port, profile);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profile);

            /*
                1.1.2. Call cpssDxChPortRxFcProfileGet
                Expected: GT_OK and the same profileSet.
            */
            st = cpssDxChPortRxFcProfileGet(dev, port, &pfGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortRxFcProfileGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(profile, pfGet,
                       "get another state than was set: %d, %d", dev, port);

            /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
            profile = CPSS_PORT_RX_FC_PROFILE_3_E;

            st = cpssDxChPortRxFcProfileSet(dev, port, profile);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profile);

            /*
                1.1.2. Call cpssDxChPortRxFcProfileGet
                Expected: GT_OK and the same profileSet.
            */
            st = cpssDxChPortRxFcProfileGet(dev, port, &pfGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortRxFcProfileGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(profile, pfGet,
                       "get another state than was set: %d, %d", dev, port);

            /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_4_E] */
            profile = CPSS_PORT_RX_FC_PROFILE_4_E;

            st = cpssDxChPortRxFcProfileSet(dev, port, profile);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profile);

            /*
                1.1.2. Call cpssDxChPortRxFcProfileGet
                Expected: GT_OK and the same profileSet.
            */
            st = cpssDxChPortRxFcProfileGet(dev, port, &pfGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortRxFcProfileGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(profile, pfGet,
                       "get another state than was set: %d, %d", dev, port);

            /*
               1.1.3. Call with profileSet[wrong enum values]
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortRxFcProfileSet
                                (dev, port, profile),
                                profile);
        }

        profile = CPSS_PORT_RX_FC_PROFILE_2_E;

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */

            st = cpssDxChPortRxFcProfileSet(dev, port, profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* profile == CPSS_PORT_TX_SCHEDULER_PROFILE_2_E */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortRxFcProfileSet(dev, port, profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* profile == CPSS_PORT_RX_FC_PROFILE_2_E */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortRxFcProfileSet(dev, port, profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    profile = CPSS_PORT_RX_FC_PROFILE_2_E;
    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxFcProfileSet(dev, port, profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, profile == CPSS_PORT_RX_FC_PROFILE_2_E */

    st = cpssDxChPortRxFcProfileSet(dev, port, profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxFcProfileGet
(
    IN  GT_U8                            devNum,
    IN  GT_U8                            portNum,
    OUT CPSS_PORT_RX_FC_PROFILE_SET_ENT  *profileSetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxFcProfileGet)
{
/*
    ITERATE_DEVICES_RXDMA_PORTS (DxChx)
    1.1.1. Call with non-null profileSetPtr.
    Expected: GT_OK.
    1.1.2. Call with profileSetPtr [NULL].
    Expected: GT_BAD_PTR.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: GT_OK.
*/
    GT_STATUS                       st      = GT_OK;

    GT_U8                           dev;
    GT_PHYSICAL_PORT_NUM            port    = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile = CPSS_PORT_RX_FC_PROFILE_1_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null profileSetPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortRxFcProfileGet(dev, port, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with profileSetPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortRxFcProfileGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while (GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortRxFcProfileGet(dev, port, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortRxFcProfileGet(dev, port, &profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortRxFcProfileGet(dev, port, &profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxFcProfileGet(dev, port, &profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortRxFcProfileGet(dev, port, &profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXonLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U16                           xonLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXonLimitSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                               CPSS_PORT_RX_FC_PROFILE_2_E /
                               CPSS_PORT_RX_FC_PROFILE_3_E /
                               CPSS_PORT_RX_FC_PROFILE_4_E ]
                   and xonLimit[8,16,32, 0xFFFE].
    Expected: GT_OK for xonLimit[8,16,32] and GT_BAD_PARAM for xonLimit[0xFFFE]
    1.2. Call cpssDxChPortXonLimitGet with profileSet from 1.1.
    Expected: GT_OK and the same xonLimit.
    1.3. Call with profileSet[5]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile  = CPSS_PORT_RX_FC_PROFILE_1_E;
    GT_U32                          limit    = 0;
    GT_U32                          limitGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                      CPSS_PORT_RX_FC_PROFILE_2_E /
                                      CPSS_PORT_RX_FC_PROFILE_3_E /
                                      CPSS_PORT_RX_FC_PROFILE_4_E]
                          and xonLimit[8,16,32, 0xFFFE].
           Expected: GT_OK.
        */

        /*
            1.2. Call cpssDxChPortXonLimitGet with profileSet from 1.1.
            Expected: GT_OK and the same xonLimit.
        */

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        /* xonLimit[8]                                             */
        profile = CPSS_PORT_RX_FC_PROFILE_1_E;
        limit = 8;

        st = cpssDxChPortXonLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        st = cpssDxChPortXonLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortXonLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
                       "get another xonLimit than was set: %d, %d", dev, profile);

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        /* xonLimit[16]                                            */
        profile = CPSS_PORT_RX_FC_PROFILE_2_E;
        limit = 16;

        st = cpssDxChPortXonLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        st = cpssDxChPortXonLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortXonLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
                       "get another xonLimit than was set: %d, %d", dev, profile);

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        /* xonLimit[32]                                            */
        profile = CPSS_PORT_RX_FC_PROFILE_3_E;
        limit = 32;

        st = cpssDxChPortXonLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        st = cpssDxChPortXonLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortXonLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
                       "get another xonLimit than was set: %d, %d", dev, profile);

        /* 1.3. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_4_E] */
        /* xonLimit[0xFFFE]. As limit should have length 11 bits  */
        /* it is expected : GT_BAD_PARAM*/
        profile = CPSS_PORT_RX_FC_PROFILE_4_E;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            limit = BIT_16;
        }
        else
        {
            limit = ((PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)) ? 0x1FFD : 0x0FFE;
        }

        st = cpssDxChPortXonLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, limit);

        /*
            1.4. Call with wrong enum values profileSet, limit [10],
            Expected: GT_BAD_PARAM.
        */
        limit = 10;

        UTF_ENUMS_CHECK_MAC(cpssDxChPortXonLimitSet
                            (dev, profile, limit),
                            profile);
    }

    profile = CPSS_PORT_RX_FC_PROFILE_1_E;
    limit = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXonLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXonLimitSet(dev, profile, limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXonLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U16                          *xonLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXonLimitGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                               CPSS_PORT_RX_FC_PROFILE_2_E /
                               CPSS_PORT_RX_FC_PROFILE_3_E /
                               CPSS_PORT_RX_FC_PROFILE_4_E]
                   and non-null xonLimitPtr.
    Expected: GT_OK.
    1.2. Call with xonLimitPtr [NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with profileSet[5] and non-null xonLimitPtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st      = GT_OK;

    GT_U8                           dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile = CPSS_PORT_RX_FC_PROFILE_1_E;
    GT_U32                          limit   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                      CPSS_PORT_RX_FC_PROFILE_2_E /
                                      CPSS_PORT_RX_FC_PROFILE_3_E /
                                      CPSS_PORT_RX_FC_PROFILE_4_E]
                          and non-null xonLimitPtr.
           Expected: GT_OK.
        */

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_1_E;

        st = cpssDxChPortXonLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_2_E;

        st = cpssDxChPortXonLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_3_E;

        st = cpssDxChPortXonLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_4_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_4_E;

        st = cpssDxChPortXonLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /*
            1.2. Call with xonLimitPtr [NULL] and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */

        /* profileSet == CPSS_PORT_RX_FC_PROFILE_4_E */
        st = cpssDxChPortXonLimitGet(dev, profile, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, profile);

        /*
            1.3. Call with profileSet[wrong enum values] and non-null xonLimitPtr.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortXonLimitGet
                            (dev, profile, &limit),
                            profile);
    }

    profile = CPSS_PORT_RX_FC_PROFILE_1_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXonLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXonLimitGet(dev, profile, &limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXoffLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U16                           xoffLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXoffLimitSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                               CPSS_PORT_RX_FC_PROFILE_2_E /
                               CPSS_PORT_RX_FC_PROFILE_3_E /
                               CPSS_PORT_RX_FC_PROFILE_4_E]
                   and xoffLimit[8, 16, 32, 0xF333].
    Expected: GT_OK for xoffLimit[8, 16, 32] and GT_BAD_PARAM for xoffLimit[0xF333]
    1.2. Call cpssDxChPortXoffLimitGet with profileSet from 1.1.
    Expected: GT_OK and the same xoffLimit.
    1.3. Call with profileSet[5]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile  = CPSS_PORT_RX_FC_PROFILE_1_E;
    GT_U32                          limit    = 0;
    GT_U32                          limitGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                      CPSS_PORT_RX_FC_PROFILE_2_E /
                                      CPSS_PORT_RX_FC_PROFILE_3_E /
                                      CPSS_PORT_RX_FC_PROFILE_4_E]
                          and xoffLimit[8,16,32, 0xF333].
           Expected: GT_OK.
        */

        /*
            1.2. Call cpssDxChPortXoffLimitGet with profileSet from 1.1.
            Expected: GT_OK and the same xonLimit.
        */

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        /* xoffLimit[8]                                            */
        profile = CPSS_PORT_RX_FC_PROFILE_1_E;
        limit = 8;

        st = cpssDxChPortXoffLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        st = cpssDxChPortXoffLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortXoffLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
                       "get another xoffLimit than was set: %d, %d", dev, profile);

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        /* xoffLimit[16]                                           */
        profile = CPSS_PORT_RX_FC_PROFILE_2_E;
        limit = 16;

        st = cpssDxChPortXoffLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        st = cpssDxChPortXoffLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortXoffLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
                       "get another xoffLimit than was set: %d, %d", dev, profile);

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        /* xoffLimit[32]                                           */
        profile = CPSS_PORT_RX_FC_PROFILE_3_E;
        limit = 32;

        st = cpssDxChPortXoffLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        st = cpssDxChPortXoffLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChPortXoffLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
                       "get another xoffLimit than was set: %d, %d", dev, profile);

        /* 1.3. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_4_E] */
        /* xoffLimit[0x0FFE]. As limit should have length 11 bits  */
        /* it is expected : GT_BAD_PARAM*/
        profile = CPSS_PORT_RX_FC_PROFILE_4_E;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            limit = BIT_16;
        }
        else
        {
            limit = ((PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)) ? 0x1FFD : 0x0FFE; /* The limit should have length 11 bits */
        }

        st = cpssDxChPortXoffLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, limit);

        /*
            1.4. Call with wrong enum values profileSet, limit [10],
            Expected: GT_BAD_PARAM.
        */
        limit = 10;

        UTF_ENUMS_CHECK_MAC(cpssDxChPortXoffLimitSet
                            (dev, profile, limit),
                            profile);
    }

    profile = CPSS_PORT_RX_FC_PROFILE_1_E;
    limit = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXoffLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXoffLimitSet(dev, profile, limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortXoffLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U16                          *xoffLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortXoffLimitGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                               CPSS_PORT_RX_FC_PROFILE_2_E /
                               CPSS_PORT_RX_FC_PROFILE_3_E /
                               CPSS_PORT_RX_FC_PROFILE_4_E]
                   and non-null xoffLimitPtr.
    Expected: GT_OK.
    1.2. Call with xoffLimitPtr [NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with profileSet[5] and non-null xoffLimitPtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile  = CPSS_PORT_RX_FC_PROFILE_1_E;
    GT_U32                          limit    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                      CPSS_PORT_RX_FC_PROFILE_2_E /
                                      CPSS_PORT_RX_FC_PROFILE_3_E /
                                      CPSS_PORT_RX_FC_PROFILE_4_E]
                          and non-null xoffLimitPtr.
           Expected: GT_OK.
        */

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_1_E;

        st = cpssDxChPortXoffLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_2_E;

        st = cpssDxChPortXoffLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_3_E;

        st = cpssDxChPortXoffLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_4_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_4_E;

        st = cpssDxChPortXoffLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /*
            1.2. Call with xoffLimitPtr [NULL] and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */

        /* profileSet == CPSS_PORT_RX_FC_PROFILE_4_E */
        st = cpssDxChPortXoffLimitGet(dev, profile, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, profile);

        /*
            1.3. Call with profileSet[wrong enum values] and non-null xoffLimitPtr.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortXoffLimitGet
                            (dev, profile, &limit),
                            profile);
    }

    profile = CPSS_PORT_RX_FC_PROFILE_1_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortXoffLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortXoffLimitGet(dev, profile, &limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxBufLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U16                           rxBufLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxBufLimitSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                               CPSS_PORT_RX_FC_PROFILE_2_E /
                               CPSS_PORT_RX_FC_PROFILE_3_E /
                               CPSS_PORT_RX_FC_PROFILE_4_E]
                   and rxBufLimit [16,32,48,0x7FE].
    Expected: GT_OK for rxBufLimit [16,32,48] and GT_BAD_PARAM for rxBufLimit[0x7FE]
    1.2. Call cpssDxChPortRxBufLimitGet with profileSet from 1.1.
    Expected: GT_OK and the same rxBufLimit.
    1.3. Call with profileSet[wrong enum values]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile  = CPSS_PORT_RX_FC_PROFILE_1_E;
    GT_U32                          limit    = 0;
    GT_U32                          limitGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                      CPSS_PORT_RX_FC_PROFILE_2_E /
                                      CPSS_PORT_RX_FC_PROFILE_3_E /
                                      CPSS_PORT_RX_FC_PROFILE_4_E]
                          and rxBufLimit [16 / 32 / 48 / 0x7FE].
           Expected: GT_OK.
        */

        /*
            1.2. Call cpssDxChPortRxBufLimitGet with profileSet from 1.1.
            Expected: GT_OK and the same rxBufLimit.
        */

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        /* rxBufLimit[16]                                          */
        profile = CPSS_PORT_RX_FC_PROFILE_1_E;
        limit = 16;

        st = cpssDxChPortRxBufLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        st = cpssDxChPortRxBufLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChPortRxBufLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
            "get another rxBufLimit than was set: %d, %d", dev, profile);

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        /* rxBufLimit[32]                                          */
        profile = CPSS_PORT_RX_FC_PROFILE_2_E;
        limit = 32;

        st = cpssDxChPortRxBufLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        st = cpssDxChPortRxBufLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChPortRxBufLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
            "get another rxBufLimit than was set: %d, %d", dev, profile);

        /* 1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        /* rxBufLimit[48]                                          */
        profile = CPSS_PORT_RX_FC_PROFILE_3_E;
        limit = 48;

        st = cpssDxChPortRxBufLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, limit);

        /* 1.2. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        st = cpssDxChPortRxBufLimitGet(dev, profile, &limitGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChPortRxBufLimitGet: %d, %d", dev, profile);
        UTF_VERIFY_EQUAL2_STRING_MAC(limit, limitGet,
            "get another rxBufLimit than was set: %d, %d", dev, profile);

        /* 1.3. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_4_E] */
        /* rxBufLimit[0x7FE] As limit should have length 10 bits  */
        /* it is expected : GT_BAD_PARAM*/
        profile = CPSS_PORT_RX_FC_PROFILE_4_E;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            limit = BIT_16;
        }
        else
        {
            limit = ((PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)) ? 0xFFD : 0x07FE;
        }

        st = cpssDxChPortRxBufLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, limit);

        /*
            1.3. Call with wrong enum values profileSet, rxBufLimit [10],
            Expected: GT_BAD_PARAM.
        */
        limit = 10;

        UTF_ENUMS_CHECK_MAC(cpssDxChPortRxBufLimitSet
                            (dev, profile, limit),
                            profile);
    }

    profile = CPSS_PORT_RX_FC_PROFILE_1_E;
    limit = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxBufLimitSet(dev, profile, limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortRxBufLimitSet(dev, profile, limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxBufLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U16                          *rxBufLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxBufLimitGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                               CPSS_PORT_RX_FC_PROFILE_2_E /
                               CPSS_PORT_RX_FC_PROFILE_3_E /
                               CPSS_PORT_RX_FC_PROFILE_4_E]
                   and non-null rxBufLimitPtr.
    Expected: GT_OK.
    1.2. Call with rxBufLimitPtr [NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with profileSet[wrong enum values] and non-null rxBufLimitPtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT profile  = CPSS_PORT_RX_FC_PROFILE_1_E;
    GT_U32                          limit    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E /
                                      CPSS_PORT_RX_FC_PROFILE_2_E /
                                      CPSS_PORT_RX_FC_PROFILE_3_E /
                                      CPSS_PORT_RX_FC_PROFILE_4_E]
                          and non-null rxBufLimitPtr.
           Expected: GT_OK.
        */

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_1_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_1_E;

        st = cpssDxChPortRxBufLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_2_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_2_E;

        st = cpssDxChPortRxBufLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_3_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_3_E;

        st = cpssDxChPortRxBufLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /* Call with profileSet [CPSS_PORT_RX_FC_PROFILE_4_E] */
        profile = CPSS_PORT_RX_FC_PROFILE_4_E;

        st = cpssDxChPortRxBufLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

        /*
            1.2. Call with rxBufLimitPtr [NULL] and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */

        /* profileSet == CPSS_PORT_RX_FC_PROFILE_4_E */
        st = cpssDxChPortRxBufLimitGet(dev, profile, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, profile);

        /*
            1.3. Call with profileSet[wrong enum values] and non-null rxBufLimitPtr.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortRxBufLimitGet
                            (dev, profile, &limit),
                            profile);
    }

    profile = CPSS_PORT_RX_FC_PROFILE_1_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxBufLimitGet(dev, profile, &limit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortRxBufLimitGet(dev, profile, &limit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCpuRxBufCountGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *cpuRxBufCntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCpuRxBufCountGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with non-null cpuRxBufCntPtr.
    Expected: GT_OK.
    1.2. Call with cpuRxBufCntPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st     = GT_OK;

    GT_U8      dev;
    GT_U16     cpuBuf = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null cpuRxBufCntPtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortCpuRxBufCountGet(dev, &cpuBuf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with cpuRxBufCntPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCpuRxBufCountGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCpuRxBufCountGet(dev, &cpuBuf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCpuRxBufCountGet(dev, &cpuBuf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFcHolSysModeSet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT modeFcHol
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortFcHolSysModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with modeFcHol [CPSS_DXCH_PORT_FC_E /
                              CPSS_DXCH_PORT_HOL_E].
    Expected: GT_OK.
    1.2. Call with modeFcHol [5]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                 st   = GT_OK;

    GT_U8                     dev;
    CPSS_DXCH_PORT_HOL_FC_ENT mode = CPSS_DXCH_PORT_FC_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with modeFcHol [CPSS_DXCH_PORT_FC_E /
                                     CPSS_DXCH_PORT_HOL_E].
           Expected: GT_OK.
        */

        /* Call with modeFcHol [CPSS_DXCH_PORT_FC_E] */
        mode = CPSS_DXCH_PORT_FC_E;

        st = cpssDxChPortFcHolSysModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* Call with modeFcHol [CPSS_DXCH_PORT_HOL_E] */
        mode = CPSS_DXCH_PORT_HOL_E;

        st = cpssDxChPortFcHolSysModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call with modeFcHol [wrong enum values]
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortFcHolSysModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DXCH_PORT_HOL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortFcHolSysModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortFcHolSysModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortBuffersModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_PORT_BUFFERS_MODE_ENT  bufferMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortBuffersModeSet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with bufferMode [CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E /
                              CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E].
    Expected: GT_OK.
    1.2. Call cpssDxChPortBuffersModeGet with non-NULL bufferModeGet
    Expected: GT_OK and same bufferModeGet that was written
    1.3. Call with wrong enum values bufferMode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_BUFFERS_MODE_ENT bufferMode    = CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E;
    CPSS_DXCH_PORT_BUFFERS_MODE_ENT bufferModeGet = CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with bufferMode [CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E /
                                      CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E].
            Expected: GT_OK.
        */
        /* call with bufferMode = CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E */
        bufferMode = CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E;

        st = cpssDxChPortBuffersModeSet(dev, bufferMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bufferMode);

        /*
            1.2. Call cpssDxChPortBuffersModeGet with non-NULL bufferModeGet
            Expected: GT_OK and same bufferModeGet that was written
        */
        st = cpssDxChPortBuffersModeGet(dev, &bufferModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortBuffersModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(bufferMode, bufferModeGet, "got another bufferMode than was set: %d", dev);

        /* call with bufferMode = CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E */
        bufferMode = CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E;

        st = cpssDxChPortBuffersModeSet(dev, bufferMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bufferMode);

        /*
            1.2. Call cpssDxChPortBuffersModeGet with non-NULL bufferModeGet
            Expected: GT_OK and same bufferModeGet that was written
        */
        st = cpssDxChPortBuffersModeGet(dev, &bufferModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortBuffersModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(bufferMode, bufferModeGet, "got another bufferMode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values bufferMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortBuffersModeSet
                            (dev, bufferMode),
                            bufferMode);
    }

    bufferMode = CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortBuffersModeSet(dev, bufferMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortBuffersModeSet(dev, bufferMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortBuffersModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_PORT_BUFFERS_MODE_ENT *bufferModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortBuffersModeGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with non-NULL bufferMode
    Expected: GT_OK.
    1.2. Call with bufferMode [NULL]
              and other parameter form 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                 st   = GT_OK;
    GT_U8                     dev;

    CPSS_DXCH_PORT_BUFFERS_MODE_ENT bufferMode = CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL bufferMode
            Expected: GT_OK.
        */
        st = cpssDxChPortBuffersModeGet(dev, &bufferMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with bufferMode [NULL]
                      and other parameter form 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortBuffersModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bufferMode = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortBuffersModeGet(dev, &bufferMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortBuffersModeGet(dev, &bufferMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupXonLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xonLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupXonLimitSet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                              CPSS_DXCH_PORT_GROUP_HGS_E],
                   xonLimit [0 / 0xFFF]
    Expected: GT_OK.
    1.2. Call cpssDxChPortGroupXonLimitGet with non-NULL xonLimitGet
              and other parameters from 1.1.
    Expected: GT_OK and same xonLimitGet that was written
    1.3. Call with wrong enum values portGroup
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range xonLimit [0x1000](for DXCH and DXCH2) or
              xonLimit [0x4000](for DXCH3 and above)
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_U8                       dev;
    GT_STATUS                   st              = GT_OK;
    CPSS_DXCH_PORT_GROUP_ENT    portGroup       = CPSS_DXCH_PORT_GROUP_GIGA_E;
    GT_U32                      xonLimit        = 0;
    GT_U32                      xonLimitGet     = 0;
    GT_U32                      xonResolution   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
        /* For DXCH3 and above, the field resolution is of 8 buffers,*/
        /* which we are rounding down */
        if ((PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily>= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            xonResolution = 3;
        }
        else
        {
            xonResolution = 1;
        }

        /*
            1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                                      CPSS_DXCH_PORT_GROUP_HGS_E],
                           xonLimit [0 / 0xFFF]
            Expected: GT_OK.
        */
        /* call with portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E */
        portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
        xonLimit  = 0;

        st = cpssDxChPortGroupXonLimitSet(dev, portGroup, xonLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroup, xonLimit);
        /*
            1.2. Call cpssDxChPortGroupXonLimitGet with non-NULL xonLimitGet
                      and other parameters from 1.1.
            Expected: GT_OK and same xonLimitGet that was written
        */
        st = cpssDxChPortGroupXonLimitGet(dev, portGroup, &xonLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortGroupXonLimitGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(xonLimit >> xonResolution,
                                     xonLimitGet >> xonResolution,
                                     "got another xonLimitGet than was set: %d",
                                     dev);

        /* call with portGroup = CPSS_DXCH_PORT_GROUP_HGS_E */
        portGroup = CPSS_DXCH_PORT_GROUP_HGS_E;
        xonLimit  = 0xFFE;

        st = cpssDxChPortGroupXonLimitSet(dev, portGroup, xonLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroup, xonLimit);

        /*
            1.2. Call cpssDxChPortGroupXonLimitGet with non-NULL xonLimitGet
                      and other parameters from 1.1.
            Expected: GT_OK and same xonLimitGet that was written
        */
        st = cpssDxChPortGroupXonLimitGet(dev, portGroup, &xonLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortGroupXonLimitGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(xonLimit >> xonResolution,
                                     xonLimitGet >> xonResolution,
                                     "got another xonLimitGet than was set: %d",
                                     dev);

        /*
            1.3. Call with wrong enum values portGroup and
                other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupXonLimitSet
                            (dev, portGroup, xonLimit),
                            portGroup);

        /*
            1.4. Call with out of range xoffLimit [0x1000](for DXCH and DXCH2) or
                xoffLimit [0x4000](for DXCH3 and above)
                and other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        xonLimit = 0x800 << xonResolution;

        st = cpssDxChPortGroupXonLimitSet(dev, portGroup, xonLimit);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, xonLimit = %d",
                                     dev, xonLimit);

        xonLimit = 0;
    }

    portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
    xonLimit  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupXonLimitSet(dev, portGroup, xonLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupXonLimitSet(dev, portGroup, xonLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupXonLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xonLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupXonLimitGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                              CPSS_DXCH_PORT_GROUP_HGS_E],
                   and non-NULL xonLimit
    Expected: GT_OK.
    1.2. Call with wrong enum values portGroup
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with xonLimit [NULL]
              and other parameters form 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st          = GT_OK;
    GT_U8                       dev;
    CPSS_DXCH_PORT_GROUP_ENT    portGroup   = CPSS_DXCH_PORT_GROUP_GIGA_E;
    GT_U32                      xonLimit    = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                                      CPSS_DXCH_PORT_GROUP_HGS_E],
                           and non-NULL xonLimit
            Expected: GT_OK.
        */
        /* call with portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E */
        portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;

        st = cpssDxChPortGroupXonLimitGet(dev, portGroup, &xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroup);

        /* call with portGroup = CPSS_DXCH_PORT_GROUP_HGS_E */
        portGroup = CPSS_DXCH_PORT_GROUP_HGS_E;

        st = cpssDxChPortGroupXonLimitGet(dev, portGroup, &xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroup);

        /*
            1.2. Call with wrong enum values portGroup and
                other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupXonLimitGet
                            (dev, portGroup, &xonLimit),
                            portGroup);

        /*
            1.3. Call with xonLimit [NULL]
                      and other parameters form 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortGroupXonLimitGet(dev, portGroup, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xonLimit = %d", dev);
    }

    portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupXonLimitGet(dev, portGroup, &xonLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupXonLimitGet(dev, portGroup, &xonLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupXoffLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xoffLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupXoffLimitSet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                              CPSS_DXCH_PORT_GROUP_HGS_E],
                   xoffLimit [0 / 0xFFF]
    Expected: GT_OK.
    1.2. Call cpssDxChPortGroupXoffLimitGet with non-NULL xoffLimitGet
              and other parameters from 1.1.
    Expected: GT_OK and same xoffLimitGet that was written
    1.3. Call with wrong enum values portGroup
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range xoffLimit [0x1000](for DXCH and DXCH2) or
              xoffLimit [0x4000](for DXCH3 and above)
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_U8                       dev;
    GT_STATUS                   st              = GT_OK;
    CPSS_DXCH_PORT_GROUP_ENT    portGroup       = CPSS_DXCH_PORT_GROUP_GIGA_E;
    GT_U32                      xoffLimit       = 0;
    GT_U32                      xoffLimitGet    = 0;
    GT_U32                      xoffResolution  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
        /* For DXCH3 and above, the field resolution is of 8 buffers,*/
        /* which we are rounding down */
        if ((PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily>= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            xoffResolution = 3;
        }
        else
        {
            xoffResolution = 1;
        }

        /*
            1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                                      CPSS_DXCH_PORT_GROUP_HGS_E],
                           xoffLimit [0 / 0xFFF]
            Expected: GT_OK.
        */
        /* call with portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E */
        portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
        xoffLimit = 0;

        st = cpssDxChPortGroupXoffLimitSet(dev, portGroup, xoffLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroup, xoffLimit);

        /*
            1.2. Call cpssDxChPortGroupXoffLimitGet with non-NULL xoffLimitGet
                      and other parameters from 1.1.
            Expected: GT_OK and same xoffLimitGet that was written
        */
        st = cpssDxChPortGroupXoffLimitGet(dev, portGroup, &xoffLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortGroupXoffLimitGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(xoffLimit >> xoffResolution,
                                     xoffLimitGet >> xoffResolution,
                                    "got another xoffLimitGet than was set: %d",
                                    dev);

        /* call with portGroup = CPSS_DXCH_PORT_GROUP_HGS_E */
        portGroup = CPSS_DXCH_PORT_GROUP_HGS_E;
        xoffLimit = 0xFFE;

        st = cpssDxChPortGroupXoffLimitSet(dev, portGroup, xoffLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroup, xoffLimit);

        /*
            1.2. Call cpssDxChPortGroupXoffLimitGet with non-NULL xoffLimitGet
                      and other parameters from 1.1.
            Expected: GT_OK and same xoffLimitGet that was written
        */
        st = cpssDxChPortGroupXoffLimitGet(dev, portGroup, &xoffLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortGroupXoffLimitGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(xoffLimit >> xoffResolution,
                                     xoffLimitGet  >> xoffResolution,
                                    "got another xoffLimitGet than was set: %d",
                                    dev);

        /*
            1.3. Call with wrong enum values portGroup and
                other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupXoffLimitSet
                            (dev, portGroup, xoffLimit),
                            portGroup);

        /*
            1.4. Call with out of range xoffLimit [0x1000](for DXCH and DXCH2) or
                xoffLimit [0x4000](for DXCH3 and above)
                and other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        xoffLimit = 0x800 << xoffResolution;

        st = cpssDxChPortGroupXoffLimitSet(dev, portGroup, xoffLimit);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                     "%d, xoffLimit = %d", dev, xoffLimit);

        xoffLimit = 0;
    }

    portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
    xoffLimit = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupXoffLimitSet(dev, portGroup, xoffLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupXoffLimitSet(dev, portGroup, xoffLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupXoffLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xoffLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupXoffLimitGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                              CPSS_DXCH_PORT_GROUP_HGS_E],
                   and non-NULL xoffLimit
    Expected: GT_OK.
    1.3. Call with wrong enum values portGroup
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with xoffLimit [NULL]
              and other parameters form 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                 st   = GT_OK;
    GT_U8                     dev;

    CPSS_DXCH_PORT_GROUP_ENT    portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
    GT_U32                      xoffLimit = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                                      CPSS_DXCH_PORT_GROUP_HGS_E],
                           and non-NULL xoffLimit
            Expected: GT_OK.
        */
        /* call with portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E */
        portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;

        st = cpssDxChPortGroupXoffLimitGet(dev, portGroup, &xoffLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroup);

        /* call with portGroup = CPSS_DXCH_PORT_GROUP_HGS_E */
        portGroup = CPSS_DXCH_PORT_GROUP_HGS_E;

        st = cpssDxChPortGroupXoffLimitGet(dev, portGroup, &xoffLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroup);

        /*
            1.3. Call with wrong enum values portGroup and
                other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupXoffLimitGet
                            (dev, portGroup, &xoffLimit),
                            portGroup);

        /*
            1.3. Call with xoffLimit [NULL]
                      and other parameters form 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortGroupXoffLimitGet(dev, portGroup, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st,
                                    "%d, xoffLimit = NULL", dev);
    }

    portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupXoffLimitGet(dev, portGroup, &xoffLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupXoffLimitGet(dev, portGroup, &xoffLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupRxBufLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    rxBufLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupRxBufLimitSet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                              CPSS_DXCH_PORT_GROUP_HGS_E],
                   rxBufLimit [0 / 0xFFF]
    Expected: GT_OK.
    1.2. Call cpssDxChPortGroupRxBufLimitGet.
    Expected: GT_OK and same rxBufLimitGet that was written
    1.3. Call with wrong enum values portGroup
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range rxBufLimit [0x1000] (for DXCH and DXCH2)
              or rxBufLimit [0x4000] (for DXCH3 and above)
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_U8                       dev;
    GT_STATUS                   st              = GT_OK;
    CPSS_DXCH_PORT_GROUP_ENT    portGroup       = CPSS_DXCH_PORT_GROUP_GIGA_E;
    GT_U32                      rxBufLimit      = 0;
    GT_U32                      rxBufLimitGet   = 0;
    GT_U32                      limitResolution = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* For DXCH and DXCH2 devices the field resolution is of 4 buffers */
        /* For DXCH3 and above, the field resolution is of 16 buffers,*/
        /* which we are rounding down */
        if (PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
            limitResolution = 4;
        }
        else
        {
            limitResolution = 2;
        }

        /*
            1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                                      CPSS_DXCH_PORT_GROUP_HGS_E],
                           rxBufLimit [0 / 0xFFF]
            Expected: GT_OK.
        */

        /* call with portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E */
        portGroup  = CPSS_DXCH_PORT_GROUP_GIGA_E;
        rxBufLimit = 0;

        st = cpssDxChPortGroupRxBufLimitSet(dev, portGroup, rxBufLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroup, rxBufLimit);

        /*
            1.2. Call cpssDxChPortGroupRxBufLimitGet.
            Expected: GT_OK and same rxBufLimitGet that was written
        */
        st = cpssDxChPortGroupRxBufLimitGet(dev, portGroup, &rxBufLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortGroupRxBufLimitGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(rxBufLimit >> limitResolution,
                                     rxBufLimitGet >> limitResolution,
                                     "got another rxBufLimit than was set: %d",
                                     dev);

        /* call with portGroup = CPSS_DXCH_PORT_GROUP_HGS_E */
        portGroup  = CPSS_DXCH_PORT_GROUP_HGS_E;
        rxBufLimit = 0xFFC;

        st = cpssDxChPortGroupRxBufLimitSet(dev, portGroup, rxBufLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroup, rxBufLimit);

        /*
            1.2. Call cpssDxChPortGroupRxBufLimitGet.
            Expected: GT_OK and same rxBufLimitGet that was written
        */
        st = cpssDxChPortGroupRxBufLimitGet(dev, portGroup, &rxBufLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChPortGroupRxBufLimitGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(rxBufLimit >> limitResolution,
                                     rxBufLimitGet >> limitResolution,
                                     "got another rxBufLimit than was set: %d",
                                     dev);

        /*
            1.3. Call with wrong enum values portGroup and
                other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupRxBufLimitSet
                            (dev, portGroup, rxBufLimit),
                            portGroup);

        /*
            1.4. Call with out of range rxBufLimit [0x1000] (for DXCH and DXCH2)
                  or rxBufLimit [0x4000] (for DXCH3 and above)
                  and other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        rxBufLimit = ((GT_U32)(BIT_10)) << limitResolution;

        st = cpssDxChPortGroupRxBufLimitSet(dev, portGroup, rxBufLimit);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, rxBufLimit = %d",
                                         dev, rxBufLimit);
    }

    portGroup  = CPSS_DXCH_PORT_GROUP_GIGA_E;
    rxBufLimit = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupRxBufLimitSet(dev, portGroup, rxBufLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupRxBufLimitSet(dev, portGroup, rxBufLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupRxBufLimitGet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32                    *rxBufLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupRxBufLimitGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                              CPSS_DXCH_PORT_GROUP_HGS_E],
                   and non-NULL rxBufLimit
    Expected: GT_OK.
    1.2. Call with wrong enum values portGroup
              and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with rxBufLimit [NULL]
              and other parameters form 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                 st   = GT_OK;
    GT_U8                     dev;

    CPSS_DXCH_PORT_GROUP_ENT    portGroup  = CPSS_DXCH_PORT_GROUP_GIGA_E;
    GT_U32                      rxBufLimit = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E /
                                      CPSS_DXCH_PORT_GROUP_HGS_E],
                           and non-NULL rxBufLimit
            Expected: GT_OK.
        */
        /* call with portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E*/
        portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;

        st = cpssDxChPortGroupRxBufLimitGet(dev, portGroup, &rxBufLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroup);

        /* call with portGroup = CPSS_DXCH_PORT_GROUP_HGS_E*/
        portGroup = CPSS_DXCH_PORT_GROUP_HGS_E;

        st = cpssDxChPortGroupRxBufLimitGet(dev, portGroup, &rxBufLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroup);

        /*
            1.2. Call with wrong enum values portGroup and other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupRxBufLimitGet
                            (dev, portGroup, &rxBufLimit),
                            portGroup);

        /*
            1.3. Call with rxBufLimit [NULL]
                      and other parameters form 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortGroupRxBufLimitGet(dev, portGroup, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rxBufLimit = NULL", dev);
    }

    portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupRxBufLimitGet(dev, portGroup, &rxBufLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupRxBufLimitGet(dev, portGroup, &rxBufLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCrossChipFcPacketRelayEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCrossChipFcPacketRelayEnableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with enable [GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.2. Call cpssDxChPortCrossChipFcPacketRelayEnableGet with non-NULL enableGet
              and other parameters from 1.1.
    Expected: GT_OK and same enableGet that was written
*/
    GT_STATUS                 st   = GT_OK;
    GT_U8                     dev;

    GT_BOOL  enable    = GT_FALSE;
    GT_BOOL  enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE / GT_FALSE],
            Expected: GT_OK.
        */
        /* call with enable = GT_TRUE*/
        enable = GT_TRUE;

        st = cpssDxChPortCrossChipFcPacketRelayEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChPortCrossChipFcPacketRelayEnableGet with non-NULL enableGet
                      and other parameters from 1.1.
            Expected: GT_OK and same enableGet that was written
        */
        st = cpssDxChPortCrossChipFcPacketRelayEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCrossChipFcPacketRelayEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableGet, enable, "got another enableGet than was set: %d", dev);

        /* call with enable = GT_FALSE*/
        enable = GT_FALSE;

        st = cpssDxChPortCrossChipFcPacketRelayEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChPortCrossChipFcPacketRelayEnableGet with non-NULL enableGet
                      and other parameters from 1.1.
            Expected: GT_OK and same enableGet that was written
        */
        st = cpssDxChPortCrossChipFcPacketRelayEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCrossChipFcPacketRelayEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableGet, enable, "got another enableGet than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCrossChipFcPacketRelayEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCrossChipFcPacketRelayEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCrossChipFcPacketRelayEnableGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCrossChipFcPacketRelayEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL enable
    Expected: GT_OK.
    1.2. Call with enable [NULL]
    Expected: GT_BAD_PTR
*/
    GT_STATUS                 st   = GT_OK;
    GT_U8                     dev;

    GT_BOOL enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL enable
            Expected: GT_OK.
        */
        st = cpssDxChPortCrossChipFcPacketRelayEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enable [NULL]
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPortCrossChipFcPacketRelayEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCrossChipFcPacketRelayEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCrossChipFcPacketRelayEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortClearByBufferEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortClearByBufferEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortClearByBufferEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_PHYSICAL_PORT_NUM  port      = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChPortClearByBufferEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. Call cpssDxChPortClearByBufferEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortClearByBufferEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChPortClearByBufferEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                         "get another enable value than was set: %d, %d", dev, port);

            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChPortClearByBufferEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. Call cpssDxChPortClearByBufferEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortClearByBufferEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChPortClearByBufferEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                         "get another enable value than was set: %d, %d", dev, port);
        }

        /*
            1.2. For all active devices go over all non available
            physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssDxChPortClearByBufferEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortClearByBufferEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortClearByBufferEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortClearByBufferEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortClearByBufferEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortClearByBufferEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortClearByBufferEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_PHYSICAL_PORT_NUM port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null enable.
               Expected: GT_OK.
            */
            st = cpssDxChPortClearByBufferEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enable [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortClearByBufferEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortClearByBufferEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortClearByBufferEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortClearByBufferEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortClearByBufferEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortClearByBufferEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGlobalPacketNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfPacketsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGlobalPacketNumberGet)
{
/*
    ITERATE_DEVICES(DxCh3 and above)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st           = GT_OK;
    GT_U8       dev;
    GT_U32      numOfPackets = 0;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enable.
            Expected: GT_OK.
        */
        st = cpssDxChPortGlobalPacketNumberGet(dev, &numOfPackets);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enable [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPortGlobalPacketNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGlobalPacketNumberGet(dev, &numOfPackets);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGlobalPacketNumberGet(dev, &numOfPackets);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGlobalRxBufNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfBuffersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGlobalRxBufNumberGet)
{
/*
    ITERATE_DEVICES(DxChx)
    1.1. Call with non-null numOfBuffersPtr.
    Expected: GT_OK.
    1.2. Call with numOfBuffersPtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st           = GT_OK;
    GT_U8       dev;
    GT_U32      numOfBuffers = 0;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enable.
            Expected: GT_OK.
        */
        st = cpssDxChPortGlobalRxBufNumberGet(dev, &numOfBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with numOfBuffersPtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPortGlobalRxBufNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGlobalRxBufNumberGet(dev, &numOfBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGlobalRxBufNumberGet(dev, &numOfBuffers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortGroupRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32  *numOfBuffersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortGroupRxBufNumberGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1. Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E / CPSS_DXCH_PORT_GROUP_HGS_E]
    and non-null numOfBuffersPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values portGroup and other parameters form 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with numOfBuffersPtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS                st           = GT_OK;
    GT_U8                    dev;
    CPSS_DXCH_PORT_GROUP_ENT portGroup    = CPSS_DXCH_PORT_GROUP_GIGA_E;
    GT_U32                   numOfBuffers = 0;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enable.
            Expected: GT_OK.
        */
        /* Call with portGroup [CPSS_DXCH_PORT_GROUP_GIGA_E] */
        portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
        st = cpssDxChPortGroupRxBufNumberGet(dev, portGroup, &numOfBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call with portGroup [CPSS_DXCH_PORT_GROUP_HGS_E] */
        portGroup = CPSS_DXCH_PORT_GROUP_HGS_E;
        st = cpssDxChPortGroupRxBufNumberGet(dev, portGroup, &numOfBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enum values portGroup and other parameters form 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortGroupRxBufNumberGet
                            (dev, portGroup, &numOfBuffers),
                            portGroup);

        /*
            1.3. Call with numOfBuffersPtr [NULL].
            Expected: GT_BAD_PTR
        */
        portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
        st = cpssDxChPortGroupRxBufNumberGet(dev, portGroup, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    portGroup = CPSS_DXCH_PORT_GROUP_GIGA_E;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortGroupRxBufNumberGet(dev, portGroup, &numOfBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortGroupRxBufNumberGet(dev, portGroup, &numOfBuffers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    OUT GT_U32  *numOfBuffersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxBufNumberGet)
{
/*
    ITERATE_DEVICES_RXDMA_PORTS(DxChx)
    1.1.1. Call with non-null numOfBuffersPtr.
    Expected: GT_OK.
    1.1.2. Call with numOfBuffersPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_U8                   dev;
    GT_STATUS               st              = GT_OK;
    GT_PHYSICAL_PORT_NUM    port            = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_U32                  numOfBuffers    = 0;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null numOfBuffersPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortRxBufNumberGet(dev, port, &numOfBuffers);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with numOfBuffersPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortRxBufNumberGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortRxBufNumberGet(dev, port, &numOfBuffers);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortRxBufNumberGet(dev, port, &numOfBuffers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortRxBufNumberGet(dev, port, &numOfBuffers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxBufNumberGet(dev, port, &numOfBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortRxBufNumberGet(dev, port, &numOfBuffers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxNumOfAgedBuffersGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *agedBuffersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxNumOfAgedBuffersGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2)
    1.1.1. Call with non-null agedBuffersPtr.
    Expected: GT_OK.
    1.1.2. Call with agedBuffersPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      agedBuffers = 0;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1. Call with non-null parameters.
           Expected: GT_OK.
        */
        st = cpssDxChPortRxNumOfAgedBuffersGet(dev, &agedBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2. Call with agedBuffers [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortRxNumOfAgedBuffersGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }


    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxNumOfAgedBuffersGet(dev, &agedBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortRxNumOfAgedBuffersGet(dev, &agedBuffers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxMcCntrGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   cntrIdx,
    OUT GT_U32   *mcCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxMcCntrGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS(DxChx)
    1.1.1. Call with valid cntrIdx [0xFFF(Lion) or 0xFF(xCat2)] and non-null mcCntrPtr.
    Expected: GT_OK.
    1.1.2. Call with valid cntrIdx [0x3FFF(Lion) or 0xFFF(xCat2)] and non-null mcCntrPtr.
    Expected: GT_OK.
    1.1.3. Call with valid cntrIdx [0x0] and non-null mcCntrPtr.
    Expected: GT_OK.
    1.1.4. Call with agedBuffersPtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call with out-of-range cntrIdx [0x4000(Lion) or 0x1000(xCat2)] and non-null mcCntrPtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      cntrIdx;
    GT_U32      mcCntr = 0;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
#if 0  /* The function result in critical interrupt -
             PRV_CPSS_LION_B_BMA_MC_CNT_PARITY_ERROR_E
          The reason is that memory of RxMcCntr is not initialized
          (PP did not used buffer memory MC counters yet).
          This critical interrupt result in fatal error handling with
          the reset of a board. */

        /*
            1.1.1. Call with valid cntrIdx [0xFFF(Lion) or 0xFF(xCat2)]
            and non-null mcCntrPtr.
            Expected: GT_OK.
        */
        if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /* lion */
            cntrIdx = 0xFFF;
        }
        else
        {
            /* xCat2 */
            cntrIdx = 0xFF;
        }


        st = cpssDxChPortRxMcCntrGet(dev, cntrIdx, &mcCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with valid cntrIdx [0x3FFF(Lion) or 0xFFF(xCat2)]
            and non-null mcCntrPtr.
            Expected: GT_OK.
        */

        if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E ||
           PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
           PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /* lion */
            cntrIdx = 0x3FFF;
        }
        else
        {
            /* xCat2 */
            cntrIdx = 0xFFF;
        }

        st = cpssDxChPortRxMcCntrGet(dev, cntrIdx, &mcCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.3. Call with valid cntrIdx [0x0] and non-null mcCntrPtr.
            Expected: GT_OK.
        */
        cntrIdx = 0x0;

        st = cpssDxChPortRxMcCntrGet(dev, cntrIdx, &mcCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
#endif
        /*
           1.1.4. Call with agedBuffersPtr [NULL].
           Expected: GT_BAD_PTR.
        */
        cntrIdx = 0;

        st = cpssDxChPortRxMcCntrGet(dev, cntrIdx, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /*
            1.1.5. Call with out-of-range cntrIdx [0x4000(Lion) or 0x1000(xCat2)]
            and non-null mcCntrPtr.
            Expected: GT_BAD_PARAM.
        */

        if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E ||
           PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
           PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /* lion */
            cntrIdx = 0x4000;
        }
        else
        {
            /* xCat2 */
            cntrIdx = 0x1000;
        }
        mcCntr = 0;

        st = cpssDxChPortRxMcCntrGet(dev, cntrIdx,  &mcCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, NULL", dev);
    }


    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    cntrIdx = 0;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxMcCntrGet(dev, cntrIdx,  &mcCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortRxMcCntrGet(dev, cntrIdx,  &mcCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortBufMemFifosThresholdSet)
{
/*
    1.1 Call with correct parameters:
          fifoType[ CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E/
                    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XLG_E/
                    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_CPU_E ],
          descFifoThreshold[0/8/15],
          txFifoThreshold[63/32/0],
          minXFifoReadThreshold[32/0/63].
    Expected: GT_OK.
    1.2 Call get function.
    Expected: GT_OK and same values.
    1.3  Call with out of range fifoType
    Expected: GT_BAD_PARAM.
    1.4  Call with out of range descFifoThreshold lion[16], lion2
    Expected: NOT GT_OK.
    1.5  Call with out of range txFifoThreshold [64]
    Expected: NOT GT_OK.
    1.6  Call with out of range minXFifoReadThreshold [64]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      descFifoThreshold        = 0;
    GT_U32      txFifoThreshold          = 0;
    GT_U32      minXFifoReadThreshold    = 0;
    GT_U32      descFifoThresholdGet     = 0;
    GT_U32      txFifoThresholdGet       = 0;
    GT_U32      minXFifoReadThresholdGet = 0;

    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT   fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E;


    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E;
        descFifoThreshold     = 0;
        txFifoThreshold       = 63;
        minXFifoReadThreshold = 32;

        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                            txFifoThreshold, minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThresholdGet,
                                            &txFifoThresholdGet, &minXFifoReadThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(descFifoThreshold, descFifoThresholdGet,
                              "descFifoThreshold - %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(txFifoThreshold, txFifoThresholdGet,
                              "txFifoThreshold - %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(minXFifoReadThreshold, minXFifoReadThresholdGet,
                              "minXFifoReadThreshold - %d", dev);

        /* 1.1 */
        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XLG_E;
        descFifoThreshold     = 8;
        txFifoThreshold       = 32;
        minXFifoReadThreshold = 0;

        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                            txFifoThreshold, minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThresholdGet,
                                            &txFifoThresholdGet, &minXFifoReadThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(descFifoThreshold, descFifoThresholdGet,
                              "descFifoThreshold - %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(txFifoThreshold, txFifoThresholdGet,
                              "txFifoThreshold - %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(minXFifoReadThreshold, minXFifoReadThresholdGet,
                              "minXFifoReadThreshold - %d", dev);

        /* 1.1 */
        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_CPU_E;
        descFifoThreshold     = 15;
        txFifoThreshold       = 0;
        minXFifoReadThreshold = 63;

        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                            txFifoThreshold, minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThresholdGet,
                                            &txFifoThresholdGet, &minXFifoReadThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(descFifoThreshold, descFifoThresholdGet,
                              "descFifoThreshold - %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(txFifoThreshold, txFifoThresholdGet,
                              "txFifoThreshold - %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(minXFifoReadThreshold, minXFifoReadThresholdGet,
                              "minXFifoReadThreshold - %d", dev);

        /* 1.3 */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                            txFifoThreshold, minXFifoReadThreshold),
                            fifoType);
        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E;

        /* 1.4 */
        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E;
        descFifoThreshold     = 16;
        txFifoThreshold       = 0;
        minXFifoReadThreshold = 0;

        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                            txFifoThreshold, minXFifoReadThreshold);
        if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        descFifoThreshold     = 64;
        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                                    txFifoThreshold, minXFifoReadThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        descFifoThreshold     = 0;

        /* 1.5 */
        txFifoThreshold       = 64;
        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                                    txFifoThreshold, minXFifoReadThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        txFifoThreshold       = 0;

        /* 1.6 */
        minXFifoReadThreshold       = 64;
        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                                    txFifoThreshold, minXFifoReadThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        minXFifoReadThreshold       = 0;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                            txFifoThreshold, minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortBufMemFifosThresholdSet(dev, fifoType, descFifoThreshold,
                                        txFifoThreshold, minXFifoReadThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortBufMemFifosThresholdGet)
{
/*
    1.1 Call with correct parameters:
          fifoType[ CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E/
                    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XLG_E/
                    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_CPU_E ],
          and not NULL descFifoThresholdPtr, txFifoThresholdPtr,
          minXFifoReadThresholdPtr.
    Expected: GT_OK.
    1.2 Call with out of range fifoType.
    Expected: GT_BAD_PARAM.
    1.3 Call with NULL descFifoThresholdPtr.
    Expected: GT_BAD_PTR.
    1.4 Call with NULL txFifoThresholdPtr.
    Expected: GT_BAD_PTR.
    1.5 Call with NULL minXFifoReadThresholdPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      descFifoThreshold        = 0;
    GT_U32      txFifoThreshold          = 0;
    GT_U32      minXFifoReadThreshold    = 0;

    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT   fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E;


    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E;
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThreshold,
                                            &txFifoThreshold, &minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XLG_E;
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThreshold,
                                            &txFifoThreshold, &minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_CPU_E;
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThreshold,
                                            &txFifoThreshold, &minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortBufMemFifosThresholdGet(dev, fifoType,
                          &descFifoThreshold, &txFifoThreshold, &minXFifoReadThreshold),
                            fifoType);
        fifoType = CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E;

        /* 1.3 */
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, NULL,
                                            &txFifoThreshold, &minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* 1.4 */
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThreshold,
                                            NULL, &minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* 1.5 */
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThreshold,
                                            &txFifoThreshold, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThreshold,
                                            &txFifoThreshold, &minXFifoReadThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortBufMemFifosThresholdGet(dev, fifoType, &descFifoThreshold,
                                        &txFifoThreshold, &minXFifoReadThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortTxdmaBurstLimitEnableSet)
{
/*
    ITERATE_DEVICES_TXDMA_PORTS(Lion, Lion2, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortTxdmaBurstLimitEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_U8                   dev;
    GT_STATUS               st              = GT_OK;
    GT_PHYSICAL_PORT_NUM    port            = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable          = GT_FALSE;
    GT_BOOL                 enableGet       = GT_FALSE;

    /* there is no TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            /* 1.1.1 */
            enable = GT_FALSE;

            st = cpssDxChPortTxdmaBurstLimitEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,"get another enable %d, %d", dev, port);

            /* 1.1.1 */
            enable = GT_TRUE;

            st = cpssDxChPortTxdmaBurstLimitEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,"get another enable %d, %d", dev, port);
        }

        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortTxdmaBurstLimitEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortTxdmaBurstLimitEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortTxdmaBurstLimitEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortTxdmaBurstLimitEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortTxdmaBurstLimitEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortTxdmaBurstLimitEnableGet)
{
/*
    ITERATE_DEVICES_TXDMA_PORTS(Lion, Lion2, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_U8                   dev;
    GT_STATUS               st          = GT_OK;
    GT_PHYSICAL_PORT_NUM    port        = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable      = GT_FALSE;

    /* there is no TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            /* 1.1.1 */
            st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortTxdmaBurstLimitEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortTxdmaBurstLimitThresholdsSet)
{
/*
    ITERATE_DEVICES_TXDMA_PORTS(Lion, Lion2, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with valid almostFullThreshold [0/3FFC0/2FFC0],
                           fullThreshold [2FFC0/0/3FFC0].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortTxdmaBurstLimitThresholdsGet.
    Expected: GT_OK and the same value.
    1.1.3. Call with out of range almostFullThreshold[3FFC1](for non E_ARCH)
        and almostFullThreshold[3FFF81] (for E_ARCH devices).
    Expected: not GT_OK.
    1.1.4. Call with out of range fullThreshold[3FFC1](for non E_ARCH)
        and fullThreshold[3FFF81] (for E_ARCH devices).
    Expected: not GT_OK.
*/
    GT_U8                   dev;
    GT_STATUS               st                  = GT_OK;
    GT_PHYSICAL_PORT_NUM    port                = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_U32                  almostFullThreshold          = 0;
    GT_U32                  fullThreshold                = 0;
    GT_U32                  almostFullThresholdGet       = 0;
    GT_U32                  fullThresholdGet             = 0;
    GT_U32                  thresholdResolution          = 0;

    /* there is no TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                            UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Calculate threshold resolution for active device */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            thresholdResolution = 7;
        }
        else
        {
            thresholdResolution = 6;
        }

        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            /* 1.1.1 */
            almostFullThreshold  = 0;
            fullThreshold = 0x2FFC0;
            st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThresholdGet, &fullThresholdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* check results rounded to threshold resolution  */
            UTF_VERIFY_EQUAL2_STRING_MAC((almostFullThreshold>>thresholdResolution),
                                         (almostFullThresholdGet>>thresholdResolution),
                                         "get another almostFullThreshold %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC((fullThreshold>>thresholdResolution),
                                         (fullThresholdGet>>thresholdResolution),
                                         "get another fullThreshold %d, %d", dev, port);

            /* 1.1.1 */
            almostFullThreshold  = 0x3FFC0;
            fullThreshold = 0;
            st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThresholdGet, &fullThresholdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* check results rounded to threshold resolution  */
            UTF_VERIFY_EQUAL2_STRING_MAC((almostFullThreshold>>thresholdResolution),
                                         (almostFullThresholdGet>>thresholdResolution),
                                         "get another almostFullThreshold %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC((fullThreshold>>thresholdResolution),
                                         (fullThresholdGet>>thresholdResolution),
                                         "get another fullThreshold %d, %d", dev, port);

            /* 1.1.1 */
            almostFullThreshold  = 0x2FFC0;
            fullThreshold = 0x3FFC0;
            st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThresholdGet, &fullThresholdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* check results rounded to threshold resolution  */
            UTF_VERIFY_EQUAL2_STRING_MAC((almostFullThreshold>>thresholdResolution),
                                         (almostFullThresholdGet>>thresholdResolution),
                                         "get another almostFullThreshold %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC((fullThreshold>>thresholdResolution),
                                         (fullThresholdGet>>thresholdResolution),
                                         "get another fullThreshold %d, %d", dev, port);

            /* 1.1.3 */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                almostFullThreshold  = 0x3FFF81;
            }
            else
            {
                almostFullThreshold  = 0x3FFC1;
            }
            st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            almostFullThreshold  = 0;

            /* 1.1.4 */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                fullThreshold  = 0x3FFF81;
            }
            else
            {
                fullThreshold = 0x3FFC1;
            }
            st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            fullThreshold  = 0;
        }

        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortTxdmaBurstLimitThresholdsSet(dev, port, almostFullThreshold, fullThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortTxdmaBurstLimitThresholdsGet)
{
/*
    ITERATE_DEVICES_TXDMA_PORTS(Lion, Lion2, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL almostFullThresholdPtr, fullThresholdPtr.
    Expected: GT_OK.
    1.1.2. Call with NULL almostFullThresholdPtr.
    Expected: GT_BAD_PTR.
    1.1.3. Call with NULL fullThresholdPtr.
    Expected: GT_BAD_PTR.
*/
    GT_U8                   dev;
    GT_STATUS               st                  = GT_OK;
    GT_PHYSICAL_PORT_NUM    port                = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_U32                  almostFullThreshold = 0;
    GT_U32                  fullThreshold       = 0;

    /* there is no TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            /* 1.1.1 */
            st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThreshold, &fullThreshold);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, NULL, &fullThreshold);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /* 1.1.3 */
            st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThreshold, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThreshold, &fullThreshold);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThreshold, &fullThreshold);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThreshold, &fullThreshold);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThreshold, &fullThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortTxdmaBurstLimitThresholdsGet(dev, port, &almostFullThreshold, &fullThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChPortBufMg suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortBufMg)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGlobalXonLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGlobalXonLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGlobalXoffLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGlobalXoffLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxFcProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxFcProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXonLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXonLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXoffLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortXoffLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxBufLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxBufLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCpuRxBufCountGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFcHolSysModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortBuffersModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortBuffersModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupXonLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupXonLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupXoffLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupXoffLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupRxBufLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupRxBufLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCrossChipFcPacketRelayEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCrossChipFcPacketRelayEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortClearByBufferEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortClearByBufferEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGlobalPacketNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGlobalRxBufNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortGroupRxBufNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxBufNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxNumOfAgedBuffersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxMcCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortBufMemFifosThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortBufMemFifosThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortTxdmaBurstLimitEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortTxdmaBurstLimitEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortTxdmaBurstLimitThresholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortTxdmaBurstLimitThresholdsGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortBufMg)


