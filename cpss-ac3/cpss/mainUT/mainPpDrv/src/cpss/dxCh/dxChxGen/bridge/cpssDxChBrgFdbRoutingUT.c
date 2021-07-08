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
* @file cpssDxChBrgFdbRoutingUT.c
*
* @brief Unit tests for cpssDxChBrgFdbRouting, that provides
* FDB Host Routing CPSS DxCh implementation.
*
* @version   843
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>

#include <stdlib.h>

/* Defines */
/* #define UT_FDB_DEBUG */

#ifdef UT_FDB_DEBUG
#define UT_FDB_DUMP(_x) cpssOsPrintf _x
#else
#define UT_FDB_DUMP(_x)
#endif


/* Default valid value for port id */
#define BRG_FDB_VALID_PHY_PORT_CNS  0


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingPortIpUcEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    IN  GT_BOOL                      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingPortIpUcEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with enable [GT_FALSE / GT_TRUE]  and
                              protocol [CPSS_IP_PROTOCOL_IPV4_E /
                                        CPSS_IP_PROTOCOL_IPV6_E /
                                        CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgFdbRoutingPortIpUcEnableGet with
                    protocol [CPSS_IP_PROTOCOL_IPV4_E] and  not NULL enablePtr
        Expected: NOT GT_OK.
    1.1.3. Call function Call function with out of range protocol
    Expected: GT_BAD_PARAM

*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;
        CPSS_IP_PROTOCOL_STACK_ENT   protocol = CPSS_IP_PROTOCOL_IPV4_E;

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
                1.1.1. Call function with enable [GT_FALSE / GT_TRUE]  and
                                          protocol [CPSS_IP_PROTOCOL_IPV4_E /
                                                    CPSS_IP_PROTOCOL_IPV6_E].
                Expected: GT_OK.
            */
            /*
                Call with enable [GT_FALSE] and
                         protocol [CPSS_IP_PROTOCOL_IPV4_E]
            */
            enable = GT_FALSE;
            protocol = CPSS_IP_PROTOCOL_IPV4_E;

            st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol,enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* Verify values */
            st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol,&enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutingPortIpUcEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);
            /*
                Call with enable [GT_TRUE] and
                         protocol [CPSS_IP_PROTOCOL_IPV6_E]
            */
            enable = GT_TRUE;
            protocol = CPSS_IP_PROTOCOL_IPV6_E;

            st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol,enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* Verify values */
            st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol,&enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutingPortIpUcEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);
            /*
                Call with enable [GT_TRUE] and
                         protocol [CPSS_IP_PROTOCOL_IPV4V6_E]
            */
            enable = GT_TRUE;
            protocol = CPSS_IP_PROTOCOL_IPV4V6_E;

            st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol,enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgFdbRoutingPortIpUcEnableGet with
                       not NULL enablePtr
                Expected: GT_OK
            */
            protocol = CPSS_IP_PROTOCOL_IPV4_E;

            st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol,&enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutingPortIpUcEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);

            /*
                1.1.3. Call function Call function with out of range protocol
                Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbRoutingPortIpUcEnableSet
                                (dev, port, protocol, enable),
                                protocol);
        }

        /* set correct values*/
        enable = GT_FALSE;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol,enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }
    /* set correct values*/
    enable   = GT_FALSE;
    protocol = CPSS_IP_PROTOCOL_IPV4_E;
    port     = BRG_FDB_VALID_PHY_PORT_CNS;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingPortIpUcEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    OUT GT_BOOL                      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingPortIpUcEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with protocol [CPSS_IP_PROTOCOL_IPV4_E /
                                        CPSS_IP_PROTOCOL_IPV6_E] and
                              not NULL enablePtr
    Expected: GT_OK.
        1.1.2. Call function with not NULL enablePtr and out of range protocol
        Expected: NOT GT_OK.
    1.1.3. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;
        CPSS_IP_PROTOCOL_STACK_ENT   protocol = CPSS_IP_PROTOCOL_IPV4_E;


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
                1.1.1. Call function with protocol [CPSS_IP_PROTOCOL_IPV4_E /
                                                    CPSS_IP_PROTOCOL_IPV6_E] and
                                          not NULL enablePtr
                Expected: GT_OK.
            */
            /* Call with protocol [CPSS_IP_PROTOCOL_IPV4_E] */
            protocol = CPSS_IP_PROTOCOL_IPV4_E;
            st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Call with protocol [CPSS_IP_PROTOCOL_IPV6_E] */
            protocol = CPSS_IP_PROTOCOL_IPV6_E;
            st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call function Call function with out of range protocol
                Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbRoutingPortIpUcEnableGet
                                (dev, port, protocol, &enable),
                                protocol);

            /*
                1.1.3. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, enablePtr = NULL", dev, port);
        }
        /* set correct values*/
            protocol = CPSS_IP_PROTOCOL_IPV4_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }
    /* set correct values*/
    enable   = GT_FALSE;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
    port     = BRG_FDB_VALID_PHY_PORT_CNS;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcRefreshEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcRefreshEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1 Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2 Call cpssDxChBrgFdbRoutingUcRefreshEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with enable [GT_FALSE]
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbRoutingUcRefreshEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcRefreshEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcRefreshEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
        /*
            1.1.1. Call function with enable [GT_TRUE]
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgFdbRoutingUcRefreshEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcRefreshEnableGet
            Expected: GT_OK and the same enablePtr.
        */

        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcRefreshEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
    }
    /* set correct values*/
    enable   = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcRefreshEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcRefreshEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcRefreshEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcRefreshEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                        "%d, enablePtr = NULL", dev);
     }

    /* set correct values*/
    enable   = GT_FALSE;
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcAgingEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcAgingEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1 Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2 Call cpssDxChBrgFdbRoutingUcAgingEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with enable [GT_FALSE]
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbRoutingUcAgingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcAgingEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcAgingEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcAgingEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
        /*
            1.1.1. Call function with enable [GT_TRUE]
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgFdbRoutingUcAgingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcAgingEnableGet
            Expected: GT_OK and the same enablePtr.
        */

        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcAgingEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcAgingEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
    }
    /* set correct values*/
    enable   = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcAgingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcAgingEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcAgingEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcAgingEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbRoutingUcAgingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbRoutingUcAgingEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                        "%d, enablePtr = NULL", dev);
     }

    /* set correct values*/
    enable   = GT_FALSE;
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcAgingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcAgingEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcTransplantEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcTransplantEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1 Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2 Call cpssDxChBrgFdbRoutingUcTransplantEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with enable [GT_FALSE]
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbRoutingUcTransplantEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcTransplantEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcTransplantEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcTransplantEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
        /*
            1.1.1. Call function with enable [GT_TRUE]
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgFdbRoutingUcTransplantEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcTransplantEnableGet
            Expected: GT_OK and the same enablePtr.
        */

        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcTransplantEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcTransplantEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
    }
    /* set correct values*/
    enable   = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcTransplantEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcTransplantEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcTransplantEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcTransplantEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbRoutingUcTransplantEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbRoutingUcTransplantEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                        "%d, enablePtr = NULL", dev);
     }

    /* set correct values*/
    enable   = GT_FALSE;
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcTransplantEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcTransplantEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcDeleteEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcDeleteEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1 Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2 Call cpssDxChBrgFdbRoutingUcDeleteEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with enable [GT_FALSE]
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbRoutingUcDeleteEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcDeleteEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcDeleteEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcDeleteEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
        /*
            1.1.1. Call function with enable [GT_TRUE]
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgFdbRoutingUcDeleteEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcDeleteEnableGet
            Expected: GT_OK and the same enablePtr.
        */

        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcDeleteEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcDeleteEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
    }
    /* set correct values*/
    enable   = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcDeleteEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcDeleteEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcDeleteEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcDeleteEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbRoutingUcDeleteEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbRoutingUcDeleteEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                        "%d, enablePtr = NULL", dev);
     }

    /* set correct values*/
    enable   = GT_FALSE;
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcDeleteEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcDeleteEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcAAandTAToCpuSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcAAandTAToCpuSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1 Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2 Call cpssDxChBrgFdbRoutingUcAAandTAToCpuGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with enable [GT_FALSE]
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcAAandTAToCpuGet
            Expected: GT_OK and the same enablePtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcAAandTAToCpuGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
        /*
            1.1.1. Call function with enable [GT_TRUE]
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingUcAAandTAToCpuGet
            Expected: GT_OK and the same enablePtr.
        */

        /* Verify values */
        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingUcAAandTAToCpuGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "got another enable than was set: %d", dev);
    }
    /* set correct values*/
    enable   = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingUcAAandTAToCpuGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingUcAAandTAToCpuGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                        "%d, enablePtr = NULL", dev);
     }

    /* set correct values*/
    enable   = GT_FALSE;
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingNextHopPacketCmdSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PACKET_CMD_ENT         nhPacketCmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingNextHopPacketCmdSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1 Call function with nhPacketCmd [CPSS_PACKET_CMD_ROUTE_E,
                                         CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                         CPSS_PACKET_CMD_DROP_SOFT_E ,
                                         CPSS_PACKET_CMD_DROP_HARD_E]
    Expected: GT_OK.
    1.1.2 Call cpssDxChBrgFdbRoutingNextHopPacketCmdGet
    Expected: GT_OK and the same nhPacketCmdPtr.

    1.1.3 Call function with nhPacketCmd [CPSS_PACKET_CMD_FORWARD_E
                                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E
                                          CPSS_PACKET_CMD_NONE_E
                                          CPSS_PACKET_CMD_LOOPBACK_E,
                                          CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
                                          CPSS_PACKET_CMD_BRIDGE_E]
    Expected: GT_BAD_PARAM.

    1.1.4 Call function with wrong enum values mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    CPSS_PACKET_CMD_ENT        nhPacketCmd    = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT        nhPacketCmdGet = CPSS_PACKET_CMD_FORWARD_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with nhPacketCmd [CPSS_PACKET_CMD_ROUTE_E]
            Expected: GT_OK.
        */
        nhPacketCmd    = CPSS_PACKET_CMD_ROUTE_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, nhPacketCmd);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingNextHopPacketCmdGet
            Expected: GT_OK and the same nhPacketCmdPtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingNextHopPacketCmdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(nhPacketCmd, nhPacketCmdGet,
                      "got another nhPacketCmd than was set: %d", dev);
        /*
            1.1.1. Call function with nhPacketCmd [CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E]
            Expected: GT_OK.
        */
        nhPacketCmd    = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, nhPacketCmd);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingNextHopPacketCmdGet
            Expected: GT_OK and the same nhPacketCmdPtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingNextHopPacketCmdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(nhPacketCmd, nhPacketCmdGet,
                      "got another nhPacketCmd than was set: %d", dev);

        /*
            1.1.1. Call function with nhPacketCmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E]
            Expected: GT_OK.
        */
        nhPacketCmd    = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, nhPacketCmd);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingNextHopPacketCmdGet
            Expected: GT_OK and the same nhPacketCmdPtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingNextHopPacketCmdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(nhPacketCmd, nhPacketCmdGet,
                      "got another nhPacketCmd than was set: %d", dev);

        /*
            1.1.1. Call function with nhPacketCmd [CPSS_PACKET_CMD_DROP_SOFT_E]
            Expected: GT_OK.
        */
        nhPacketCmd    = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, nhPacketCmd);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingNextHopPacketCmdGet
            Expected: GT_OK and the same nhPacketCmdPtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingNextHopPacketCmdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(nhPacketCmd, nhPacketCmdGet,
                      "got another nhPacketCmd than was set: %d", dev);

        /*
            1.1.1. Call function with nhPacketCmd [CPSS_PACKET_CMD_DROP_HARD_E]
            Expected: GT_OK.
        */
        nhPacketCmd    = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, nhPacketCmd);

        /* 1.1.2 Call cpssDxChBrgFdbRoutingNextHopPacketCmdGet
            Expected: GT_OK and the same nhPacketCmdPtr.
        */
        /* Verify values */
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbRoutingNextHopPacketCmdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(nhPacketCmd, nhPacketCmdGet,
                      "got another nhPacketCmd than was set: %d", dev);

        /*
        1.1.3 Call function with nhPacketCmd [CPSS_PACKET_CMD_FORWARD_E
                                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E
                                          CPSS_PACKET_CMD_NONE_E
                                          CPSS_PACKET_CMD_LOOPBACK_E,
                                          CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
                                          CPSS_PACKET_CMD_BRIDGE_E]
        Expected: GT_BAD_PARAM.
        */
        nhPacketCmd    = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, nhPacketCmd);

        nhPacketCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, nhPacketCmd);

        nhPacketCmd    = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, nhPacketCmd);


        nhPacketCmd    = CPSS_PACKET_CMD_LOOPBACK_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, nhPacketCmd);

        nhPacketCmd    = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, nhPacketCmd);

        nhPacketCmd    = CPSS_PACKET_CMD_BRIDGE_E;

        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, nhPacketCmd);

        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbRoutingNextHopPacketCmdSet
                            (dev, nhPacketCmd),
                            nhPacketCmd);
    }
    /* set correct values*/
    nhPacketCmd   = CPSS_PACKET_CMD_ROUTE_E;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(dev, nhPacketCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutingNextHopPacketCmdGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_PACKET_CMD_ENT         &nhPacketCmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutingNextHopPacketCmdGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL nhPacketCmd
    Expected: GT_OK.
    1.1.2. Call function with nhPacketCmd [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    CPSS_PACKET_CMD_ENT        nhPacketCmd    = CPSS_PACKET_CMD_FORWARD_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function with not NULL enablePtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with nhPacketCmdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                        "%d, nhPacketCmdPtr = NULL", dev);
     }

    /* set correct values*/
    nhPacketCmd   = CPSS_PACKET_CMD_ROUTE_E;
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(dev, &nhPacketCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgFdbRouting suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgFdbRouting)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingPortIpUcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingPortIpUcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcRefreshEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcRefreshEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcAgingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcAgingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcDeleteEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcDeleteEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcTransplantEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcTransplantEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcAAandTAToCpuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingUcAAandTAToCpuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingNextHopPacketCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutingNextHopPacketCmdGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgFdbRouting)


