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
* @file cpssDxChNstPortIsolationUT.c
*
* @brief Unit tests for cpssDxChNstPortIsolation, that provides
* Network Shield Technology facility Cheetah CPSS declarations
*
* @version   23
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>

#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
#define NST_VALID_PHY_PORT_CNS  0

#define UTF_PRV_MC_RELAY_PORTS_BMP_CNS 0x10001000

/* get HW device number */
#define UTF_HW_DEV_NUM_MAC(_devNum) \
    (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ? PRV_CPSS_HW_DEV_NUM_MAC(_devNum) : _devNum)


/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

/*even if the device support 512 port , still the port isolation not supports more than 256 */
#define PORT_NUM_MAX_PORT_ISOLATION_CNS 256

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationEnableSet(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1. Call with enable [GT_FALSE/ GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChNstPortIsolationEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable = GT_FALSE and GT_TRUE.   */
        /* Expected: GT_OK.                                             */
        enable = GT_FALSE;

        /*call with enable = GT_FALSE */
        st = cpssDxChNstPortIsolationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChNstPortIsolationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call cpssDxChNstPortIsolationEnableGet.   */
        /* Expected: GT_OK and the same value.                          */

        enable = GT_FALSE;

        /*call with enable = GT_FALSE */
        st = cpssDxChNstPortIsolationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*checking value */
        st = cpssDxChNstPortIsolationEnableGet(dev, &enableGet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                  "got another enable than was set: %d", dev);

        /*call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChNstPortIsolationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*checking value */
        st = cpssDxChNstPortIsolationEnableGet(dev, &enableGet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                  "got another enable than was set: %d", dev);
        enable = GT_TRUE;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE    */

    st = cpssDxChNstPortIsolationEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationEnableGet(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationEnableGet)
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with non null enable pointer.   */
        /* Expected: GT_OK.                                             */

        st = cpssDxChNstPortIsolationEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call function for with null enable pointer [NULL].      */
        /* Expected: GT_BAD_PTR.                                        */
        st = cpssDxChNstPortIsolationEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationPortAdd
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        srcInterface,
    IN GT_U8                                          portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationPortAdd)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                             and  srcInterface.type [CPSS_INTERFACE_PORT_E].
    Expected: GT_OK.
    1.1.2. Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                             and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
    Expected: GT_OK.
    1.1.3. Call with srcInterface.type [CPSS_INTERFACE_PORT_E] and out of range
                                devNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                           and portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                             and  srcInterface.type [CPSS_INTERFACE_FABRIC_VIDX_E].
    Expected: NOT GT_OK.
    1.1.5. Call with wrong enum values trafficType.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with wrong enum values port.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                      st       = GT_OK;

    GT_U8                                          dev;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_PHYSICAL_PORT_NUM                           port; /* target port in the bmp of the entry */
    GT_PHYSICAL_PORT_NUM                           utPhysicalPort;/*  port for 'UT iterations' */

    GT_U32   numberOfPortBits;
    GT_U32   numberOfDeviceBits;
    GT_U32   numberOfTrunkBits;

    trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

    cpssOsBzero((GT_VOID*) &srcInterface, sizeof(CPSS_INTERFACE_INFO_STC));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChNstPortIsolationLookupBitsGet(dev,
                                                   &numberOfPortBits,
                                                   &numberOfDeviceBits,
                                                   &numberOfTrunkBits);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        else
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;

            /*
                1.1.1. Call with trafficType
                       [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                        and  srcInterface.type [CPSS_INTERFACE_PORT_E].
                Expected: GT_OK.
            */

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;
            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
            srcInterface.devPort.portNum = port;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                srcInterface.devPort.portNum &= BIT_MASK_MAC(numberOfPortBits);
            }

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*
                1.1.2. Call with trafficType
                       [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                        and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                Expected: GT_OK.
            */

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*
                1.1.3. Call with srcInterface.type [CPSS_INTERFACE_PORT_E] and out of range
                                            devNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                                       and portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
                Expected: GT_BAD_PARAM.
            */

            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;

            /* call with wrong devNum */
            srcInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);

            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            /* call with wrong portNum */
            srcInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);

            srcInterface.devPort.portNum = port;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                srcInterface.devPort.portNum &= BIT_MASK_MAC(numberOfPortBits);
            }

            /*
                1.1.4. Call with trafficType
                           [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E],
                      and  srcInterface.type [CPSS_INTERFACE_FABRIC_VIDX_E].
                Expected: NOT GT_OK.
            */
            trafficType       = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;
            srcInterface.type = CPSS_INTERFACE_FABRIC_VIDX_E;

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*
                1.1.5. Call with wrong enum values trafficType
                        and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                Expected: GT_BAD_PARAM.
            */
            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationPortAdd
                                (dev, trafficType, &srcInterface, port),
                                trafficType);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;

            /* 1.2.1. Call function for each non-active port    */

            st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
            if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev) ||
               port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                /* the device supports ports up to PRV_CPSS_DXCH_MAX_PHY_PORT_MAC(dev) */
                /* regardless to physical ports */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* the EGF already supports 256 ports */
            port = 256;
        }
        else
            port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;
    port         = NST_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationPortAdd(dev, trafficType, &srcInterface, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationPortDelete
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        srcInterface,
    IN GT_U8                                          portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationPortDelete)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                             and  srcInterface.type [CPSS_INTERFACE_PORT_E].
    Expected: GT_OK.
    1.1.2. Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                             and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
    Expected: GT_OK.
