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
* @file cpssDxChBrgNestVlanUT.c
*
* @brief Unit tests for cpssDxChBrgNestVlan, that provides
* Nested VLANs CPSS DxCh facility implementation.
*
* @version   19
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
#define BRG_NEST_VLAN_VALID_PHY_PORT_CNS    0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgNestVlanAccessPortSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgNestVlanAccessPortSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgNestVlanAccessPortGet
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             port;
    GT_BOOL                 enable;
    GT_BOOL                 retEnable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with enable [GT_FALSE and GT_TRUE]. Expected: GT_OK. */
            /* 1.2. Call cpssDxChBrgNestVlanAccessPortGet.
            Expected: GT_OK and the same enable. */

            /*1.1. enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChBrgNestVlanAccessPortSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2. enable = GT_FALSE */
            st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgNestVlanAccessPortGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable than was set: %d, %d", dev, port);

            /*1.1. enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChBrgNestVlanAccessPortSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*1.2. enable = GT_TRUE */
            st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
               "cpssDxChBrgNestVlanAccessPortGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgNestVlanAccessPortSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgNestVlanAccessPortSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgNestVlanAccessPortSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_NEST_VLAN_VALID_PHY_PORT_CNS;
    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgNestVlanAccessPortSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgNestVlanAccessPortSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgNestVlanAccessPortGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *enablePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChBrgNestVlanAccessPortGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL] .
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port;
    GT_BOOL     retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function with non-NULL retEnable.
            Expected: GT_OK. */

            st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, retEnable);

            /* 1.1.2. Call function with retEnable[NULL].
            Expected: GT_BAD_PTR. */

            st = cpssDxChBrgNestVlanAccessPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_NEST_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgNestVlanAccessPortGet(dev, port, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgNestVlanEtherTypeSelectSet
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   portNum,
    IN  CPSS_ETHER_MODE_ENT     ingressVlanSel
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgNestVlanEtherTypeSelectSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh1, DxCh2 and DxCh3)
    1.1. Call function with ingressVlanSel
         [CPSS_VLAN_ETHERTYPE0_E/ CPSS_VLAN_ETHERTYPE1_E].
    Expected: GT_OK.
    1.2. Call function cpssDxChBrgNestVlanEtherTypeSelectGet.
    Expected: GT_OK and the same ingressVlanSel
    1.3. Call function with wrong enum values ingressVlanSel .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             port;
    CPSS_ETHER_MODE_ENT     ingressVlanSel;
    CPSS_ETHER_MODE_ENT     retIngressVlanSel;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with ingressVlanSel [CPSS_VLAN_ETHERTYPE0_E
            / CPSS_VLAN_ETHERTYPE1_E]. Expected: GT_OK. */
            /* 1.2. Call cpssDxChBrgNestVlanEtherTypeSelectGet.
            Expected: GT_OK and the same ingressVlanSel. */

            /* 1.1. ingressVlanSel = CPSS_VLAN_ETHERTYPE0_E */
            ingressVlanSel = CPSS_VLAN_ETHERTYPE0_E;

            st = cpssDxChBrgNestVlanEtherTypeSelectSet(dev, port, ingressVlanSel);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ingressVlanSel);

            /* 1.2. ingressVlanSel = CPSS_VLAN_ETHERTYPE0_E */
            st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgNestVlanEtherTypeSelectGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(ingressVlanSel, retIngressVlanSel,
                "get another ingressVlanSel than was set: %d, %d", dev, port);

            /* 1.1.ingressVlanSel = CPSS_VLAN_ETHERTYPE1_E  */
            ingressVlanSel = CPSS_VLAN_ETHERTYPE1_E;

            st = cpssDxChBrgNestVlanEtherTypeSelectSet(dev, port, ingressVlanSel);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ingressVlanSel);

            /*1.2. ingressVlanSel = CPSS_VLAN_ETHERTYPE1_E */
            st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgNestVlanEtherTypeSelectGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(ingressVlanSel, retIngressVlanSel,
                "get another ingressVlanSel than was set: %d, %d", dev, port);

            /*
                1.3. Call function with wrong enum values ingressVlanSel
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgNestVlanEtherTypeSelectSet
                            (dev, port, ingressVlanSel),
                            ingressVlanSel);
        }

        ingressVlanSel = CPSS_VLAN_ETHERTYPE1_E;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgNestVlanEtherTypeSelectSet(dev, port, ingressVlanSel);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgNestVlanEtherTypeSelectSet(dev, port, ingressVlanSel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgNestVlanEtherTypeSelectSet(dev, port, ingressVlanSel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_NEST_VLAN_VALID_PHY_PORT_CNS;
    ingressVlanSel = CPSS_VLAN_ETHERTYPE1_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgNestVlanEtherTypeSelectSet(dev, port, ingressVlanSel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgNestVlanEtherTypeSelectSet(dev, port, ingressVlanSel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgNestVlanEtherTypeSelectGet
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   portNum,
    OUT CPSS_ETHER_MODE_ENT     *ingressVlanSelPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgNestVlanEtherTypeSelectGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh1, DxCh2 and DxCh3)
    1.1. Call function with non-null ingressVlanSelPtr.
    Expected: GT_OK.
    1.2. Call function with null ingressVlanSelPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port;
    CPSS_ETHER_MODE_ENT retIngressVlanSel;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with non-NULL retIngressvlanSel
            Expected: GT_OK. */

            st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, retIngressVlanSel);

            /* 1.2. Call function with retIngressVlanSel[NULL].
            Expected: GT_BAD_PTR. */

            st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_NEST_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgNestVlanEtherTypeSelectGet(dev, port, &retIngressVlanSel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgNestVlan suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgNestVlan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgNestVlanAccessPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgNestVlanAccessPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgNestVlanEtherTypeSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgNestVlanEtherTypeSelectGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgNestVlan)

