/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*  cpssDxChBrgSecurityBreach.c
*
* DESCRIPTION:
*       Unit tests for cpssDxChBrgSecurityBreach, that provides
*       CPSS DxCh implementation for Bridge Security Breach Functionality
*
*
* FILE REVISION NUMBER:
*       $Revision: 24 $
*
*******************************************************************************/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachPortVlanDropCntrModeSet
(
    IN GT_U8                                        dev,
    IN CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachPortVlanDropCntrModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL cntrCfgPtr
    [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E; port = 0;
    vlan = PRV_CPSS_MAX_NUM_VLANS_CNS = 4096  (should be ignored)}].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSecurBreachPortVlanDropCntrModeGet.
    Expected: GT_OK and the same cntrCfgPtr.
    1.1. Call function with non-NULL cntrCfgPtr
    [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E; port = UTF_CPSS_PP_MAX_PORTS_NUM_CNS
    (should be ignored); vlan = 100}].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSecurBreachPortVlanDropCntrModeGet.
    Expected: GT_OK and the same cntrCfgPtr.
    1.3. Call function with cntrCfgPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call function with cntrCfgPtr [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;
    port = UTF_CPSS_PP_MAX_PORTS_NUM_CNS; vlan = 100}].
    Expected: NON GT_OK.
    1.5. Call function with cntrCfgPtr [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E;
    port = 0; vlan = PRV_CPSS_MAX_NUM_VLANS_CNS = 4096}].
    Expected: NON GT_OK.
    1.6. Call function with cntrCfgPtr [{ out of range dropCntMode ;
    port = 0; vlan = 100}].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8                                       dev;
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC    cntrCfg;
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC    retCntrCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL cntrCfgPtr
        [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E; port = 0;
        vlan = PRV_CPSS_MAX_NUM_VLANS_CNS = 4096  (should be ignored)}].
        Expected: GT_OK.
        1.2. Call cpssDxChBrgSecurBreachPortVlanDropCntrModeGet.
        Expected: GT_OK and the same cntrCfgPtr. */
        cntrCfg.dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;
        cntrCfg.port = 0;
        cntrCfg.vlan = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(dev, &cntrCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(dev, &retCntrCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChBrgSecurBreachPortVlanDropCntrModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cntrCfg.dropCntMode,
                                     retCntrCfg.dropCntMode,
                        "got another cntrCfg.dropCntMode than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cntrCfg.port, retCntrCfg.port,
                        "got another cntrCfg.port than was set: %d", dev);

        /* 1.1. Call function with non-NULL cntrCfgPtr
        [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E; port = UTF_CPSS_PP_MAX_PORTS_NUM_CNS
        (should be ignored); vlan = 100}]. Expected: GT_OK.
        1.2. Call cpssDxChBrgSecurBreachPortVlanDropCntrModeGet.
        Expected: GT_OK and the same cntrCfgPtr. */
        cntrCfg.dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E;
        cntrCfg.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        cntrCfg.vlan = 100;

        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(dev, &cntrCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(dev, &retCntrCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSecurBreachPortVlanDropCntrModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cntrCfg.dropCntMode, retCntrCfg.dropCntMode,
                     "got another cntrCfg.dropCntMode than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cntrCfg.vlan, retCntrCfg.vlan,
                            "got another cntrCfg.vlan than was set: %d", dev);

        /* 1.3. Call function with cntrCfgPtr [NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /* 1.4. Call function with cntrCfgPtr [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;
        port = UTF_CPSS_PP_MAX_PORTS_NUM_CNS; vlan = 100}]. Expected: NON GT_OK. */
        cntrCfg.dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;
        cntrCfg.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        cntrCfg.vlan = 100;

        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(dev, &cntrCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.5. Call function with cntrCfgPtr [{CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E;
        port = 0; vlan = PRV_CPSS_MAX_NUM_VLANS_CNS = 4096}]. Expected: NON GT_OK. */
        cntrCfg.dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E;
        cntrCfg.port = 0;
        cntrCfg.vlan = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(dev, &cntrCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.6. Call function w4th cntrCfgPtr [{ wrong enum values dropCntMode ;
                                                port = 0; vlan = 100}].
            Expected: GT_BAD_PARAM.
        */
        cntrCfg.port = 0;
        cntrCfg.vlan = 100;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSecurBreachPortVlanDropCntrModeSet
                            (dev, &cntrCfg),
                            cntrCfg.dropCntMode);
    }

    cntrCfg.dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;
    cntrCfg.port = 0;
    cntrCfg.vlan = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st =cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(dev, &cntrCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(dev, &cntrCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachPortVlanDropCntrModeGet
(
    IN  GT_U8                                        dev,
    OUT CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachPortVlanDropCntrModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL cntrCfgPtr.
    Expected: GT_OK.
    1.2. Call function with cntrCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC    retCntrCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL cntrCfgPtr. Expected: GT_OK. */
        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(dev, &retCntrCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. . Call function with cntrCfgPtr [NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(dev, &retCntrCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(dev, &retCntrCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachGlobalDropCntrGet
(
    IN  GT_U8    dev,
    OUT GT_U32   *counValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachGlobalDropCntrGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL counValuePtr.
    Expected: GT_OK.
    1.2. Call function with counValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_U32          retCounValue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL cntrCfgPtr. Expected: GT_OK. */
        st = cpssDxChBrgSecurBreachGlobalDropCntrGet(dev, &retCounValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with counValuePtr [NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChBrgSecurBreachGlobalDropCntrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachGlobalDropCntrGet(dev, &retCounValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachGlobalDropCntrGet(dev, &retCounValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachPortVlanCntrGet
(
    IN  GT_U8    dev,
    OUT GT_U32   *counValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachPortVlanCntrGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL counValuePtr.
    Expected: GT_OK.
    1.2. Call function with counValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_U32          retCounValue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL counValuePtr. Expected: GT_OK. */
        st = cpssDxChBrgSecurBreachPortVlanCntrGet(dev, &retCounValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with counValuePtr [NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChBrgSecurBreachPortVlanCntrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachPortVlanCntrGet(dev, &retCounValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachPortVlanCntrGet(dev, &retCounValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachNaPerPortSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachNaPerPortSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1. Call function with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSecurBreachNaPerPortGet with non-NULL pointer.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_PORT_NUM     port;
    GT_BOOL         enable;
    GT_BOOL         retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with enable [GT_TRUE / GT_FALSE]. Expected: GT_OK. */
            /* 1.2. Call cpssDxChBrgSecurBreachNaPerPortGet with non-NULL
            pointer. Expected: GT_OK and the same enable. */
            enable = GT_TRUE;

            st = cpssDxChBrgSecurBreachNaPerPortSet(dev,port,enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSecurBreachNaPerPortGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "got another enable than was set: &d, %d", dev, port);

            enable = GT_FALSE;

            st = cpssDxChBrgSecurBreachNaPerPortSet(dev,port,enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSecurBreachNaPerPortGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "got another enable than was set: &d, %d", dev, port);
        }
        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSecurBreachNaPerPortSet(dev,port,enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSecurBreachNaPerPortSet(dev,port,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSecurBreachNaPerPortSet(dev,port,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable  = GT_TRUE;
    port    = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachNaPerPortSet(dev,port,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachNaPerPortSet(dev,port,enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachNaPerPortGet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    OUT GT_BOOL             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachNaPerPortGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_PORT_NUM     port;
    GT_BOOL         retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with non-NULL enablePtr. Expected: GT_OK. */
            st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. Call function with enablePtr [NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachNaPerPortGet(dev, port, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachMovedStaticAddrSet
(
    IN GT_U8                dev,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachMovedStaticAddrSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSecurBreachMovedStaticAddrGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_BOOL         enable;
    GT_BOOL         retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgSecurBreachMovedStaticAddrSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgSecurBreachMovedStaticAddrGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgSecurBreachMovedStaticAddrGet(dev, &retEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSecurBreachMovedStaticAddrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, retEnable,
                    "got another enable than was set: %d", dev);

        /*
            1.1. Call function with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgSecurBreachMovedStaticAddrSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgSecurBreachMovedStaticAddrGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgSecurBreachMovedStaticAddrGet(dev, &retEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSecurBreachMovedStaticAddrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, retEnable,
                "got another enable than was set: %d", dev);
    }
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachMovedStaticAddrSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachMovedStaticAddrSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachMovedStaticAddrGet
(
    IN  GT_U8                dev,
    OUT GT_BOOL              *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachMovedStaticAddrGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_BOOL         retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr. Expected: GT_OK. */
        st = cpssDxChBrgSecurBreachMovedStaticAddrGet(dev, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with enablePtr [NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChBrgSecurBreachMovedStaticAddrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachMovedStaticAddrGet(dev, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachMovedStaticAddrGet(dev, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachEventDropModeSet
(
    IN GT_U8                                dev,
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_DROP_MODE_TYPE_ENT              dropMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachEventDropModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                         and dropMode [CPSS_DROP_MODE_SOFT_E /
                                       CPSS_DROP_MODE_HARD_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSecurBreachEventDropModeGet with eventType the same as in 1.1.
         (call once for each) and non NULL pointer .
    Expected: GT_OK and the same dropMode.
    1.3. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                         and dropMode [CPSS_DROP_MODE_SOFT_E /
                                       CPSS_DROP_MODE_HARD_E].
    Expected: NON GT_OK.
    1.4. Call function with out of range eventType
         and and dropMode [CPSS_DROP_MODE_SOFT_E].
    Expected: GT_BAD_PARAM.
    1.5. Call function with with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E]
         and out of range dropMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;

    GT_U8                               dev;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_DROP_MODE_TYPE_ENT             dropMode;
    CPSS_DROP_MODE_TYPE_ENT             retDropMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E;
        dropMode = CPSS_DROP_MODE_SOFT_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        /*
            1.2. Call cpssDxChBrgSecurBreachEventDropModeGet with eventType the same as in 1.1.
                 (call once for each) and non NULL pointer .
            Expected: GT_OK and the same dropMode.
        */
        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSecurBreachEventDropModeGet: %d, %d", dev, eventType);
        UTF_VERIFY_EQUAL2_STRING_MAC(dropMode, retDropMode,
                "got another dropMode than was set: %d, %d", dev, eventType);

        /*
            1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E;
        dropMode = CPSS_DROP_MODE_HARD_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        /*
            1.2. Call cpssDxChBrgSecurBreachEventDropModeGet with eventType the same as in 1.1.
                 (call once for each) and non NULL pointer .
            Expected: GT_OK and the same dropMode.
        */
        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSecurBreachEventDropModeGet: %d, %d", dev, eventType);
        UTF_VERIFY_EQUAL2_STRING_MAC(dropMode, retDropMode,
                "got another dropMode than was set: %d, %d", dev, eventType);

        /*
            1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E;
        dropMode = CPSS_DROP_MODE_SOFT_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        /*
            1.2. Call cpssDxChBrgSecurBreachEventDropModeGet with eventType the same as in 1.1.
                 (call once for each) and non NULL pointer .
            Expected: GT_OK and the same dropMode.
        */
        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSecurBreachEventDropModeGet: %d, %d", dev, eventType);
        UTF_VERIFY_EQUAL2_STRING_MAC(dropMode, retDropMode,
                "got another dropMode than was set: %d, %d", dev, eventType);

        /*
            1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E;
        dropMode = CPSS_DROP_MODE_HARD_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        /*
            1.2. Call cpssDxChBrgSecurBreachEventDropModeGet with eventType the same as in 1.1.
                 (call once for each) and non NULL pointer .
            Expected: GT_OK and the same dropMode.
        */
        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSecurBreachEventDropModeGet: %d, %d", dev, eventType);
        UTF_VERIFY_EQUAL2_STRING_MAC(dropMode, retDropMode,
                "got another dropMode than was set: %d, %d", dev, eventType);

        /*
            1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E;
        dropMode = CPSS_DROP_MODE_HARD_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        /*
            1.2. Call cpssDxChBrgSecurBreachEventDropModeGet with eventType the same as in 1.1.
                 (call once for each) and non NULL pointer .
            Expected: GT_OK and the same dropMode.
        */
        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSecurBreachEventDropModeGet: %d, %d", dev, eventType);
        UTF_VERIFY_EQUAL2_STRING_MAC(dropMode, retDropMode,
                "got another dropMode than was set: %d, %d", dev, eventType);

        /*
            1.3. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: NON GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E;
        dropMode = CPSS_DROP_MODE_SOFT_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        /*
            1.3. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: NON GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E;
        dropMode = CPSS_DROP_MODE_HARD_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        /*
            1.3. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                                 and dropMode [CPSS_DROP_MODE_SOFT_E /
                                               CPSS_DROP_MODE_HARD_E].
            Expected: NON GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E;
        dropMode = CPSS_DROP_MODE_SOFT_E;

        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, dropMode);

        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E;
        dropMode = CPSS_DROP_MODE_HARD_E;

        /*
            1.4. Call function with out of range eventType
                 and and dropMode [CPSS_DROP_MODE_SOFT_E].
            Expected: GT_BAD_PARAM.
        */
        dropMode = CPSS_DROP_MODE_SOFT_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSecurBreachEventDropModeSet
                            (dev, eventType, dropMode),
                            eventType);

        /*
            1.5. Call function with out of range dropMode .
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSecurBreachEventDropModeSet
                            (dev, eventType, dropMode),
                            dropMode);
    }

    dropMode = CPSS_DROP_MODE_SOFT_E;
    eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachEventDropModeSet(dev, eventType, dropMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSecurBreachEventDropModeGet
(
    IN  GT_U8                               dev,
    IN  CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachEventDropModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
         and non-NULL dropModePtr.
    Expected: GT_OK.
    1.2. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E /
                                       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
         and non-NULL dropModePtr.
    Expected: NON GT_OK.
    1.3. Call function with out of range eventType and non-NULL dropModePtr.
    Expected: GT_BAD_PARAM.
    1.4. Call function with with eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E]
         and  dropModePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                           st = GT_OK;

    GT_U8                               dev;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_DROP_MODE_TYPE_ENT             retDropMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                 and non-NULL dropModePtr.
            Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        /*
            1.2. Call function with eventType [CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E /
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E]
                 and non-NULL dropModePtr.
            Expected: NON GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E;

        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, eventType);

        /*
            1.3. Call function with wrong enum values eventType and non-NULL dropModePtr.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSecurBreachEventDropModeGet
                            (dev, eventType, &retDropMode),
                            eventType);

        /*
            1.4. Call function with with eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E]
                 and  dropModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, eventType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachEventDropModeGet(dev, eventType, &retDropMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChSecurBreachMsgGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_BRG_SECUR_BREACH_MSG_STC   *sbMsgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChSecurBreachMsgGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL sbMsgPtr.
    Expected: GT_OK.
    1.2. Call function with sbMsgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    CPSS_BRG_SECUR_BREACH_MSG_STC   retSbMsg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL sbMsgPtr. Expected: GT_OK. */
        st = cpssDxChSecurBreachMsgGet(dev, &retSbMsg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with sbMsgPtr [NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChSecurBreachMsgGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChSecurBreachMsgGet(dev, &retSbMsg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChSecurBreachMsgGet(dev, &retSbMsg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgSecurBreachEventPacketCommandGet
(
    IN GT_U8                                dev,
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachEventPacketCommandGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with
        eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                  CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                  CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E
                  CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
                  CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
                  CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
                  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)                  
                  ],
        and non-NULL commandPtr.
    Expected: GT_OK.
    1.2. Call with out of range enum value eventType
        and other parameters the same as 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call function with out of range commandPtr [NULL] and
        other values same as 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_PACKET_CMD_ENT                 command;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E,
                and non-NULL commandPtr.
            Expected: GT_OK.
         */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E,
                and non-NULL commandPtr.
            Expected: GT_OK.
         */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E,
                and non-NULL commandPtr.
            Expected: GT_OK.
         */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
                          
                and non-NULL commandPtr.
            Expected: GT_OK.
         */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
                and non-NULL commandPtr.
            Expected: GT_OK.
         */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                          CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
                and non-NULL commandPtr.
            Expected: GT_OK.
         */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
            1.2. Call with out of range enum value eventType
                and other parameters the same as 1.1.
            Expected: GT_BAD_PARAM.
       */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSecurBreachEventPacketCommandGet
                            (dev, eventType, &command),
                            eventType);

        /*
            1.3. Call function with out of range commandPtr [NULL] and
                other values same as 1.1.
            Expected: GT_BAD_PTR.
       */
        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* restore valid value*/
    eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgSecurBreachEventPacketCommandSet
(
    IN GT_U8                                dev,
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_PACKET_CMD_ENT                  command
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachEventPacketCommandSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with
        eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
                  CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
                  CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                  CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
                  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
                  CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
                  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
                  CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
                  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
                  ]
	   	    
        command[CPSS_PACKET_CMD_FORWARD_E,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                CPSS_PACKET_CMD_DROP_SOFT_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSecurBreachEventPacketCommandGet.
    Expected: GT_OK and the same values that was set.
    1.3. Call with out of range enum value eventType
        and other parameters the same as 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range enum value command
        and other parameters the same as 1.1.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_PACKET_CMD_ENT                 command;
    CPSS_PACKET_CMD_ENT                 commandRet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
				 ]
                command[CPSS_PACKET_CMD_FORWARD_E,
                        CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                        CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, command);
            /*
                1.2. Call cpssDxChBrgSecurBreachEventPacketCommandGet.
                Expected: GT_OK and the same values that was set.
            */

            st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &commandRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandRet,
                                         "got another value: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, eventType, command);
        }


        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
				 ]
                command[CPSS_PACKET_CMD_FORWARD_E,
                        CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                        CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, command);
            /*
                1.2. Call cpssDxChBrgSecurBreachEventPacketCommandGet.
                Expected: GT_OK and the same values that was set.
            */

            st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &commandRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandRet,
                                         "got another value: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, eventType, command);
        }


        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
				 ]
                command[CPSS_PACKET_CMD_FORWARD_E,
                        CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                        CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E;
        command = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2. Call cpssDxChBrgSecurBreachEventPacketCommandGet.
            Expected: GT_OK and the same values that was set.
        */

        st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &commandRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandRet,
                                     "got another value: %d", dev);


        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
				 ]
                command[CPSS_PACKET_CMD_FORWARD_E,
                        CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                        CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, command);
            /*
                1.2. Call cpssDxChBrgSecurBreachEventPacketCommandGet.
                Expected: GT_OK and the same values that was set.
            */

            st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &commandRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandRet,
                                         "got another value: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, eventType, command);
        }

        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
				 ]
                command[CPSS_PACKET_CMD_FORWARD_E,
                        CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                        CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
        */
        eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E;
        command = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, command);
            /*
                1.2. Call cpssDxChBrgSecurBreachEventPacketCommandGet.
                Expected: GT_OK and the same values that was set.
            */

            st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &commandRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandRet,
                                         "got another value: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, eventType, command);
        }



        /*
            1.1. Call function with
                eventType[CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
				 CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E
				 ]
                command[CPSS_PACKET_CMD_FORWARD_E,
                        CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                        CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
        */
				eventType = CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E;
				command = CPSS_PACKET_CMD_DROP_SOFT_E;
		
				st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
				if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
				{
					UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, eventType, command);
					/*
						1.2. Call cpssDxChBrgSecurBreachEventPacketCommandGet.
						Expected: GT_OK and the same values that was set.
					*/
		
					st = cpssDxChBrgSecurBreachEventPacketCommandGet(dev, eventType, &commandRet);
					UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
		
					/* verifying values */
					UTF_VERIFY_EQUAL1_STRING_MAC(command, commandRet,
												 "got another value: %d", dev);
				}
				else
				{
					UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, eventType, command);
				}


        /*
            1.3. Call with out of range enum value eventType
                and other parameters the same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSecurBreachEventPacketCommandSet
                            (dev, eventType, command),
                            eventType);

        /*
            1.4. Call with out of range enum value command
                and other parameters the same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSecurBreachEventPacketCommandSet
                            (dev, eventType, command),
                            command);

    }

    /* restore valid values */
    eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E;
    command = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSecurBreachEventPacketCommandSet(dev, eventType, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

#ifdef ASIC_SIMULATION

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet
(
    IN  GT_U8                   dev,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *counValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet)
{
    GT_U32 regAddr;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_LION2_E, "JIRA-6760");

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet())->bridgeRegs.bridgeGlobalSecurBreachDropCounter;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x02040104;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
              cpssDxChBrgSecurBreachGlobalDropCntrGet,
              NULL,
              cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet,
              NULL);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet
(
    IN  GT_U8                   dev,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *counValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet)
{
    GT_U32 regAddr;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_LION2_E, "JIRA-6761");

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet())->bridgeRegs.bridgePortVlanSecurBreachDropCounter;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x02040108;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
              cpssDxChBrgSecurBreachPortVlanCntrGet,
              NULL,
              cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet,
              NULL);
}
#endif /* ASIC_SIMULATION*/
/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgSecurityBreach suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgSecurityBreach)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachPortVlanDropCntrModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachPortVlanDropCntrModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachGlobalDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachPortVlanCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachNaPerPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachNaPerPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachMovedStaticAddrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachMovedStaticAddrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachEventDropModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachEventDropModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChSecurBreachMsgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachEventPacketCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachEventPacketCommandSet)

#ifdef ASIC_SIMULATION
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet)
#endif /* ASIC_SIMULATION*/
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgSecurityBreach)