*/
    GT_STATUS                                      st       = GT_OK;

    GT_U8                                          dev;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_PHYSICAL_PORT_NUM                           port; /* target port in the bmp of the entry */
    GT_PHYSICAL_PORT_NUM                           utPhysicalPort;/*  port for 'UT iterations' */

    GT_U32   numberOfPortBits;
    GT_U32   numberOfDeviceBits;
    GT_U32   numberOfTrunkBits;

    trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

    cpssOsBzero((GT_VOID*) &srcInterface, sizeof(CPSS_INTERFACE_INFO_STC));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChNstPortIsolationLookupBitsGet(dev,
                                                   &numberOfPortBits,
                                                   &numberOfDeviceBits,
                                                   &numberOfTrunkBits);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        else
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;

            /*
                1.1.1. Call with trafficType
                       [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                        and  srcInterface.type [CPSS_INTERFACE_PORT_E].
                Expected: GT_OK.
            */

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;
            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
            srcInterface.devPort.portNum = port;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                srcInterface.devPort.portNum &= BIT_MASK_MAC(numberOfPortBits);
            }

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*
                1.1.2. Call with trafficType
                       [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
                        and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                Expected: GT_OK.
            */

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(port >= PORT_NUM_MAX_PORT_ISOLATION_CNS)
            {
                /* support BC3 in 512 ports mode*/
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st, dev, port, trafficType);
            }
            else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            /*
                1.1.3. Call with wrong enum values trafficType
                Expected: GT_BAD_PARAM.
            */
            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationPortDelete
                                (dev, trafficType, &srcInterface, port),
                                trafficType);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;

            /* 1.2.1. Call function for each non-active port    */

            st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
            if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev) ||
               port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                /* the device supports ports up to PRV_CPSS_DXCH_MAX_PHY_PORT_MAC(dev) */
                /* regardless to physical ports */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* the EGF already supports 256 ports */
            port = 256;
        }
        else
            port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;
    port         = NST_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationPortDelete(dev, trafficType, &srcInterface, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationTableEntrySet
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        srcInterface,
    IN GT_BOOL                                        cpuPortMember,
    IN CPSS_PORTS_BMP_STC                             *localPortsMembersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationTableEntrySet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
          and  srcInterface.type [CPSS_INTERFACE_TRUNK_E / CPSS_INTERFACE_TRUNK_E].
                   cpuPortMember [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
    1.1.3. Call with srcInterface.type [CPSS_INTERFACE_PORT_E] and out of range
                                devNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                           and portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong enum values trafficType.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with incorrect localPortsMembersPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                      st       = GT_OK;

    GT_U8                                          dev;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_BOOL                                        cpuPortMember = GT_FAIL;
    CPSS_PORTS_BMP_STC                             localPortsMembers;

    GT_BOOL                                        cpuPortMemberGet;
    GT_BOOL                                        expectedCpuPortMember;
    GT_U32                                         tmpValue;
    CPSS_PORTS_BMP_STC                             localPortsMembersGet;

    GT_PORT_NUM                                    port; /* src port of the interface */
    GT_BOOL                                        checkNumOfBits = GT_FALSE;
    GT_U32                                         numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits;
    GT_U32  numBitsForPort = 0;
    GT_U32  numBitsForDevice = 0;
    GT_U32  numBitsForTrunk = 0;

    port = NST_VALID_PHY_PORT_CNS;

    trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

    cpssOsBzero((GT_VOID*) &srcInterface, sizeof(CPSS_INTERFACE_INFO_STC));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get number of bits for index calculation */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBits, &numberOfDeviceBits, &numberOfTrunkBits);
        if (st == GT_OK)
        {
            checkNumOfBits = GT_TRUE;
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with trafficType
                               [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E],
                       srcInterface.type [CPSS_INTERFACE_TRUNK_E /
                                          CPSS_INTERFACE_TRUNK_E].
                       and cpuPortMember [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;
            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
            srcInterface.devPort.portNum = port;

            if(GT_TRUE == checkNumOfBits &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            cpuPortMember                = GT_TRUE;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            if ((GT_BAD_PARAM == st) && (GT_TRUE == checkNumOfBits))
            {
                if ((port > BIT_MASK_MAC(numberOfPortBits)) ||
                    (srcInterface.devPort.hwDevNum > BIT_MASK_MAC(numberOfDeviceBits)))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);
                }
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);
            }
            else
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember parameter and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);


            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            if ((GT_BAD_PARAM == st) && (GT_TRUE == checkNumOfBits))
            {
                if ((port > BIT_MASK_MAC(numberOfPortBits)) ||
                    (srcInterface.devPort.hwDevNum > BIT_MASK_MAC(numberOfDeviceBits)))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);
                }
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);
            }
            else
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember parameter and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            cpuPortMember                = GT_FALSE;
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);

            localPortsMembers.ports[0] = 0x1FFF1EEE;
            localPortsMembers.ports[1] = 0x1DDD1CCC;
            localPortsMembers.ports[2] = 0x1BBB1AAA;
            localPortsMembers.ports[3] = 0x15551777;
            /* remove ports that not relevant to the device */
            prvCpssDrvPortsBmpMaskWithMaxPorts(dev,&localPortsMembers);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_FALSE &&
               PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
            {
                /* port 12 'in each port gorup' is internal port and should be set by the application.
                the CPSS uses it internally , so the 'Get' will not return those ports even set by the application !
                */
                localPortsMembers.ports[0] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[1] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[2] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[3] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
            }

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember parameter and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);

            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);

            localPortsMembers.ports[0] = 0x1FFF1EEE;
            localPortsMembers.ports[1] = 0x1DDD1CCC;
            localPortsMembers.ports[2] = 0x1BBB1AAA;
            localPortsMembers.ports[3] = 0x15551777;
            /* remove ports that not relevant to the device */
            prvCpssDrvPortsBmpMaskWithMaxPorts(dev,&localPortsMembers);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_FALSE &&
               PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
            {
                /* port 12 'in each port gorup' is internal port and should be set by the application.
                the CPSS uses it internally , so the 'Get' will not return those ports even set by the application !
                */
                localPortsMembers.ports[0] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[1] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[2] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[3] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
            }

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember parameter and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);
            }

            /*
                1.1.1. Call with trafficType
                               [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E],
                       srcInterface.type [CPSS_INTERFACE_TRUNK_E /
                                          CPSS_INTERFACE_TRUNK_E].
                       and cpuPortMember [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;
            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
            srcInterface.devPort.portNum = port;

            if(GT_TRUE == checkNumOfBits &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            cpuPortMember                = GT_TRUE;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);

            localPortsMembers.ports[0] =0x6FFFEFFF;
            localPortsMembers.ports[1] =0x6FFFEFFF;
            localPortsMembers.ports[2] =0x6FFFEFFF;
            localPortsMembers.ports[3] =0x6FFFEFFF;
            /* remove ports that not relevant to the device */
            prvCpssDrvPortsBmpMaskWithMaxPorts(dev,&localPortsMembers);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_FALSE &&
               PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
            {
                /* port 12 'in each port gorup' is internal port and should be set by the application.
                the CPSS uses it internally , so the 'Get' will not return those ports even set by the application !
                */
                localPortsMembers.ports[0] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[1] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[2] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[3] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
            }

            if ((GT_BAD_PARAM == st) && (GT_TRUE == checkNumOfBits))
            {
                if ((port > BIT_MASK_MAC(numberOfPortBits)) ||
                    (srcInterface.devPort.hwDevNum > BIT_MASK_MAC(numberOfDeviceBits)))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);
                }
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);
            }
            else
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember parameter and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);

            localPortsMembers.ports[0] =0x6FFFEFFF;
            localPortsMembers.ports[1] =0x6FFFEFFF;
            localPortsMembers.ports[2] =0x6FFFEFFF;
            localPortsMembers.ports[3] =0x6FFFEFFF;
            /* remove ports that not relevant to the device */
            prvCpssDrvPortsBmpMaskWithMaxPorts(dev,&localPortsMembers);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_FALSE &&
               PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
            {
                /* port 12 'in each port gorup' is internal port and should be set by the application.
                the CPSS uses it internally , so the 'Get' will not return those ports even set by the application !
                */
                localPortsMembers.ports[0] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[1] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[2] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[3] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
            }

            if ((GT_BAD_PARAM == st) && (GT_TRUE == checkNumOfBits))
            {
                if ((port > BIT_MASK_MAC(numberOfPortBits)) ||
                    (srcInterface.devPort.hwDevNum > BIT_MASK_MAC(numberOfDeviceBits)))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);
                }
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);
            }
            else
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember parameter and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            cpuPortMember                = GT_FALSE;
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);

            localPortsMembers.ports[0] =0xFFFFFFFF;
            localPortsMembers.ports[1] =0xFFFFFFFF;
            localPortsMembers.ports[2] =0xFFFFFFFF;
            localPortsMembers.ports[3] =0xFFFFFFFF;
            /* remove ports that not relevant to the device */
            prvCpssDrvPortsBmpMaskWithMaxPorts(dev,&localPortsMembers);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_FALSE &&
               PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
            {
                /* port 12 'in each port gorup' is internal port and should be set by the application.
                the CPSS uses it internally , so the 'Get' will not return those ports even set by the application !
                */
                localPortsMembers.ports[0] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[1] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[2] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[3] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
            }

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember parameter and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);

            localPortsMembers.ports[0] =0xFFFFFFFF;
            localPortsMembers.ports[1] =0xFFFFFFFF;
            localPortsMembers.ports[2] =0xFFFFFFFF;
            localPortsMembers.ports[3] =0xFFFFFFFF;
            /* remove ports that not relevant to the device */
            prvCpssDrvPortsBmpMaskWithMaxPorts(dev,&localPortsMembers);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_FALSE &&
               PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
            {
                /* port 12 'in each port gorup' is internal port and should be set by the application.
                the CPSS uses it internally , so the 'Get' will not return those ports even set by the application !
                */
                localPortsMembers.ports[0] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[1] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[2] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
                localPortsMembers.ports[3] &= ~UTF_PRV_MC_RELAY_PORTS_BMP_CNS;
            }

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
                    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
                */

                st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                    &srcInterface, &cpuPortMemberGet, &localPortsMembersGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

                /*  Calculate expected cpuPortMember  parameters  and mask bitmap to ignore CPU bit  */
                if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[0] >> 28) & 1);
                    localPortsMembers.ports[0] &= ~(1<<28);
                }
                else
                {
                    tmpValue = BIT2BOOL_MAC((localPortsMembers.ports[1] >> 31) & 1);
                    localPortsMembers.ports[1] &= ~(1<<31);
                }
                expectedCpuPortMember = BIT2BOOL_MAC(tmpValue) || cpuPortMember;

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuPortMember, cpuPortMemberGet,
                        "get another cpuPortMember than was set: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[0], localPortsMembersGet.ports[0],
                        "get another localPortsMembers.ports[0] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[1], localPortsMembersGet.ports[1],
                        "get another localPortsMembers.ports[1] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[2], localPortsMembersGet.ports[2],
                        "get another localPortsMembers.ports[2] than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(localPortsMembers.ports[3], localPortsMembersGet.ports[3],
                        "get another localPortsMembers.ports[3] than was set: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with srcInterface.type [CPSS_INTERFACE_PORT_E] and out of range
                                            devNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                                       and portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
                Expected: GT_BAD_PARAM.
            */

            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;

            /* call with wrong devNum */
            srcInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);

            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);

            if(GT_TRUE == checkNumOfBits &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            /* call with wrong portNum */
            srcInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);

            srcInterface.devPort.portNum = port;

            /*
                1.1.4. Call with wrong enum values trafficType
                        and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                Expected: GT_BAD_PARAM.
            */
            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationTableEntrySet
                                (dev, trafficType, &srcInterface,
                                 cpuPortMember, &localPortsMembers),
                                trafficType);

            /*
                1.1.5. Call with incorrect localPortsMembersPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, trafficType);
        }

        st = cpssDxChNstPortIsolationLookupBitsGet(dev,&numBitsForPort,&numBitsForDevice,&numBitsForTrunk);
        if(st != GT_OK)
        {
            numBitsForPort = 0;
            numBitsForDevice = 0;
            numBitsForTrunk = 0;
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        srcInterface.type            = CPSS_INTERFACE_PORT_E;
        srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);

        if(GT_TRUE == checkNumOfBits &&
            ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
        {
            srcInterface.devPort.hwDevNum =
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
        }

        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            srcInterface.devPort.portNum = port;

            st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                                &srcInterface, cpuPortMember, &localPortsMembers);
            if((port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev)) &&
               ((numBitsForPort == 0) || (BIT_MASK_MAC(numBitsForPort) < port))
               )
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                /* the device supports ports up to PRV_CPSS_DXCH_MAX_PHY_PORT_MAC(dev) */
                /* regardless to physical ports */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                            &srcInterface, cpuPortMember, &localPortsMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationTableEntrySet(dev, trafficType,
                        &srcInterface, cpuPortMember, &localPortsMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationTableEntryGet
(
    IN  GT_U8                                          devNum,
    IN  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN  CPSS_INTERFACE_INFO_STC                        srcInterface,
    OUT GT_BOOL                                       *cpuPortMemberPtr,
    OUT CPSS_PORTS_BMP_STC                            *localPortsMembersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationTableEntryGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                  CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]
          and  srcInterface.type [CPSS_INTERFACE_TRUNK_E / CPSS_INTERFACE_TRUNK_E].
                   cpuPortMember [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChNstPortIsolationTableEntryGet.
    Expected: GT_OK and the same cpuPortMember and localPortsMembers.
    1.1.3. Call with srcInterface.type [CPSS_INTERFACE_PORT_E] and out of range
                                devNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                           and portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong enum values trafficType.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with incorrect localPortsMembersPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                      st       = GT_OK;

    GT_U8                                          dev;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_BOOL                                        cpuPortMember = GT_FALSE;
    CPSS_PORTS_BMP_STC                             localPortsMembers;

    GT_PORT_NUM                                    port; /* src port of the interface */
    GT_BOOL                                        checkNumOfBits = GT_FALSE;
    GT_U32                                         numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits;
    GT_U32  numBitsForPort = 0;
    GT_U32  numBitsForDevice = 0;
    GT_U32  numBitsForTrunk = 0;

    port = NST_VALID_PHY_PORT_CNS;

    trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

    cpssOsBzero((GT_VOID*) &srcInterface, sizeof(CPSS_INTERFACE_INFO_STC));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get number of bits for index calculation */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBits, &numberOfDeviceBits, &numberOfTrunkBits);
        if (st == GT_OK)
        {
            checkNumOfBits = GT_TRUE;
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with trafficType
                               [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E /
                                CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E],
                       srcInterface.type [CPSS_INTERFACE_TRUNK_E /
                                          CPSS_INTERFACE_TRUNK_E].
                Expected: GT_OK.
            */

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;
            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
            srcInterface.devPort.portNum = port;

            if(GT_TRUE == checkNumOfBits &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                &srcInterface, &cpuPortMember, &localPortsMembers);
            if ((GT_BAD_PARAM == st) && (GT_TRUE == checkNumOfBits))
            {
                if ((port > BIT_MASK_MAC(numberOfPortBits)) ||
                    (srcInterface.devPort.hwDevNum > BIT_MASK_MAC(numberOfDeviceBits)))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);
                }
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);
            }

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call with wrong enum values trafficType
                            and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                    Expected: GT_BAD_PARAM.
                */
                srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
                srcInterface.trunkId  = 0;
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

                UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationTableEntryGet
                                    (dev, trafficType, &srcInterface,
                                     &cpuPortMember, &localPortsMembers),
                                    trafficType);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                &srcInterface, &cpuPortMember, &localPortsMembers);
            if ((GT_BAD_PARAM == st) && (GT_TRUE == checkNumOfBits))
            {
                if ((port > BIT_MASK_MAC(numberOfPortBits)) ||
                    (srcInterface.devPort.hwDevNum > BIT_MASK_MAC(numberOfDeviceBits)))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);
                }
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);
            }
            else
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call with wrong enum values trafficType
                            and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                    Expected: GT_BAD_PARAM.
                */
                srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
                srcInterface.trunkId  = 0;
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

                UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationTableEntryGet
                                    (dev, trafficType, &srcInterface,
                                     &cpuPortMember, &localPortsMembers),
                                    trafficType);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
            srcInterface.trunkId  = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                &srcInterface, &cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call with wrong enum values trafficType
                            and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                    Expected: GT_BAD_PARAM.
                */
                srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
                srcInterface.trunkId  = 0;
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

                UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationTableEntryGet
                                    (dev, trafficType, &srcInterface, &cpuPortMember, &localPortsMembers),
                                    trafficType);
            }

            /*Call with trafficType [CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E]*/
            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                &srcInterface, &cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trafficType);

            if(GT_OK == st)
            {
                /*
                    1.1.2. Call with wrong enum values trafficType
                            and  srcInterface.type [CPSS_INTERFACE_TRUNK_E].
                    Expected: GT_BAD_PARAM.
                */
                srcInterface.type     = CPSS_INTERFACE_TRUNK_E;
                srcInterface.trunkId  = 0;
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

                UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationTableEntryGet
                                    (dev, trafficType, &srcInterface,
                                     &cpuPortMember, &localPortsMembers),
                                    trafficType);
            }

            /*
                1.1.3. Call with srcInterface.type [CPSS_INTERFACE_PORT_E] and out of range
                                            devNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                                       and portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
                Expected: GT_BAD_PARAM.
            */

            trafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

            srcInterface.type            = CPSS_INTERFACE_PORT_E;

            /* call with wrong devNum */
            srcInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                &srcInterface, &cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);

            srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
            if(GT_TRUE == checkNumOfBits &&
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
            {
                srcInterface.devPort.hwDevNum =
                    ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
            }

            /* call with wrong portNum */
            srcInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                &srcInterface, &cpuPortMember, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, trafficType);

            srcInterface.devPort.portNum = port;

            /*
                1.1.4. Call with incorrect cpuPortMember [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                        &srcInterface, NULL, &localPortsMembers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, trafficType);

            /*
                1.1.5. Call with incorrect localPortsMembersPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                             &srcInterface, &cpuPortMember, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, trafficType);
        }

        st = cpssDxChNstPortIsolationLookupBitsGet(dev,&numBitsForPort,&numBitsForDevice,&numBitsForTrunk);
        if(st != GT_OK)
        {
            numBitsForPort = 0;
            numBitsForDevice = 0;
            numBitsForTrunk = 0;
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        srcInterface.type            = CPSS_INTERFACE_PORT_E;
        srcInterface.devPort.hwDevNum  = UTF_HW_DEV_NUM_MAC(dev);
        if(GT_TRUE == checkNumOfBits &&
            ((srcInterface.devPort.hwDevNum << numberOfPortBits) >= _2K))/* the 'common' table size for {dev,port} */
        {
            srcInterface.devPort.hwDevNum =
                ((srcInterface.devPort.hwDevNum << numberOfPortBits) % _2K) >> numberOfPortBits;
        }

        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            srcInterface.devPort.portNum = port;

            st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                                &srcInterface, &cpuPortMember, &localPortsMembers);
            if((port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev)) &&
               ((numBitsForPort == 0) || (BIT_MASK_MAC(numBitsForPort) < port))
               )
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                /* the device supports ports up to PRV_CPSS_DXCH_MAX_PHY_PORT_MAC(dev) */
                /* regardless to physical ports */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                            &srcInterface, &cpuPortMember, &localPortsMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationTableEntryGet(dev, trafficType,
                        &srcInterface, &cpuPortMember, &localPortsMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationLookupTrunkIndexBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   trunkIndexBase
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationLookupTrunkIndexBaseSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with trunkIndexBase [0 / 1000 / 2047 / 2048]
    Expected: GT_OK.
    1.2. Call cpssDxChNstPortIsolationLookupTrunkIndexBaseGet with not NULL trunkIndexBasePtr.
    Expected: GT_OK and the same trunkIndexBase.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      trunkIndexBase    = 0;
    GT_U32      trunkIndexBaseGet = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkIndexBase [0 / 1000 / 2047 / 2048]
            Expected: GT_OK.
        */
        /* call with trunkIndexBase = 0 */
        trunkIndexBase = 0;

        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseSet(dev, trunkIndexBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkIndexBase);

        /*
            1.2. Call cpssDxChNstPortIsolationLookupTrunkIndexBaseGet with not NULL trunkIndexBasePtr.
            Expected: GT_OK and the same trunkIndexBase.
        */
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, &trunkIndexBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationLookupTrunkIndexBaseGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(trunkIndexBase, trunkIndexBaseGet,
                       "get another trunkIndexBase than was set: %d", dev);

        /* call with trunkIndexBase = 1000 */
        trunkIndexBase = 1000;

        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseSet(dev, trunkIndexBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkIndexBase);

        /*
            1.2. Call cpssDxChNstPortIsolationLookupTrunkIndexBaseGet with not NULL trunkIndexBasePtr.
            Expected: GT_OK and the same trunkIndexBase.
        */
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, &trunkIndexBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationLookupTrunkIndexBaseGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(trunkIndexBase, trunkIndexBaseGet,
                       "get another trunkIndexBase than was set: %d", dev);

        /* call with trunkIndexBase = 2047 */
        trunkIndexBase = 2047;

        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseSet(dev, trunkIndexBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkIndexBase);

        /*
            1.2. Call cpssDxChNstPortIsolationLookupTrunkIndexBaseGet with not NULL trunkIndexBasePtr.
            Expected: GT_OK and the same trunkIndexBase.
        */
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, &trunkIndexBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationLookupTrunkIndexBaseGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(trunkIndexBase, trunkIndexBaseGet,
                       "get another trunkIndexBase than was set: %d", dev);

        /* call with trunkIndexBase = 2048 */
        trunkIndexBase = 2048;

        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseSet(dev, trunkIndexBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkIndexBase);

        /*
            1.2. Call cpssDxChNstPortIsolationLookupTrunkIndexBaseGet with not NULL trunkIndexBasePtr.
            Expected: GT_OK and the same trunkIndexBase.
        */
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, &trunkIndexBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationLookupTrunkIndexBaseGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(trunkIndexBase, trunkIndexBaseGet,
                       "get another trunkIndexBase than was set: %d", dev);
    }

    trunkIndexBase = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseSet(dev, trunkIndexBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationLookupTrunkIndexBaseSet(dev, trunkIndexBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationLookupTrunkIndexBaseGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *trunkIndexBasePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationLookupTrunkIndexBaseGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non null trunkIndexBasePtr.
    Expected: GT_OK.
    1.2. Call with trunkIndexBasePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      trunkIndexBase = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null trunkIndexBasePtr.
            Expected: GT_OK.
        */
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, &trunkIndexBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with trunkIndexBasePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, trunkIndexBasePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, &trunkIndexBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(dev, &trunkIndexBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationModeSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationModeSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with mode [CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E /
                           CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E /
                           CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E /
                           CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChNstPortIsolationModeGet with non NULL modePtr.
    Expected: GT_OK and the same mode.
    1.1.3. Call with mode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT    mode    = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E;
    CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT    modeGet = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with mode [CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E /
                                       CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E /
                                       CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E /
                                       CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E].
                Expected: GT_OK.
            */
            /* call with mode = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E */
            mode = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E;

            st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.1.2. Call cpssDxChNstPortIsolationModeGet with non NULL modePtr.
                Expected: GT_OK and the same mode.
            */
            st = cpssDxChNstPortIsolationModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationModeGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d, %d", dev, port);

            /* call with mode = CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E */
            mode = CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E;

            st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.1.2. Call cpssDxChNstPortIsolationModeGet with non NULL modePtr.
                Expected: GT_OK and the same mode.
            */
            st = cpssDxChNstPortIsolationModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationModeGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d, %d", dev, port);

            /* call with mode = CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E */
            mode = CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E;

            st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.1.2. Call cpssDxChNstPortIsolationModeGet with non NULL modePtr.
                Expected: GT_OK and the same mode.
            */
            st = cpssDxChNstPortIsolationModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationModeGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d, %d", dev, port);

            /* call with mode = CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E */
            mode = CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E;

            st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.1.2. Call cpssDxChNstPortIsolationModeGet with non NULL modePtr.
                Expected: GT_OK and the same mode.
            */
            st = cpssDxChNstPortIsolationModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationModeGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with mode [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIsolationModeSet
                                (dev, port, mode),
                                mode);
        }

        mode = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    mode = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_NUM                             portNum,
    OUT CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationModeGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL modePtr.
    Expected: GT_OK.
    1.1.2. Call with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT   mode = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL modePtr.
                Expected: GT_OK.
            */
            st = cpssDxChNstPortIsolationModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with modePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChNstPortIsolationModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNstPortIsolationModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNstPortIsolationModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNstPortIsolationModeGet(dev, port, &mode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationLookupBitsSet
(
    IN GT_U8    devNum,
    IN GT_U32   numberOfPortBits,
    IN GT_U32   numberOfDeviceBits,
    IN GT_U32   numberOfTrunkBits
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationLookupBitsSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with numberOfPortBits [0 / 10 / 15],
                   numberOfDeviceBits [0 / 10 / 15],
                   numberOfTrunkBits [0 / 10 / 15]
    Expected: GT_OK.
    1.2. Call cpssDxChNstPortIsolationLookupBitsGet with not NULL Ptrs
    Expected: GT_OK and the same values.
    1.3. Call with out of range numberOfPortBits [16]
                   and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range numberOfDeviceBits [16]
                   and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range numberOfTrunkBits [16]
                   and other valid params.
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      numberOfPortBits      = 0;
    GT_U32      numberOfDeviceBits    = 0;
    GT_U32      numberOfTrunkBits     = 0;
    GT_U32      numberOfPortBitsGet   = 0;
    GT_U32      numberOfDeviceBitsGet = 0;
    GT_U32      numberOfTrunkBitsGet  = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with numberOfPortBits [0 / 10 / 15],
                           numberOfDeviceBits [0 / 10 / 15],
                           numberOfTrunkBits [0 / 10 / 15]
            Expected: GT_OK.
        */
        /* call with numberOfBits = 0 */
        numberOfPortBits   = 0;
        numberOfDeviceBits = 10;
        numberOfTrunkBits  = 15;

        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);

        /*
            1.2. Call cpssDxChNstPortIsolationLookupBitsGet with not NULL Ptrs
            Expected: GT_OK and the same values.
        */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBitsGet, &numberOfDeviceBitsGet, &numberOfTrunkBitsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationLookupBitsGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfPortBits, numberOfPortBitsGet,
                       "get another numberOfPortBits than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfDeviceBits, numberOfDeviceBitsGet,
                       "get another numberOfDeviceBits than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfTrunkBits, numberOfTrunkBitsGet,
                       "get another numberOfTrunkBits than was set: %d", dev);

        /* call with numberOfBits = 15 */
        numberOfPortBits   = 15;
        numberOfDeviceBits = 0;
        numberOfTrunkBits  = 10;

        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);

        /*
            1.2. Call cpssDxChNstPortIsolationLookupBitsGet with not NULL Ptrs
            Expected: GT_OK and the same values.
        */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBitsGet, &numberOfDeviceBitsGet, &numberOfTrunkBitsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationLookupBitsGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfPortBits, numberOfPortBitsGet,
                       "get another numberOfPortBits than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfDeviceBits, numberOfDeviceBitsGet,
                       "get another numberOfDeviceBits than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfTrunkBits, numberOfTrunkBitsGet,
                       "get another numberOfTrunkBits than was set: %d", dev);

        /* call with numberOfBits = 10 */
        numberOfPortBits   = 10;
        numberOfDeviceBits = 15;
        numberOfTrunkBits  = 0;

        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);

        /*
            1.2. Call cpssDxChNstPortIsolationLookupBitsGet with not NULL Ptrs
            Expected: GT_OK and the same values.
        */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBitsGet, &numberOfDeviceBitsGet, &numberOfTrunkBitsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationLookupBitsGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfPortBits, numberOfPortBitsGet,
                       "get another numberOfPortBits than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfDeviceBits, numberOfDeviceBitsGet,
                       "get another numberOfDeviceBits than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfTrunkBits, numberOfTrunkBitsGet,
                       "get another numberOfTrunkBits than was set: %d", dev);

        /*
            1.3. Call with out of range numberOfPortBits [31]
                   and other valid params.
            Expected: GT_OK.
        */
        numberOfPortBits = 31;

        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, numberOfPortBits);

        /*
            1.3. Call with out of range numberOfPortBits [32]
                   and other valid params.
            Expected: NON GT_OK.
        */
        numberOfPortBits = 32;

        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, numberOfPortBits);

        numberOfPortBits = 0;

        /*
            1.4. Call with out of range numberOfDeviceBits [16]
                           and other valid params.
            Expected: NON GT_OK.
        */
        numberOfDeviceBits = 16;

        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, numberOfDeviceBits);

        numberOfDeviceBits = 0;

        /*
            1.5. Call with out of range numberOfTrunkBits [16]
                           and other valid params.
            Expected: NON GT_OK.
        */
        numberOfTrunkBits = 16;

        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, numberOfTrunkBits);
    }

    numberOfPortBits   = 0;
    numberOfDeviceBits = 0;
    numberOfTrunkBits  = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationLookupBitsSet(dev, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationLookupBitsGet
(
    IN GT_U8    devNum,
    OUT GT_U32   *numberOfPortBitsPtr,
    OUT GT_U32   *numberOfDeviceBitsPtr,
    OUT GT_U32   *numberOfTrunkBitsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationLookupBitsGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non null numberOfPortBitsPtr,
                   non null numberOfDeviceBitsPtr,
                   non null numberOfTrunkBitsPtr.
    Expected: GT_OK.
    1.2. Call with numberOfPortBitsPtr [NULL]
                   and other valid params.
    Expected: GT_BAD_PTR.
    1.3. Call with numberOfDeviceBitsPtr [NULL]
                   and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with numberOfTrunkBitsPtr [NULL]
                   and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      numberOfPortBits   = 0;
    GT_U32      numberOfDeviceBits = 0;
    GT_U32      numberOfTrunkBits  = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null numberOfPortBitsPtr,
                           non null numberOfDeviceBitsPtr,
                           non null numberOfTrunkBitsPtr.
            Expected: GT_OK.
        */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBits, &numberOfDeviceBits, &numberOfTrunkBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with numberOfPortBitsPtr [NULL]
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, NULL, &numberOfDeviceBits, &numberOfTrunkBits);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numberOfPortBitsPtr = NULL", dev);

        /*
            1.3. Call with numberOfDeviceBitsPtr [NULL]
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBits, NULL, &numberOfTrunkBits);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numberOfDeviceBitsPtr = NULL", dev);

        /*
            1.4. Call with numberOfTrunkBitsPtr [NULL]
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBits, &numberOfDeviceBits, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numberOfTrunkBitsPtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBits, &numberOfDeviceBits, &numberOfTrunkBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationLookupBitsGet(dev, &numberOfPortBits, &numberOfDeviceBits, &numberOfTrunkBits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationOnEportsEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationOnEportsEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK.
    1.2. Call cpssDxChNstPortIsolationOnEportsEnableGet with not NULL enablePtr.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE / GT_FALSE]
            Expected: GT_OK.
        */
        /* call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChNstPortIsolationOnEportsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChNstPortIsolationOnEportsEnableGet with not NULL enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChNstPortIsolationOnEportsEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationOnEportsEnableGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);

        /* call with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChNstPortIsolationOnEportsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChNstPortIsolationOnEportsEnableGet with not NULL enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChNstPortIsolationOnEportsEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChNstPortIsolationOnEportsEnableGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationOnEportsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationOnEportsEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortIsolationOnEportsEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIsolationOnEportsEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non null enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChNstPortIsolationOnEportsEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChNstPortIsolationOnEportsEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIsolationOnEportsEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIsolationOnEportsEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChNstPortIsolation suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChNstPortIsolation)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationPortAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationPortDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationLookupTrunkIndexBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationLookupTrunkIndexBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationLookupBitsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationLookupBitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationOnEportsEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIsolationOnEportsEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChNstPortIsolation)


