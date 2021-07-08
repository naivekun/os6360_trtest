/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChCutThroughUT.c
*
* DESCRIPTION:
*       Unit tests for cpssDxChCutThrough, that provides
*       CPSS DXCH Cut Through facility implementation.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 18 $
*******************************************************************************/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* defines */

/* Default valid value for port id */
#define CUT_THROUGH_VALID_PHY_PORT_CNS  0

/* Invalid profileIndex */
#define CUT_THROUGH_INVALID_PROFILEINDEX_CNS   8

/* Invalid tcQueue */
#define CUT_THROUGH_INVALID_TCQUEUE_CNS        8

/* Invalid xoffThreshold */
#define CUT_THROUGH_INVALID_XOFFTHRESHOLD_CNS  0x7FF+1

/* Invalid dropThreshold */
#define CUT_THROUGH_INVALID_DROPTHRESHOLD_CNS  0x7FF+1

/* Invalid packetSize */
#define CUT_THROUGH_INVALID_PACKET_SIZE_CNS      16377

/* Invalid etherType */
#define CUT_THROUGH_INVALID_ETHERTYPE_CNS      0xFFFF+1

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughMemoryRateLimitSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable,
    IN CPSS_PORT_SPEED_ENT  portSpeed
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughMemoryRateLimitSet)
{
/*
    ITERATE_DEVICES_PHY_AND_CPU_PORTS (Lion and above)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE] and all portSpeed values
                                    [CPSS_PORT_SPEED_10_E   /
                                     CPSS_PORT_SPEED_100_E  /
                                     CPSS_PORT_SPEED_1000_E /
                                     CPSS_PORT_SPEED_10000_E/
                                     CPSS_PORT_SPEED_12000_E/
                                     CPSS_PORT_SPEED_2500_E /
                                     CPSS_PORT_SPEED_5000_E /
                                     CPSS_PORT_SPEED_13600_E/
                                     CPSS_PORT_SPEED_20000_E/
                                     CPSS_PORT_SPEED_40000_E]
    Expected: GT_OK.
    1.1.2. Call cpssDxChCutThroughMemoryRateLimitGet with the same params.
    Expected: GT_OK and the same values.
    1.1.3. Call with wrong enum values portSpeed.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet = GT_FALSE;
    CPSS_PORT_SPEED_ENT  portSpeed = 0;
    CPSS_PORT_SPEED_ENT  portSpeedGet = 1;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                           UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
            {
            /*
                1.1.1. Call with enable [GT_TRUE / GT_FALSE] and portSpeed
                                                [CPSS_PORT_SPEED_10_E   /
                                                 CPSS_PORT_SPEED_5000_E /
                                                 CPSS_PORT_SPEED_40000_E]
                Expected: GT_OK/GT_BAD_PARAM.
            */

                portSpeed = CPSS_PORT_SPEED_10_E;

                st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
                if(enable == GT_TRUE)
                {
                    UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, enable, portSpeed);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, enable, portSpeed);
                }

                if(GT_OK == st)
                {
                    /*
                       1.1.2. Call cpssDxChCutThroughMemoryRateLimitGet.
                       Expected: GT_OK and the same values.
                    */
                    st = cpssDxChCutThroughMemoryRateLimitGet(dev, port,
                                                              &enableGet, &portSpeedGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "[cpssDxChCutThroughMemoryRateLimitGet]: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable value than was set: %d, %d", dev, port);
                    if(enable == GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(portSpeed, portSpeedGet,
                               "get another portSpeed value than was set: %d, %d", dev, port);
                    }
                }

                portSpeed = CPSS_PORT_SPEED_5000_E;

                st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, enable, portSpeed);

                if(GT_OK == st)
                {
                    /*
                       1.1.2. Call cpssDxChCutThroughMemoryRateLimitGet.
                       Expected: GT_OK and the same values.
                    */
                    st = cpssDxChCutThroughMemoryRateLimitGet(dev, port,
                                                              &enableGet, &portSpeedGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "[cpssDxChCutThroughMemoryRateLimitGet]: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable value than was set: %d, %d", dev, port);

                    if(enable == GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(portSpeed, portSpeedGet,
                               "get another portSpeed value than was set: %d, %d", dev, port);
                    }
                }

                portSpeed = CPSS_PORT_SPEED_40000_E;

                st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
                if(enable == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, enable, portSpeed);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }

                if(GT_OK == st)
                {
                    /*
                       1.1.2. Call cpssDxChCutThroughMemoryRateLimitGet.
                       Expected: GT_OK and the same values.
                    */
                    st = cpssDxChCutThroughMemoryRateLimitGet(dev, port,
                                                              &enableGet, &portSpeedGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "[cpssDxChCutThroughMemoryRateLimitGet]: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable value than was set: %d, %d", dev, port);
                    if(enable == GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(portSpeed, portSpeedGet,
                               "get another portSpeed value than was set: %d, %d", dev, port);
                    }
                }

                if(enable == GT_TRUE) /* when enable == GT_FALSE the portSpeed is not checked/used */
                {
                    /*
                        1.1.3. Call with enable [GT_TRUE / GT_FALSE]
                               and wrong enum values portSpeed.
                        Expected: NOT GT_OK.
                    */
                    UTF_ENUMS_CHECK_MAC(cpssDxChCutThroughMemoryRateLimitSet
                                        (dev, port, enable, portSpeed),
                                        portSpeed);
                }
            }
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;
        portSpeed = CPSS_PORT_SPEED_1000_E;

        st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    portSpeed = 0;
    port = CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughMemoryRateLimitSet(dev, port, enable, portSpeed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughMemoryRateLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr,
    OUT CPSS_PORT_SPEED_ENT  *portSpeedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughMemoryRateLimitGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1.1. Call with non-null pointers.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR
    1.1.3. Call with portSpeedPtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL     enable;
    CPSS_PORT_SPEED_ENT  portSpeed;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                           UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null profileIndexGetPtr.
               Expected: GT_OK.
            */
            st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, &enable, &portSpeed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, NULL, &portSpeed);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);

            /*
               1.1.3. Call with portSpeedPtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, &enable, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, &enable, &portSpeed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, &enable, &portSpeed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, &enable, &portSpeed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, &enable, &portSpeed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughMemoryRateLimitGet(dev, port, &enable, &portSpeed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughMinimalPacketSizeSet
(
    IN GT_U8    devNum,
    IN GT_U32   size
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughMinimalPacketSizeSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with correct size.
    Expected: GT_OK.
    1.2. Call cpssDxChCutThroughMinimalPacketSizeGet with not-NULL sizePtr.
    Expected: GT_OK and the same size as was set.
    1.3. Call with wrong size [CUT_THROUGH_INVALID_PACKET_SIZE_CNS].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32  size = 0;
    GT_U32  sizeGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct size
            Expected: GT_OK.
        */
        for (size = 136; size < CUT_THROUGH_INVALID_PACKET_SIZE_CNS; size += 8)
        {
            st = cpssDxChCutThroughMinimalPacketSizeSet(dev, size);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(GT_OK == st)
            {
                /*
                    1.2. Call cpssDxChCutThroughMinimalPacketSizeGet with not-NULL sizePtr.
                    Expected: GT_OK and the same size as was set.
                */
                st = cpssDxChCutThroughMinimalPacketSizeGet(dev, &sizeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChCutThroughMinimalPacketSizeGet: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(size, sizeGet,
                                 "got another size then was set: %d", dev);
            }
        }

        /*
            1.3. Call with wrong size [CUT_THROUGH_INVALID_PACKET_SIZE_CNS].
            Expected: NOT GT_OK.
        */
        size = CUT_THROUGH_INVALID_PACKET_SIZE_CNS;

        st = cpssDxChCutThroughMinimalPacketSizeSet(dev, size);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        size = 512;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughMinimalPacketSizeSet(dev, size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughMinimalPacketSizeSet(dev, size);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughMinimalPacketSizeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *sizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughMinimalPacketSizeGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null sizePtr.
    Expected: GT_OK.
    1.2. Call with wrong sizePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      size;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null sizePtr.
            Expected: GT_OK.
        */
        st = cpssDxChCutThroughMinimalPacketSizeGet(dev, &size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong sizePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCutThroughMinimalPacketSizeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughMinimalPacketSizeGet(dev, &size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughMinimalPacketSizeGet(dev, &size);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughPortEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable,
    IN GT_BOOL  untaggedEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughPortEnableSet)
{
/*
    ITERATE_DEVICES_PHY_AND_CPU_PORTS (Lion and above)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE]
           and untaggedEnable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCutThroughPortEnableGet with the same params.
    Expected: GT_OK and the same values.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL     untaggedEnable = GT_TRUE;
    GT_BOOL     untaggedEnableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);



    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_DXCH_PP_MAC(dev)->cutThroughEnable == GT_FALSE)
        {
            continue;
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE / GT_FALSE]
                       and untaggedEnable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */

            for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
                for(untaggedEnable = GT_FALSE; untaggedEnable <= GT_TRUE; untaggedEnable++)
                {
                    st = cpssDxChCutThroughPortEnableSet(dev, port, enable, untaggedEnable);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, enable, untaggedEnable);

                    if(GT_OK == st)
                    {
                        /*
                           1.1.2. Call cpssDxChCutThroughPortEnableGet.
                           Expected: GT_OK and the same values.
                        */
                        st = cpssDxChCutThroughPortEnableGet(dev, port,
                                                                  &enableGet, &untaggedEnableGet);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                               "[cpssDxChCutThroughPortEnableGet]: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                               "get another enable value than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(untaggedEnable, untaggedEnableGet,
                               "get another untaggedEnable value than was set: %d, %d", dev, port);
                    }
                }
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;
        untaggedEnable = GT_TRUE;

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCutThroughPortEnableSet(dev, port, enable, untaggedEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCutThroughPortEnableSet(dev, port, enable, untaggedEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCutThroughPortEnableSet(dev, port, enable, untaggedEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPortEnableSet(dev, port, enable, untaggedEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughPortEnableSet(dev, port, enable, untaggedEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughPortEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr,
    OUT GT_BOOL *untaggedEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughPortEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1.1. Call with non-null pointers.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR
    1.1.3. Call with untaggedEnablePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL     enable;
    GT_BOOL     untaggedEnable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null profileIndexGetPtr.
               Expected: GT_OK.
            */
            st = cpssDxChCutThroughPortEnableGet(dev, port, &enable, &untaggedEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChCutThroughPortEnableGet(dev, port, NULL, &untaggedEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);

            /*
               1.1.3. Call with untaggedEnablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChCutThroughPortEnableGet(dev, port, &enable, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCutThroughPortEnableGet(dev, port, &enable, &untaggedEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCutThroughPortEnableGet(dev, port, &enable, &untaggedEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCutThroughPortEnableGet(dev, port, &enable, &untaggedEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPortEnableGet(dev, port, &enable, &untaggedEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughPortEnableGet(dev, port, &enable, &untaggedEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughUpEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     up,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughUpEnableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with up [0 - 7] and enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChCutThroughUpEnableGet.
    Expected: GT_OK and the same enable.
    1.3. Call with wrong up [8].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U8       up = 0;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with up [0 - 7] and enable [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
        */
        for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
            for(up = 0; up <= 7; up++)
            {
                st = cpssDxChCutThroughUpEnableSet(dev, up, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, up, enable);

                if(GT_OK == st)
                {
                    /*
                       1.2. Call cpssDxChCutThroughUpEnableGet.
                       Expected: GT_OK and the same enable.
                    */
                    st = cpssDxChCutThroughUpEnableGet(dev, up, &state);

                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "[cpssDxChCutThroughUpEnableGet]: %d, %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                                    "get another enable value than was set: %d, %d", dev);
                }
            }

        /*
            1.3. Call with wrong up [8].
            Expected: NOT GT_OK.
        */
        st = cpssDxChCutThroughUpEnableSet(dev, 8, enable);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, up, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    up = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughUpEnableSet(dev, up, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughUpEnableSet(dev, up, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughUpEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     up,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughUpEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.2. Call with wrong up [8].
    Expected: NOT GT_OK.
    1.3. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U8       up = 0;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null statePtr.
            Expected: GT_OK.
        */
        st = cpssDxChCutThroughUpEnableGet(dev, up, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong up [8].
            Expected: NOT GT_OK.
        */
        st = cpssDxChCutThroughUpEnableGet(dev, 8, &state);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, up, state);

        /*
            1.3. Call with statePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChCutThroughUpEnableGet(dev, up, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughUpEnableGet(dev, up, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughUpEnableGet(dev, up, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughVlanEthertypeSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   etherType0,
    IN  GT_U32   etherType1
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughVlanEthertypeSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with etherType0 [0 - 0x0FFFF] and etherType1 [0 - 0x0FFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChCutThroughVlanEthertypeGet.
    Expected: GT_OK and the same values.
    1.3. Call with wrong etherType0 [CUT_THROUGH_INVALID_ETHERTYPE_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong etherType1 [CUT_THROUGH_INVALID_ETHERTYPE_CNS].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      etherType0 = 0;
    GT_U32      etherType0Get = 0;
    GT_U32      etherType1 = 0;
    GT_U32      etherType1Get = 0;
    GT_U32      counter = 0;/* loop counter */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with etherType0 [0 - 0x0FFFF] and etherType1 [0 - 0x0FFFF].
           Expected: GT_OK.
        */
        for(etherType0 = 0; etherType0 < CUT_THROUGH_INVALID_ETHERTYPE_CNS; etherType0+=128)
        {
            for(etherType1 = 0; etherType1 < CUT_THROUGH_INVALID_ETHERTYPE_CNS; etherType1+=256)
            {
                counter++;
                st = cpssDxChCutThroughVlanEthertypeSet(dev, etherType0, etherType1);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, etherType0, etherType1);

                if(GT_OK == st)
                {
                    /*
                       1.2. Call cpssDxChCutThroughVlanEthertypeGet.
                       Expected: GT_OK and the same etherType1.
                    */
                    st = cpssDxChCutThroughVlanEthertypeGet(dev, &etherType0Get, &etherType1Get);

                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "[cpssDxChCutThroughVlanEthertypeGet]: %d, %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(etherType0, etherType0Get,
                                    "get another etherType0 value than was set: %d, %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(etherType1, etherType1Get,
                                    "get another etherType1 value than was set: %d, %d", dev);
                }

                if((counter % 100000) == 0)
                {
                    utfPrintKeepAlive();
                }
            }

            if((etherType0 == 0) && (GT_TRUE == prvUtfIsGmCompilation() || GT_TRUE == prvUtfReduceLogSizeFlagGet()))
            {
                /* reducing outer iterations to 2 (from 512)*/
                etherType0 = (CUT_THROUGH_INVALID_ETHERTYPE_CNS - 1)  - 128;
            }
        }

        /*
            1.3. Call with wrong etherType0 [CUT_THROUGH_INVALID_ETHERTYPE_CNS].
            Expected: NOT GT_OK.
        */
        etherType0 = CUT_THROUGH_INVALID_ETHERTYPE_CNS;

        st = cpssDxChCutThroughVlanEthertypeSet(dev, etherType0, etherType1);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, etherType0, etherType1);

        etherType0 = 0;

        /*
            1.4. Call with wrong etherType1 [CUT_THROUGH_INVALID_ETHERTYPE_CNS].
            Expected: NOT GT_OK.
        */
        etherType1 = CUT_THROUGH_INVALID_ETHERTYPE_CNS;

        st = cpssDxChCutThroughVlanEthertypeSet(dev, etherType0, etherType1);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, etherType0, etherType1);

        etherType1 = 0;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    etherType0 = 0;
    etherType1 = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughVlanEthertypeSet(dev, etherType0, etherType1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughVlanEthertypeSet(dev, etherType0, etherType1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughVlanEthertypeGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32   *etherType0Ptr,
    OUT  GT_U32   *etherType1Ptr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughVlanEthertypeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.2. Call with etherType0Ptr [NULL].
    Expected: GT_BAD_PTR
    1.3. Call with etherType1Ptr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U32       etherType0Get = 0;
    GT_U32       etherType1Get = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null statePtr.
            Expected: GT_OK.
        */
        st = cpssDxChCutThroughVlanEthertypeGet(dev, &etherType0Get, &etherType1Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with etherType0Ptr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChCutThroughVlanEthertypeGet(dev, NULL, &etherType1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);

        /*
            1.3. Call with statePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChCutThroughVlanEthertypeGet(dev, &etherType0Get, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughVlanEthertypeGet(dev, &etherType0Get, &etherType1Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughVlanEthertypeGet(dev, &etherType0Get, &etherType1Get);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughBypassRouterAndPolicerEnableSet
(
    IN  GT_U8   dev,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughBypassRouterAndPolicerEnableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChCutThroughBypassRouterAndPolicerEnableGet.
    Expected: GT_OK and the same enable than was set.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;
    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        /* call with state  = GT_TRUE; */
        state  = GT_TRUE;

        st = cpssDxChCutThroughBypassRouterAndPolicerEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChCutThroughBypassRouterAndPolicerEnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChCutThroughBypassRouterAndPolicerEnableGet(dev, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
             "cpssDxChCutThroughBypassRouterAndPolicerEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);

        /* call with state  = GT_FALSE; */
        state  = GT_FALSE;

        st = cpssDxChCutThroughBypassRouterAndPolicerEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChCutThroughBypassRouterAndPolicerEnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChCutThroughBypassRouterAndPolicerEnableGet(dev, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChCutThroughBypassRouterAndPolicerEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughBypassRouterAndPolicerEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughBypassRouterAndPolicerEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCutThroughBypassRouterAndPolicerEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughBypassRouterAndPolicerEnableGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null enablePtr.
    Expected: GT_OK.
    1.2. Call with incorrect enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     state;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChCutThroughBypassRouterAndPolicerEnableGet(dev, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call with incorrect enablePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCutThroughBypassRouterAndPolicerEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, state);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughBypassRouterAndPolicerEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughBypassRouterAndPolicerEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughBypassModeSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with bypassMode{bypassRouter[GT_FALSE \ GT_TRUE \
                                           GT_FALSE \ GT_TRUE],
                              bypassIngressPolicerStage0[GT_FALSE \ GT_FALSE\
                                                         GT_TRUE \ GT_TRUE],
                              bypassIngressPolicerStage1[GT_FALSE \ GT_TRUE \
                                                         GT_FALSE \ GT_TRUE],
                              bypassEgressPolicer[GT_FALSE \ GT_FALSE \
                                                  GT_TRUE \ GT_TRUE],
                              bypassEgressPcl[GT_FALSE \ GT_TRUE \
                                              GT_FALSE \ GT_TRUE],
                              bypassIngressPcl[CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E \
                                               CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E \
                                               CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E \
                                               CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E]},
                              bypassIngressOam[GT_FALSE \ GT_FALSE \
                                               GT_TRUE \ GT_TRUE],
                              bypassEgressOam[GT_FALSE \ GT_TRUE \
                                              GT_FALSE \ GT_TRUE],
                              bypassMll[GT_FALSE \ GT_FALSE \
                                        GT_TRUE \ GT_TRUE].

    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range bypassMode->bypassIngressPcl.
    Expected: GT_BAD_PARAM.
    1.4. Call with bypassModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_BOOL                          isEqual       = GT_FALSE;
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC bypassMode    = {0};
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC bypassModeGet = {0};
    GT_BOOL                          iPcl0Bypass;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
                                           UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E |
                                           UTF_AC3X_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        iPcl0Bypass = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.pcl.iPcl0Bypass;

            /* 1.1. */
            bypassMode.bypassRouter               = GT_FALSE;
            bypassMode.bypassIngressPolicerStage0 = GT_FALSE;
            bypassMode.bypassIngressPolicerStage1 = GT_FALSE;
            bypassMode.bypassEgressPolicer        = GT_FALSE;
            bypassMode.bypassEgressPcl            = GT_FALSE;
            bypassMode.bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E;
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                bypassMode.bypassIngressOam       = GT_FALSE;
                bypassMode.bypassEgressOam        = GT_FALSE;
                bypassMode.bypassMll              = GT_FALSE;
            }

        st = cpssDxChCutThroughBypassModeSet(dev, &bypassMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. */
                st = cpssDxChCutThroughBypassModeGet(dev, &bypassModeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&bypassMode, (GT_VOID*)&bypassModeGet, sizeof(bypassMode))) ?
                                   GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                "get another bypassMode than was set: %d", dev);

                /* 1.1. */
                bypassMode.bypassRouter               = GT_TRUE;
                bypassMode.bypassIngressPolicerStage0 = GT_FALSE;
                bypassMode.bypassIngressPolicerStage1 = GT_TRUE;
                bypassMode.bypassEgressPolicer        = GT_FALSE;
                bypassMode.bypassEgressPcl            = GT_TRUE;
                bypassMode.bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E;
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    bypassMode.bypassIngressOam       = GT_FALSE;
                    bypassMode.bypassEgressOam        = GT_TRUE;
                    bypassMode.bypassMll              = GT_FALSE;
                }

                st = cpssDxChCutThroughBypassModeSet(dev, &bypassMode);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChCutThroughBypassModeGet(dev, &bypassModeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.pcl.iPcl0Bypass &&
                    (bypassModeGet.bypassIngressPcl == CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E))
                {
                    bypassModeGet.bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E;
                }
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&bypassMode, (GT_VOID*)&bypassModeGet, sizeof(bypassMode))) ?
                                   GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                "get another bypassMode than was set: %d", dev);

                /* 1.1. */
                bypassMode.bypassRouter               = GT_FALSE;
                bypassMode.bypassIngressPolicerStage0 = GT_TRUE;
                bypassMode.bypassIngressPolicerStage1 = GT_FALSE;
                bypassMode.bypassEgressPolicer        = GT_TRUE;
                bypassMode.bypassEgressPcl            = GT_FALSE;
                bypassMode.bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E;
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    bypassMode.bypassIngressOam       = GT_TRUE;
                    bypassMode.bypassEgressOam        = GT_FALSE;
                    bypassMode.bypassMll              = GT_TRUE;
                }

                st = cpssDxChCutThroughBypassModeSet(dev, &bypassMode);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChCutThroughBypassModeGet(dev, &bypassModeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                if (iPcl0Bypass &&
                    (bypassModeGet.bypassIngressPcl == CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E))
                {
                    bypassModeGet.bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E;
                }
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&bypassMode, (GT_VOID*)&bypassModeGet, sizeof(bypassMode))) ?
                                   GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                "get another bypassMode than was set: %d", dev);

                /* 1.1. */
                bypassMode.bypassRouter               = GT_TRUE;
                bypassMode.bypassIngressPolicerStage0 = GT_TRUE;
                bypassMode.bypassIngressPolicerStage1 = GT_TRUE;
                bypassMode.bypassEgressPolicer        = GT_TRUE;
                bypassMode.bypassEgressPcl            = GT_TRUE;
                bypassMode.bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E;
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    bypassMode.bypassIngressOam       = GT_TRUE;
                    bypassMode.bypassEgressOam        = GT_TRUE;
                    bypassMode.bypassMll              = GT_TRUE;
                }

                st = cpssDxChCutThroughBypassModeSet(dev, &bypassMode);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChCutThroughBypassModeGet(dev, &bypassModeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&bypassMode, (GT_VOID*)&bypassModeGet, sizeof(bypassMode))) ?
                                   GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                "get another bypassMode than was set: %d", dev);

                /* 1.3. */
                UTF_ENUMS_CHECK_MAC(cpssDxChCutThroughBypassModeSet
                                            (dev, &bypassMode),
                                            bypassMode.bypassIngressPcl);

                /* 1.4. */
                st = cpssDxChCutThroughBypassModeSet(dev, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bypassModePtr = NULL", dev);
    }

    bypassMode.bypassRouter               = GT_FALSE;
    bypassMode.bypassIngressPolicerStage0 = GT_FALSE;
    bypassMode.bypassIngressPolicerStage1 = GT_FALSE;
    bypassMode.bypassEgressPolicer        = GT_FALSE;
    bypassMode.bypassEgressPcl            = GT_FALSE;
    bypassMode.bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E;
    bypassMode.bypassIngressOam       = GT_FALSE;
    bypassMode.bypassEgressOam        = GT_FALSE;
    bypassMode.bypassMll              = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
                                           UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChCutThroughBypassModeSet(dev, &bypassMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughBypassModeSet(dev, &bypassMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCutThroughBypassModeGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL bypassModePtr.
    Expected: GT_OK.
    1.2. Call with bypassModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_CUT_THROUGH_BYPASS_STC bypassMode = {0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
                                           UTF_ALDRIN_E | UTF_AC3X_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. */
        st = cpssDxChCutThroughBypassModeGet(dev, &bypassMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChCutThroughBypassModeGet(dev, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bypassModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
                                           UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChCutThroughBypassModeGet(dev, &bypassMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughBypassModeGet(dev, &bypassMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPortGroupMaxBuffersLimitSet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp;
    GT_U32             buffersLimit;
    GT_U32             buffersLimitGet;
    GT_BOOL            enable;
    GT_BOOL            enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | \
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | \
        UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
         /* Filter out all lions except LION2_B1 and above*/
        if(!PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
            (PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(dev) == 0))
        {
            continue;
        }

        for(portGroupsBmp = 1; portGroupsBmp != 0x80000000; portGroupsBmp <<= 1)
        {
            for(buffersLimit = 0; buffersLimit < 0x16001; buffersLimit +=0x6000)
            {
                for(enable = GT_FALSE; enable < GT_TRUE; enable = GT_TRUE)
                {
                    st = cpssDxChCutThroughPortGroupMaxBuffersLimitSet(dev,
                        portGroupsBmp, enable, buffersLimit);
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                    {
                        if(buffersLimit > 0xFFFF)
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st,
                                dev, buffersLimit, enable);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st,
                                dev, buffersLimit, enable);
                            st = cpssDxChCutThroughPortGroupMaxBuffersLimitGet(
                                dev, portGroupsBmp, &enableGet,
                                &buffersLimitGet);
                            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
                            if(GT_TRUE == enable)
                            {
                                UTF_VERIFY_EQUAL0_PARAM_MAC(buffersLimit,
                                    buffersLimitGet);
                            }
                        }
                    }
                    else
                    {
                        if((PRV_CPSS_PP_MAC(dev)->
                            portGroupsInfo.activePortGroupsBmp & portGroupsBmp)
                            == portGroupsBmp)
                        {
                            if(buffersLimit <= 0x3FFF)
                            {
                                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                                    portGroupsBmp, buffersLimit, enable);
                                st = cpssDxChCutThroughPortGroupMaxBuffersLimitGet(
                                    dev, portGroupsBmp, &enableGet,
                                    &buffersLimitGet);
                                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                                UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
                                UTF_VERIFY_EQUAL0_PARAM_MAC(buffersLimit,
                                    buffersLimitGet);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st,
                                    dev, portGroupsBmp, buffersLimit, enable);
                            }
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
                                portGroupsBmp, buffersLimit, enable);
                        }
                    }
                }
            }
        }
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | \
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | \
        UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_FALCON_E);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPortGroupMaxBuffersLimitSet(dev,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS, GT_TRUE, 0x1000);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughPortGroupMaxBuffersLimitSet(dev,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, GT_TRUE, 0x1000);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPortGroupMaxBuffersLimitGet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp;
    GT_U32             buffersLimitGet;
    GT_BOOL            enableGet;
    GT_BOOL           *enableVector[4];
    GT_U32            *limitsVector[4];
    GT_U32             ii;

    /* Fill the testing vector */
    enableVector[0] = NULL;
    limitsVector[0] = NULL;
    enableVector[1] = NULL;
    limitsVector[1] = &buffersLimitGet;
    enableVector[2] = &enableGet;
    limitsVector[2] = NULL;
    enableVector[3] = &enableGet;
    limitsVector[3] = &buffersLimitGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | \
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | \
        UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
         /* Filter out all lions except LION2_B1 and above*/
        if(!PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
            (PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(dev) == 0))
        {
            continue;
        }

        for(portGroupsBmp = 1; portGroupsBmp != 0x80000000; portGroupsBmp <<= 1)
        {
            for(ii = 0; ii < 4; ii++)
            {
                st = cpssDxChCutThroughPortGroupMaxBuffersLimitGet(dev,
                        portGroupsBmp, enableVector[ii], limitsVector[ii]);
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(enableVector[ii] && limitsVector[ii])
                    {
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
                    }
                }
                else
                {
                    if((PRV_CPSS_PP_MAC(dev)->
                        portGroupsInfo.activePortGroupsBmp & portGroupsBmp)
                        == portGroupsBmp)
                    {
                        if(enableVector[ii] && limitsVector[ii])
                        {
                            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
                        }
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                            portGroupsBmp);
                    }
                }
            }
        }
    }
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | \
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | \
        UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_FALCON_E);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPortGroupMaxBuffersLimitGet(dev,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &enableGet, &buffersLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughPortGroupMaxBuffersLimitGet(dev,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &enableGet, &buffersLimitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

UTF_TEST_CASE_MAC(cpssDxChCutThroughUdeCfgSet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    GT_U32             udeIndex;
    GT_BOOL            udeCutThroughEnable;
    GT_U32             udeByteCount;
    GT_BOOL            udeCutThroughEnableGet;
    GT_U32             udeByteCountGet;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* correct parameters */
        udeIndex            = 3;
        udeCutThroughEnable = GT_TRUE;
        udeByteCount        = 0x3FFF;
        st = cpssDxChCutThroughUdeCfgSet(
            dev, udeIndex, udeCutThroughEnable, udeByteCount);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st,
            dev, udeIndex, udeCutThroughEnable, udeByteCount);

        /* get and compare set-values and get-values */
        st = cpssDxChCutThroughUdeCfgGet(
            dev, udeIndex, &udeCutThroughEnableGet, &udeByteCountGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, udeIndex);
        UTF_VERIFY_EQUAL0_PARAM_MAC(udeCutThroughEnable, udeCutThroughEnableGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(udeByteCount, udeByteCountGet);

        /* bad parameters - udeIndex */
        udeIndex            = 4;
        udeCutThroughEnable = GT_TRUE;
        udeByteCount        = 0x200;
        st = cpssDxChCutThroughUdeCfgSet(
            dev, udeIndex, udeCutThroughEnable, udeByteCount);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st,
            dev, udeIndex, udeCutThroughEnable, udeByteCount);

        /* bad parameters -  udeByteCount*/
        udeIndex            = 0;
        udeCutThroughEnable = GT_TRUE;
        udeByteCount        = 0x4000;
        st = cpssDxChCutThroughUdeCfgSet(
            dev, udeIndex, udeCutThroughEnable, udeByteCount);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st,
            dev, udeIndex, udeCutThroughEnable, udeByteCount);

    }

    /* correct parameters */
    udeIndex            = 0;
    udeCutThroughEnable = GT_FALSE;
    udeByteCount        = 0;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughUdeCfgSet(
            dev, udeIndex, udeCutThroughEnable, udeByteCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughUdeCfgSet(
        dev, udeIndex, udeCutThroughEnable, udeByteCount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughUdeCfgGet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    GT_U32             udeIndex;
    GT_BOOL            udeCutThroughEnable;
    GT_U32             udeByteCount;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* correct parameters */
        udeIndex            = 3;
        st = cpssDxChCutThroughUdeCfgGet(
            dev, udeIndex, &udeCutThroughEnable, &udeByteCount);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, udeIndex);

        /* bad parameters - udeIndex */
        udeIndex            = 4;
        st = cpssDxChCutThroughUdeCfgGet(
            dev, udeIndex, &udeCutThroughEnable, &udeByteCount);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, udeIndex);

        /* bad parameters -  udeCutThroughEnablePtr*/
        udeIndex            = 0;
        st = cpssDxChCutThroughUdeCfgGet(
            dev, udeIndex, NULL/*udeCutThroughEnable*/, &udeByteCount);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, udeIndex);

        /* bad parameters -  udeByteCountPtr*/
        udeIndex            = 0;
        st = cpssDxChCutThroughUdeCfgGet(
            dev, udeIndex, &udeCutThroughEnable, NULL/*udeByteCount*/);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, udeIndex);
    }

    /* correct parameters */
    udeIndex            = 0;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughUdeCfgGet(
            dev, udeIndex, &udeCutThroughEnable, &udeByteCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughUdeCfgGet(
        dev, udeIndex, &udeCutThroughEnable, &udeByteCount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPassengerParsingModeSet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingMode;
    CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingModeGet;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* correct parameters */
        parsingMode  = CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_BYTE_COUNT_ONLY_E;
        st = cpssDxChCutThroughPassengerParsingModeSet(
            dev, parsingMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, parsingMode);

        /* get and compare set-values and get-values */
        st = cpssDxChCutThroughPassengerParsingModeGet(
            dev, &parsingModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(parsingMode, parsingModeGet);

        /* bad parameters - parsingMode */
        parsingMode  = 100;
        st = cpssDxChCutThroughPassengerParsingModeSet(
            dev, parsingMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, parsingMode);
    }

    /* correct parameters */
    parsingMode  = CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_BYTE_COUNT_ONLY_E;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPassengerParsingModeSet(
            dev, parsingMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughPassengerParsingModeSet(
        dev, parsingMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPassengerParsingModeGet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingMode;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* correct parameters */
        st = cpssDxChCutThroughPassengerParsingModeGet(
            dev, &parsingMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* bad parameters - parsingModePtr */
        st = cpssDxChCutThroughPassengerParsingModeGet(
            dev, NULL /*parsingMode*/);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPassengerParsingModeGet(
            dev, &parsingMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughPassengerParsingModeGet(
        dev, &parsingMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughByteCountExtractFailsCounterGet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    GT_U32             count;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* correct parameters */
        st = cpssDxChCutThroughByteCountExtractFailsCounterGet(
            dev, &count);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* bad parameters - parsingModePtr */
        st = cpssDxChCutThroughByteCountExtractFailsCounterGet(
            dev, NULL /*count*/);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughByteCountExtractFailsCounterGet(
            dev, &count);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughByteCountExtractFailsCounterGet(
        dev, &count);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  cfg;
    CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  cfgGet;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* correct parameters */
        cfg.enableByteCountBoundariesCheck = GT_TRUE;
        cfg.minByteCountBoundary           = 0;
        cfg.maxByteCountBoundary           = 0x3FFF;
        cfg.enableIpv4HdrCheckByChecksum   = GT_TRUE;
        cfg.hdrIntergrityExceptionPktCmd   = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        cfg.hdrIntergrityExceptionCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, &cfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* get and compare set-values and get-values */
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet(dev, &cfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(cfg.enableByteCountBoundariesCheck, cfgGet.enableByteCountBoundariesCheck);
        UTF_VERIFY_EQUAL0_PARAM_MAC(cfg.minByteCountBoundary          , cfgGet.minByteCountBoundary          );
        UTF_VERIFY_EQUAL0_PARAM_MAC(cfg.maxByteCountBoundary          , cfgGet.maxByteCountBoundary          );
        UTF_VERIFY_EQUAL0_PARAM_MAC(cfg.enableIpv4HdrCheckByChecksum  , cfgGet.enableIpv4HdrCheckByChecksum  );
        UTF_VERIFY_EQUAL0_PARAM_MAC(cfg.hdrIntergrityExceptionPktCmd  , cfgGet.hdrIntergrityExceptionPktCmd  );
        UTF_VERIFY_EQUAL0_PARAM_MAC(cfg.hdrIntergrityExceptionCpuCode , cfgGet.hdrIntergrityExceptionCpuCode );

        /* bad parameters -  cfgPtr*/
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, NULL/*&cfg*/);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* bad parameters -  minByteCountBoundary*/
        cfg.enableByteCountBoundariesCheck = GT_TRUE;
        cfg.minByteCountBoundary           = 0x4000;
        cfg.maxByteCountBoundary           = 0x3FFF;
        cfg.enableIpv4HdrCheckByChecksum   = GT_TRUE;
        cfg.hdrIntergrityExceptionPktCmd   = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        cfg.hdrIntergrityExceptionCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, &cfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* bad parameters -  maxByteCountBoundary*/
        cfg.enableByteCountBoundariesCheck = GT_TRUE;
        cfg.minByteCountBoundary           = 0;
        cfg.maxByteCountBoundary           = 0x4000;
        cfg.enableIpv4HdrCheckByChecksum   = GT_TRUE;
        cfg.hdrIntergrityExceptionPktCmd   = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        cfg.hdrIntergrityExceptionCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, &cfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* bad parameters - hdrIntergrityExceptionPktCmd */
        cfg.enableByteCountBoundariesCheck = GT_TRUE;
        cfg.minByteCountBoundary           = 0;
        cfg.maxByteCountBoundary           = 0x3FFF;
        cfg.enableIpv4HdrCheckByChecksum   = GT_TRUE;
        cfg.hdrIntergrityExceptionPktCmd   = CPSS_PACKET_CMD_ROUTE_E;
        cfg.hdrIntergrityExceptionCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, &cfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* bad parameters - hdrIntergrityExceptionPktCmd */
        cfg.enableByteCountBoundariesCheck = GT_TRUE;
        cfg.minByteCountBoundary           = 0;
        cfg.maxByteCountBoundary           = 0x3FFF;
        cfg.enableIpv4HdrCheckByChecksum   = GT_TRUE;
        cfg.hdrIntergrityExceptionPktCmd   = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        cfg.hdrIntergrityExceptionCpuCode  = CPSS_NET_ALL_CPU_OPCODES_E;
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, &cfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* correct parameters */
    cfg.enableByteCountBoundariesCheck = GT_TRUE;
    cfg.minByteCountBoundary           = 0;
    cfg.maxByteCountBoundary           = 0x3FFF;
    cfg.enableIpv4HdrCheckByChecksum   = GT_TRUE;
    cfg.hdrIntergrityExceptionPktCmd   = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    cfg.hdrIntergrityExceptionCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, &cfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(dev, &cfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet)
{
    GT_STATUS          st  = GT_OK;
    GT_U8              dev = 0;
    CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  cfg;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* correct parameters */
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet(dev, &cfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* bad parameters -  cfgPtr*/
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet(dev, NULL/*&cfg*/);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 2. Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet(dev, &cfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet(dev, &cfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPortByteCountUpdateSet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL     toSubtractOrToAdd;
    GT_U32      subtractedOrAddedValue;
    GT_BOOL     toSubtractOrToAddGet;
    GT_U32      subtractedOrAddedValueGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /* correct parameters */
            toSubtractOrToAdd      = GT_FALSE;
            subtractedOrAddedValue = 7;
            st = cpssDxChCutThroughPortByteCountUpdateSet(
                dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
            UTF_VERIFY_EQUAL4_PARAM_MAC(
                GT_OK, st, dev, port, toSubtractOrToAdd, subtractedOrAddedValue);

            /* get and compare set-values and get-values */
            st = cpssDxChCutThroughPortByteCountUpdateGet(
                dev, port, &toSubtractOrToAddGet, &subtractedOrAddedValueGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(toSubtractOrToAdd, toSubtractOrToAddGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(subtractedOrAddedValue, subtractedOrAddedValueGet);

            /* bad parameters: subtractedOrAddedValue*/
            toSubtractOrToAdd      = GT_FALSE;
            subtractedOrAddedValue = 8;
            st = cpssDxChCutThroughPortByteCountUpdateSet(
                dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(
                GT_OK, st, dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
        }

        /* correct parameters */
        toSubtractOrToAdd      = GT_FALSE;
        subtractedOrAddedValue = 7;

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCutThroughPortByteCountUpdateSet(
                dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCutThroughPortByteCountUpdateSet(
            dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCutThroughPortByteCountUpdateSet(
            dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* correct parameters */
    toSubtractOrToAdd      = GT_FALSE;
    subtractedOrAddedValue = 7;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPortByteCountUpdateSet(
            dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughPortByteCountUpdateSet(
        dev, port, toSubtractOrToAdd, subtractedOrAddedValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChCutThroughPortByteCountUpdateGet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL     toSubtractOrToAdd;
    GT_U32      subtractedOrAddedValue;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /* correct parameters */
            st = cpssDxChCutThroughPortByteCountUpdateGet(
                dev, port, &toSubtractOrToAdd, &subtractedOrAddedValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* bad parameters: toSubtractOrToAddPtr */
            st = cpssDxChCutThroughPortByteCountUpdateGet(
                dev, port, NULL/*&toSubtractOrToAdd*/, &subtractedOrAddedValue);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* bad parameters: subtractedOrAddedValuePtr */
            st = cpssDxChCutThroughPortByteCountUpdateGet(
                dev, port, &toSubtractOrToAdd, NULL/*&subtractedOrAddedValue*/);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCutThroughPortByteCountUpdateGet(
                dev, port, &toSubtractOrToAdd, &subtractedOrAddedValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCutThroughPortByteCountUpdateGet(
            dev, port, &toSubtractOrToAdd, &subtractedOrAddedValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCutThroughPortByteCountUpdateGet(
            dev, port, &toSubtractOrToAdd, &subtractedOrAddedValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCutThroughPortByteCountUpdateGet(
            dev, port, &toSubtractOrToAdd, &subtractedOrAddedValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCutThroughPortByteCountUpdateGet(
        dev, port, &toSubtractOrToAdd, &subtractedOrAddedValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChCutThrough suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChCutThrough)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughMemoryRateLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughMemoryRateLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughMinimalPacketSizeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughMinimalPacketSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPortEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPortEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughUpEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughUpEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughVlanEthertypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughVlanEthertypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughBypassRouterAndPolicerEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughBypassRouterAndPolicerEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughBypassModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughBypassModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPortGroupMaxBuffersLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPortGroupMaxBuffersLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughUdeCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughUdeCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPassengerParsingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPassengerParsingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughByteCountExtractFailsCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPortByteCountUpdateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCutThroughPortByteCountUpdateGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChCutThrough)

