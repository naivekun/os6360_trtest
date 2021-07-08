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
* @file cpssDxChBrgPrvEdgeVlanUT.c
*
* @brief Unit tests for cpssDxChBrgPrvEdgeVlan, that provides
* cpss Dx-Ch implementation for Prestera Private Edge VLANs.
*
* @version   20
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPrvEdgeVlanEnable
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPrvEdgeVlanEnable)
{
/*
    ITERATE_DEVICES (dxChx)
    1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
    1.2. Call cpssDxChBrgPrvEdgeVlanEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS                   st = GT_OK;

    GT_U8                       dev;
    GT_BOOL                     enable;
    GT_BOOL                     enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable = GT_TRUE and GT_FALSE.       */
        /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

        /* Call function with [enable==GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChBrgPrvEdgeVlanEnable(dev, enable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "Cheetah/Cheetah2 device: %d, %d",
                                         dev, enable);

        /* 1.2.  */
        st = cpssDxChBrgPrvEdgeVlanEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Cheetah/Cheetah2 device: %d",
                                         dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d", dev);

        /* Call function with [enable==GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChBrgPrvEdgeVlanEnable(dev, enable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "Cheetah/Cheetah2 device: %d, %d",
                                         dev, enable);

        /* 1.2.  */
        st = cpssDxChBrgPrvEdgeVlanEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Cheetah/Cheetah2 device: %d",
                                         dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPrvEdgeVlanEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE */

    st = cpssDxChBrgPrvEdgeVlanEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgPrvEdgeVlanEnableGet)
{
/*
    ITERATE_DEVICES (dxChx)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;

    GT_U8                       dev;
    GT_BOOL                     enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.  */
        st = cpssDxChBrgPrvEdgeVlanEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Cheetah/Cheetah2 device: %d",
                                         dev);

        /* 1.2.  */
        st = cpssDxChBrgPrvEdgeVlanEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "Cheetah/Cheetah2 device: %d",
                                         dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPrvEdgeVlanEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE */

    st = cpssDxChBrgPrvEdgeVlanEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPrvEdgeVlanPortEnable
(
    IN GT_U8      devNum,
    IN GT_U8      portNum,
    IN GT_BOOL    enable,
    IN GT_U8      dstPort,
    IN GT_U8      dstHwDev,
    IN GT_BOOL    dstTrunk
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPrvEdgeVlanPortEnable)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (dxChx) (devNum, portNum)
    1.1.1. Call function with enable [GT_FALSE] and
           dstPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (should be ignored),
           dstHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS] (should be ignored),
           dstTrunk [GT_TRUE and GT_FALSE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
    1.1.2. Check for out of range dstHwDev.  For Cheetah device call function
           with enable [GT_TRUE], dstTrunk [GT_FALSE],
           dstHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS], dstPort [0].
    Expected: GT_BAD_PARAM.
    1.1.3. Check for out of range dstPort.  For Cheetah device call function
           with enable [GT_TRUE], dstTrunk [GT_FALSE], dstHwDev [0],
           dstPort [PRV_CPSS_MAX_PP_DEVICES_CNS].
    Expected: GT_BAD_PARAM.
    1.1.4. Check for trunk-reference support. Call function with enable [GT_TRUE],
           dstTrunk [GT_TRUE] and dstPort [15],
           destDev [PRV_CPSS_MAX_PP_DEVICES_CNS] (should be ignored).
    Expected: GT_OK for Cheetah device and GT_BAD_PARAM for others.
    1.1.5. Check for out of range trunk id while it's used. For Cheetah device
           call function with dstTrunk [GT_TRUE] and dstPort
           [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128] (trunkId), destDev [0].
    Expected: NON GT_OK.
*/
    GT_STATUS                   st = GT_OK;

    GT_BOOL                     enable;
    GT_PORT_NUM                 portNum;
    GT_U8                       devNum;
    GT_PORT_NUM                 dstPort;
    GT_HW_DEV_NUM               dstHwDev;
    GT_BOOL                     dstTrunk;
    GT_PORT_NUM                 dstPortGet;
    GT_HW_DEV_NUM               dstHwDevGet;
    GT_BOOL                     dstTrunkGet;
    GT_BOOL                     enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices (devNum). */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For each active device (devNum) id go over all active virtual portNums. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /* 1.1.1. Call function with enable [GT_FALSE] and              */
            /* dstPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (should be ignored), */
            /* dstHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS] (should be ignored),    */
            /* dstTrunk [GT_TRUE and GT_FALSE].                             */
            /* Expected: GT_OK for Cheetah and GT_BAD_PARAM for others.     */

            enable = GT_FALSE;

            dstPort = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(devNum);
            dstHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);

            dstTrunk = GT_TRUE;

            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                                  "Cheetah/Cheetah2 device: %d, %d, %d, %d, %d, %d",
                                  devNum, portNum, enable, dstPort, dstHwDev, dstTrunk);

            /* Verifying values */
            st = cpssDxChBrgPrvEdgeVlanPortEnableGet(devNum, portNum, &enableGet,
                                                     &dstPortGet, &dstHwDevGet,
                                                     &dstTrunkGet);


            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgPrvEdgeVlanPortEnableGet: %d, %d", devNum, portNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d, %d", devNum, portNum);

            dstTrunk = GT_FALSE;
            dstPort = 0;
            dstHwDev = 0;

            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                         "Cheetah/Cheetah2 device: %d, %d, %d, %d, %d, %d",
                         devNum, portNum, enable, dstPort, dstHwDev, dstTrunk);

            /* Verifying values */
            st = cpssDxChBrgPrvEdgeVlanPortEnableGet(devNum, portNum, &enableGet,
                                                     &dstPortGet, &dstHwDevGet,
                                                     &dstTrunkGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgPrvEdgeVlanPortEnableGet: %d, %d", devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d, %d", devNum, portNum);

            enable = GT_TRUE;
            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                                  "Cheetah/Cheetah2 device: %d, %d, %d, %d, %d, %d",
                                  devNum, portNum, enable, dstPort, dstHwDev, dstTrunk);

            /* Verifying values */
            st = cpssDxChBrgPrvEdgeVlanPortEnableGet(devNum, portNum, &enableGet,
                                                     &dstPortGet, &dstHwDevGet,
                                                     &dstTrunkGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgPrvEdgeVlanPortEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(dstPort, dstPortGet,
            "get another dstPort value than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(dstHwDev, dstHwDevGet,
            "get another dstHwDev value than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(dstTrunk, dstTrunkGet,
            "get another dstTrunk value than was set: %d, %d", devNum, portNum);


            /* 1.1.2. Check for out of range dstHwDev. For Cheetah device call */
            /*        function with enable [GT_TRUE], dstTrunk [GT_FALSE],   */
            /*        dstHwDev [PRV_CPSS_MAX_PP_DEVICES_CNS], dstPort [0].     */
            /*        Expected: GT_BAD_PARAM.                                */
            enable = GT_TRUE;
            dstTrunk = GT_FALSE;
            dstHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
            dstPort = 0;

            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_BAD_PARAM, st,
                     "Cheetah/Cheetah2 device: %d, %d, %d, %d, %d, %d",
                     devNum, portNum, enable, dstPort, dstHwDev, dstTrunk);

            /* 1.1.3. Check for out of range dstPort. For Cheetah device call*/
            /*        function with enable [GT_TRUE], dstTrunk [GT_FALSE],   */
            /*        dstHwDev [0], dstPort [PRV_CPSS_MAX_PP_DEVICES_CNS].     */
            /*        Expected: GT_BAD_PARAM.                                */
            enable = GT_TRUE;
            dstTrunk = GT_FALSE;
            dstHwDev = 0;
            dstPort = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(devNum);

            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_BAD_PARAM, st,
                     "Cheetah/Cheetah2 device: %d, %d, %d, %d, %d, %d",
                     devNum, portNum, enable, dstPort, dstHwDev, dstTrunk);

            /* 1.1.4. Check for trunk-reference support. Call function with */
            /*    enable [GT_TRUE], dstTrunk [GT_TRUE] and dstPort [15],    */
            /*    destDev [PRV_CPSS_MAX_PP_DEVICES_CNS] (should be ignored).*/
            /*Expected: GT_OK for Cheetah dev and GT_BAD_PARAM for others.  */

            enable = GT_TRUE;
            dstTrunk = GT_TRUE;
            dstHwDev = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
            dstPort = 15;

            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                         "Cheetah/Cheetah2 device: %d, %d, %d, %d, %d, %d",
                         devNum, portNum, enable, dstPort, dstHwDev, dstTrunk);
            /* Verifying values */
            st = cpssDxChBrgPrvEdgeVlanPortEnableGet(devNum, portNum, &enableGet,
                                                     &dstPortGet, &dstHwDevGet,
                                                     &dstTrunkGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgPrvEdgeVlanPortEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(dstPort, dstPortGet,
            "get another dstPort value than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(dstTrunk, dstTrunkGet,
            "get another dstTrunk value than was set: %d, %d", devNum, portNum);

            /*1.1.5. Check for out of range trunk id while it's used.   */
            /*       For Cheetah device call function with dstTrunk     */
            /*       [GT_TRUE] and dstPort [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128]     */
            /*       (trunkId), destDev [0]. Expected: NON GT_OK.       */
            enable = GT_TRUE;
            dstTrunk = GT_TRUE;
            dstHwDev = 0;
            dstPort = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum);

            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_NOT_EQUAL6_STRING_MAC(GT_OK, st,
                                  "Cheetah/Cheetah2 device: %d, %d, %d, %d, %d, %d",
                                  devNum, portNum, enable, dstPort, dstHwDev, dstTrunk);
        }

        /* Only for Cheetah/Cheetah 2 test non-active/out-of-range VirtPort */
        enable = GT_FALSE;
        dstPort = 0;
        dstHwDev = 0;
        dstTrunk = GT_FALSE;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. Go over all non active virtual portNums and active device id (devNum). */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. <Call function for non active portNum and valid parameters>.     */
            /* Expected: GT_BAD_PARAM.                                                 */
            st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                                  dstPort, dstHwDev, dstTrunk);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. Call with out of range for virtual portNum.                       */
        /* Expected: GT_BAD_PARAM.                                                */

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        /* enable == GT_FALSE   */
        /* dstPort == 0         */
        /* dstHwDev == 0          */
        /* dstTrunk == GT_FALSE */

        st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                              dstPort, dstHwDev, dstTrunk);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        enable = GT_TRUE;

        st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                              dstPort, dstHwDev, dstTrunk);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

        /* Verifying values */
        st = cpssDxChBrgPrvEdgeVlanPortEnableGet(devNum, portNum, &enableGet,
                                                 &dstPortGet, &dstHwDevGet,
                                                 &dstTrunkGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    portNum = 0;

    enable = GT_FALSE;
    dstPort = 0;
    dstHwDev = 0;
    dstTrunk = GT_FALSE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices (devNum). */

    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* 2.1. <Call function for non active device (devNum) and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */

        st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                              dstPort, dstHwDev, dstTrunk);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of range device id (devNum).   */
    /* Expected: GT_BAD_PARAM.                                  */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_FALSE   */
    /* dstPort == 0         */
    /* dstHwDev == 0          */
    /* dstTrunk == GT_FALSE */
    /* portNum == 0         */

    st = cpssDxChBrgPrvEdgeVlanPortEnable(devNum, portNum, enable,
                                          dstPort, dstHwDev, dstTrunk);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       port,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh2 and above)
    1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call function cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet
           with non-NULL enablePtr.
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS                   st = GT_OK;

    GT_U8                       dev;
    GT_PORT_NUM                 port;
    GT_BOOL                     enable;
    GT_BOOL                     enableGet;

    /* 1. Go over all active devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&port,GT_TRUE))
        {
            /* 1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK. */
            /* 1.1.2. Call function cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet
            with non-NULL enablePtr. Expected: GT_OK and enable the same as was set. */

            /* 1.1.1. for enable == GT_FALSE*/
            enable = GT_FALSE;

            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. for enable == GT_FALSE*/
            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", dev, port);

            /* 1.1.1. for enable == GT_TRUE*/
            enable = GT_TRUE;

            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(dev, port, enable);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. for enable == GT_TRUE*/
            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", dev, port);
        }
        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_TRUE;
    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       port,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh2 and above)
    1.1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;

    GT_U8                       dev;
    GT_PORT_NUM                 port;
    GT_BOOL                     enableGet;

    /* 1. Go over all active devices. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&port,GT_TRUE))
        {
            /* 1.1.1. Call function with non-NULL enablePtr. Expected: GT_OK. */
            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call function with enablePtr [NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }


        /* 1.2. For all active devices go over all non available physical ports. */
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(dev, port, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgPrvEdgeVlan suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgPrvEdgeVlan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPrvEdgeVlanEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPrvEdgeVlanEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPrvEdgeVlanPortEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgPrvEdgeVlan)



