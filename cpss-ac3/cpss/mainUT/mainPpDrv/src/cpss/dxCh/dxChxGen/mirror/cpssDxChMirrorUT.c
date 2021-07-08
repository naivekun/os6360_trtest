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
* @file cpssDxChMirrorUT.c
*
* @brief Unit tests for cpssDxChMirror, that provides
* function implementations for Prestera port mirroring facility.
*
* @version   45
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/generic/mirror/private/prvCpssMirrorTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define MIRROR_VALID_VIRT_PORT_CNS  0

/* Tests use this vlan id for testing VLAN functions */
#define MIRROR_TESTED_VLAN_ID_CNS   100

/* Tests use this value as out-of-range value for vlanId */
#define PRV_CPSS_MAX_NUM_VLANS_CNS  4096

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerVlanTagEnable
(
    IN GT_U8                    dev,
    IN GT_U32                   portNum,
    IN GT_BOOL                  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerVlanTagEnable)
{
/*
    ITERATE_DEVICES_VIRT_CPU_PORTS (DxCh2 and above)
    1.1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChMirrorAnalyzerVlanTagEnableGet with non-NULL enablePtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM  port     = MIRROR_VALID_VIRT_PORT_CNS;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE and GT_FALSE].
                Expected: GT_OK.
            */

            /* Call function with enable [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChMirrorAnalyzerVlanTagEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* Call function with enable [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChMirrorAnalyzerVlanTagEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChMirrorAnalyzerVlanTagEnableGet with non-NULL enablePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChMirrorAnalyzerVlanTagEnableGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                                         "get another enable than was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMirrorAnalyzerVlanTagEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChMirrorAnalyzerVlanTagEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorAnalyzerVlanTagEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = MIRROR_VALID_VIRT_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerVlanTagEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerVlanTagEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerVlanTagEnableGet
(
    IN  GT_U8                    dev,
    IN  GT_U32                   portNum,
    OUT GT_BOOL                  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerVlanTagEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_CPU_PORTS (DxCh2 and above)
    1.1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM port  = MIRROR_VALID_VIRT_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = MIRROR_VALID_VIRT_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerVlanTagEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxAnalyzerVlanTagConfig
(
    IN GT_U8                                       dev,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxAnalyzerVlanTagConfig)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 2,
                                                       cfi = 1,
                                                       vid = 100 } ].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorTxAnalyzerVlanTagConfigGet with non-NULL analyzerVlanTagConfigPtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with analyzerVlanTagConfigPtr [{etherType = 0xFFFF,
                                                       vpt = 0,
                                                       cfi = 0,
                                                       vid = 100 }].
    Expected: GT_OK.
    1.4. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 8,
                                                       cfi = 0,
                                                       vid = 100 }].
    Expected: NON GT_OK.
    1.5. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 0,
                                                       cfi = 2,
                                                       vid = 100 }].
    Expected: NON GT_OK.
    1.6. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 0,
                                                       cfi = 0,
                                                       vid =  PRV_CPSS_MAX_NUM_VLANS_CNS }].
    Expected: GT_BAD_PARAM.
    1.7. Call function with analyzerVlanTagConfigPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                  st = GT_OK;

    GT_U8                                      dev;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC config;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC configGet;


    cpssOsBzero((GT_VOID*) &config, sizeof(config));
    cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 0,
                                                               cfi = 0,
                                                               vid = 100 } ].
            Expected: GT_OK.
        */
        config.etherType = 0;
        config.vpt       = 2;
        config.cfi       = 1;
        config.vid       = MIRROR_TESTED_VLAN_ID_CNS;

        st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorTxAnalyzerVlanTagConfigGet with non-NULL analyzerVlanTagConfigPtr.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChMirrorTxAnalyzerVlanTagConfigGet(dev, &configGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChMirrorTxAnalyzerVlanTagConfigGet: %d", dev);

        /* Verifying analyzerVlanTagConfigPtr fields */
        UTF_VERIFY_EQUAL1_STRING_MAC(config.etherType, configGet.etherType,
                   "get another analyzerVlanTagConfigPtr->etherType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(config.vpt, configGet.vpt,
                   "get another analyzerVlanTagConfigPtr->vpt than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(config.cfi, configGet.cfi,
                   "get another analyzerVlanTagConfigPtr->cfi than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(config.vid, configGet.vid,
                   "get another analyzerVlanTagConfigPtr->vid than was set: %d", dev);

        /*
            1.3. Call function with analyzerVlanTagConfigPtr [{etherType = 0xFFFF,
                                                               vpt = 2,
                                                               cfi = 1,
                                                               vid = 100 }].
            Expected: GT_OK.
        */
        config.etherType = 0xFFFF;

        st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, analyzerVlanTagConfigPtr->etherType = %d", dev, config.etherType);

        /*
            1.4. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 8,
                                                               cfi = 1,
                                                               vid = 100 }].
            Expected: GT_OK.
        */
        config.etherType = 0;
        config.vpt       = CPSS_USER_PRIORITY_RANGE_CNS;

        st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, analyzerVlanTagConfigPtr->vpt = %d", dev, config.vpt);

        /*
            1.5. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 0,
                                                               cfi = 2,
                                                               vid = 100 }].
            Expected: GT_OK.
        */
        config.vpt = 0;
        config.cfi = 2;

        st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, analyzerVlanTagConfigPtr->cfi = %d", dev, config.cfi);

        /*
            1.6. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 0,
                                                               cfi = 0,
                                                               vid =  PRV_CPSS_MAX_NUM_VLANS_CNS }].
            Expected: GT_BAD_PARAM.
        */
        config.cfi = 0;
        config.vid = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, analyzerVlanTagConfigPtr->vid = %d", dev, config.vid);

        /*
            1.7. Call function with analyzerVlanTagConfigPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        config.vid = MIRROR_TESTED_VLAN_ID_CNS;;

        st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerVlanTagConfigPtr = NULL", dev);
    }

    config.etherType = 0;
    config.vpt       = 0;
    config.cfi       = 0;
    config.vid       = MIRROR_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxAnalyzerVlanTagConfig(dev, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxAnalyzerVlanTagConfigGet
(
    IN GT_U8                                       dev,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxAnalyzerVlanTagConfigGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with non-NULL analyzerVlanTagConfigPtr.
    Expected: GT_OK.
    1.2. Call function with analyzerVlanTagConfigPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                  st = GT_OK;

    GT_U8                                      dev;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC config;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL analyzerVlanTagConfigPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorTxAnalyzerVlanTagConfigGet(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with analyzerVlanTagConfigPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxAnalyzerVlanTagConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerVlanTagConfigPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxAnalyzerVlanTagConfigGet(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxAnalyzerVlanTagConfigGet(dev, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxAnalyzerVlanTagConfig
(
    IN GT_U8                                       dev,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxAnalyzerVlanTagConfig)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 5,
                                                       cfi = 0,
                                                       vid = 100}].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorRxAnalyzerVlanTagConfigGet with non-NULL analyzerVlanTagConfigPtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with analyzerVlanTagConfigPtr [{etherType = 0xFFFF,
                                                       vpt = 0,
                                                       cfi = 0,
                                                       vid = 100}].
    Expected: GT_OK.
    1.4. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 8,
                                                       cfi = 0,
                                                       vid = 100 }].
    Expected: NON GT_OK.
    1.5. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 0,
                                                       cfi = 2,
                                                       vid = 100}].
    Expected: NON GT_OK.
    1.6. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                       vpt = 0,
                                                       cfi = 0,
                                                       vid =  PRV_CPSS_MAX_NUM_VLANS_CNS}].
    Expected: GT_BAD_PARAM.
    1.7. Call function with analyzerVlanTagConfigPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                  st = GT_OK;

    GT_U8                                      dev;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC config;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC configGet;


    cpssOsBzero((GT_VOID*) &config, sizeof(config));
    cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 0,
                                                               cfi = 0,
                                                               vid = 100 } ].
            Expected: GT_OK.
        */
        config.etherType = 0;
        config.vpt       = 5;
        config.cfi       = 0;
        config.vid       = MIRROR_TESTED_VLAN_ID_CNS;

        st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorRxAnalyzerVlanTagConfigGet with non-NULL analyzerVlanTagConfigPtr.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChMirrorRxAnalyzerVlanTagConfigGet(dev, &configGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChMirrorRxAnalyzerVlanTagConfigGet: %d", dev);

        /* Verifying analyzerVlanTagConfigPtr fields */
        UTF_VERIFY_EQUAL1_STRING_MAC(config.etherType, configGet.etherType,
                   "get another analyzerVlanTagConfigPtr->etherType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(config.vpt, configGet.vpt,
                   "get another analyzerVlanTagConfigPtr->vpt than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(config.cfi, configGet.cfi,
                   "get another analyzerVlanTagConfigPtr->cfi than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(config.vid, configGet.vid,
                   "get another analyzerVlanTagConfigPtr->vid than was set: %d", dev);

        /*
            1.3. Call function with analyzerVlanTagConfigPtr [{etherType = 0xFFFF,
                                                               vpt = 5,
                                                               cfi = 0,
                                                               vid = 100 }].
            Expected: GT_OK.
        */
        config.etherType = 0xFFFF;

        st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, analyzerVlanTagConfigPtr->etherType = %d", dev, config.etherType);

        /*
            1.4. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 8,
                                                               cfi = 0,
                                                               vid = 100 }].
            Expected: GT_OK.
        */
        config.etherType = 0;
        config.vpt       = CPSS_USER_PRIORITY_RANGE_CNS;

        st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, analyzerVlanTagConfigPtr->vpt = %d", dev, config.vpt);

        /*
            1.5. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 0,
                                                               cfi = 2,
                                                               vid = 100 }].
            Expected: GT_OK.
        */
        config.vpt = 0;
        config.cfi = 2;

        st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, analyzerVlanTagConfigPtr->cfi = %d", dev, config.cfi);

        /*
            1.6. Call function with analyzerVlanTagConfigPtr [{etherType = 0,
                                                               vpt = 0,
                                                               cfi = 0,
                                                               vid =  PRV_CPSS_MAX_NUM_VLANS_CNS }].
            Expected: GT_BAD_PARAM.
        */
        config.cfi = 0;
        config.vid = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, analyzerVlanTagConfigPtr->vid = %d", dev, config.vid);

        /*
            1.7. Call function with analyzerVlanTagConfigPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        config.vid = MIRROR_TESTED_VLAN_ID_CNS;;

        st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerVlanTagConfigPtr = NULL", dev);
    }

    config.etherType = 0;
    config.vpt       = 0;
    config.cfi       = 0;
    config.vid       = MIRROR_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxAnalyzerVlanTagConfig(dev, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxAnalyzerVlanTagConfigGet
(
    IN  GT_U8                                       dev,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxAnalyzerVlanTagConfigGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with non-NULL analyzerVlanTagConfigPtr.
    Expected: GT_OK.
    1.2. Call function with analyzerVlanTagConfigPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                  st = GT_OK;

    GT_U8                                      dev;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC config;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL analyzerVlanTagConfigPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorRxAnalyzerVlanTagConfigGet(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with analyzerVlanTagConfigPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorRxAnalyzerVlanTagConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerVlanTagConfigPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxAnalyzerVlanTagConfigGet(dev, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxAnalyzerVlanTagConfigGet(dev, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxAnalyzerPortSet
(
    IN  GT_U8   dev,
    IN  GT_U8   analyzerPort,
    IN  GT_U8   analyzerHwDev
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxAnalyzerPortSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with analyzerPort [2],
                            analyzerHwDev [1].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorTxAnalyzerPortGet with non-NULL analyzerPort and analyzerHwDev.
    Expected: GT_OK and same values as written.
*/
    GT_STATUS st              = GT_OK;

    GT_U8     dev;
    GT_PORT_NUM     analyzerPort    = 0;
    GT_HW_DEV_NUM   analyzerHwDev     = 0;
    GT_PORT_NUM     analyzerPortGet = 0;
    GT_HW_DEV_NUM   analyzerHwDevGet  = 0;
    GT_U32          notApplicableDeviceFimilies;

    notApplicableDeviceFimilies = UTF_XCAT_E | UTF_XCAT3_E
        | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E  | UTF_BOBCAT2_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E
        | UTF_ALDRIN2_E | UTF_FALCON_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDeviceFimilies);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with analyzerPort [2],
                                    analyzerHwDev [1].
            Expected: GT_OK.
        */
        analyzerPort = 2;
        analyzerHwDev  = 1;

        st = cpssDxChMirrorTxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerPort, analyzerHwDev);

        /*
            1.2. Call cpssDxChMirrorTxAnalyzerPortGet with non-NULL analyzerPort and analyzerHwDev.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChMirrorTxAnalyzerPortGet(dev, &analyzerPortGet, &analyzerHwDevGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChMirrorTxAnalyzerPortGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerPort, analyzerPortGet,
                   "get another analyzerPort than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerHwDev, analyzerHwDevGet,
                   "get another analyzerHwDev than was set: %d", dev);
    }

    analyzerPort = 0;
    analyzerHwDev  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDeviceFimilies);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxAnalyzerPortGet
(
    IN  GT_U8   dev,
    OUT GT_U8   *analyzerPortPtr,
    OUT GT_U8   *analyzerHwDevPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxAnalyzerPortGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr.
    Expected: GT_OK.
    1.2. Call with analyzerPortPtr [NULL] and non-NULL analyzerHwDevPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    GT_PORT_NUM     analyzerPort    = 0;
    GT_HW_DEV_NUM   analyzerHwDev     = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorTxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with analyzerPortPtr [NULL] and non-NULL analyzerHwDevPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxAnalyzerPortGet(dev, NULL, &analyzerHwDev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerPortPtr = NULL", dev);

        /*
            1.3. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxAnalyzerPortGet(dev, &analyzerPort, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerHwDevPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxAnalyzerPortSet
(
    IN  GT_U8   dev,
    IN  GT_U8   analyzerPort,
    IN  GT_U8   analyzerHwDev
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxAnalyzerPortSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with analyzerPort [3],
                            analyzerHwDev [4].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorRxAnalyzerPortGet with non-NULL analyzerPort and analyzerHwDev.
    Expected: GT_OK and same values as written.
*/
    GT_STATUS st              = GT_OK;

    GT_U8     dev;
    GT_PORT_NUM     analyzerPort    = 0;
    GT_HW_DEV_NUM   analyzerHwDev     = 0;
    GT_PORT_NUM     analyzerPortGet = 0;
    GT_HW_DEV_NUM   analyzerHwDevGet  = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with analyzerPort [3],
                                    analyzerHwDev [4].
            Expected: GT_OK.
        */
        analyzerPort = 3;
        analyzerHwDev  = 4;

        st = cpssDxChMirrorRxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerPort, analyzerHwDev);

        /*
            1.2. Call cpssDxChMirrorRxAnalyzerPortGet with non-NULL analyzerPort and analyzerHwDev.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChMirrorRxAnalyzerPortGet(dev, &analyzerPortGet, &analyzerHwDevGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChMirrorRxAnalyzerPortGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerPort, analyzerPortGet,
                   "get another analyzerPort than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerHwDev, analyzerHwDevGet,
                   "get another analyzerHwDev than was set: %d", dev);
    }

    analyzerPort = 0;
    analyzerHwDev  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxAnalyzerPortGet
(
    IN  GT_U8   dev,
    OUT GT_U8   *analyzerPortPtr,
    OUT GT_U8   *analyzerHwDevPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxAnalyzerPortGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr.
    Expected: GT_OK.
    1.2. Call with analyzerPortPtr [NULL] and non-NULL analyzerHwDevPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    GT_PORT_NUM     analyzerPort    = 0;
    GT_HW_DEV_NUM   analyzerHwDev     = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorRxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with analyzerPortPtr [NULL] and non-NULL analyzerHwDevPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorRxAnalyzerPortGet(dev, NULL, &analyzerHwDev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerPortPtr = NULL", dev);

        /*
            1.3. Call with non-NULL analyzerPortPtr and analyzerHwDevPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorRxAnalyzerPortGet(dev, &analyzerPort, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerHwDevPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxCascadeMonitorEnable
(
    IN  GT_U8   dev,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxCascadeMonitorEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorTxCascadeMonitorEnableGet with non-NULL enablePtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_BOOL   state    = GT_FALSE;
    GT_BOOL   stateGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call function with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChMirrorTxCascadeMonitorEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /* Call function with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChMirrorTxCascadeMonitorEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChMirrorTxCascadeMonitorEnableGet with non-NULL enablePtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrorTxCascadeMonitorEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxCascadeMonitorEnableGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d", dev);
    }

    state = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxCascadeMonitorEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxCascadeMonitorEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxCascadeMonitorEnableGet
(
    IN   GT_U8   dev,
    OUT  GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxCascadeMonitorEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_BOOL   state = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorTxCascadeMonitorEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxCascadeMonitorEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxCascadeMonitorEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxCascadeMonitorEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    IN  GT_BOOL         enable,
    IN  GT_U32          index
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxPortSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mirrPort [0],
                            enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorRxPortGet with mirrPort [0], non-NULL enablePtr.
    Expected: GT_OK and same value as written.
    1.3. Call function with mirrPort [CPSS_CPU_PORT_NUM_CNS = 63],
                            enable [GT_TRUE].
    Expected: GT_OK.
    1.4. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                            enable [GT_TRUE].
    Expected: NOT GT_OK.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             mirrPort = 0;
    GT_BOOL                 state    = GT_FALSE;
    GT_BOOL                 stateGet = GT_FALSE;
    GT_U32                  index = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&mirrPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For each active device (devNum) id go over all active virtual portNums. */
        while (GT_OK == prvUtfNextVirtPortGet(&mirrPort, GT_TRUE))
        {
            /*
                1.1. Call function with mirrPort [0],
                                        enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            state = GT_TRUE;

            st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_FALSE, state, 0);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call cpssDxChMirrorRxPortGet with mirrPort [0], non-NULL enablePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_FALSE, &stateGet, &index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChMirrorRxPortGet: %d, %d", dev, mirrPort);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another enable than was set: %d", dev);

            /*
                1.1. Call function with mirrPort [0],
                                        enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_FALSE, state, 0);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call cpssDxChMirrorRxPortGet with mirrPort [0], non-NULL enablePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_FALSE, &stateGet, &index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChMirrorRxPortGet: %d, %d", dev, mirrPort);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another enable than was set: %d", dev);
        }

        st = prvUtfNextPhyPortReset(&mirrPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For each active device (devNum) id go over all active physical portNums. */
        while (GT_OK == prvUtfNextPhyPortGet(&mirrPort, GT_TRUE))
        {
            /*
                1.1. Call function with mirrPort [0],
                                        enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            state = GT_TRUE;

            st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, state, 0);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call cpssDxChMirrorRxPortGet with mirrPort [0], non-NULL enablePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &stateGet, &index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChMirrorRxPortGet: %d, %d", dev, mirrPort);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another enable than was set: %d", dev);

            /*
                1.1. Call function with mirrPort [0],
                                        enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, state, 0);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call cpssDxChMirrorRxPortGet with mirrPort [0], non-NULL enablePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &stateGet, &index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChMirrorRxPortGet: %d, %d", dev, mirrPort);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another enable than was set: %d", dev);
        }

        /*
            1.3. Call function with mirrPort [CPSS_CPU_PORT_NUM_CNS = 63],
                                    enable [GT_TRUE].
            Expected: GT_OK.
        */
        mirrPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, state, 0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.4. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                                    enable [GT_TRUE].
            Expected: NOT GT_OK.
        */
        mirrPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, state, 0);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);
    }

    state = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, state, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, state, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    OUT GT_BOOL         *enablePtr,
    OUT GT_U32          *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxPortGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mirrPort [0],
                            non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with mirrPort [CPSS_CPU_PORT_NUM_CNS = 63],
                            non-NULL enablePtr.
    Expected: GT_OK.
    1.3. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                            non-NULL enablePtr.
    Expected: NON GT_OK.
    1.4. Call function with mirrPort [0],
                            enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             mirrPort = 0;
    GT_BOOL                 state    = GT_FALSE;
    GT_U32                  index = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mirrPort [0],
                                    non-NULL enablePtr.
            Expected: GT_OK.
        */
        mirrPort = 0;

        st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.2. Call function with mirrPort [CPSS_CPU_PORT_NUM_CNS = 63],
                                    non-NULL enablePtr.
            Expected: GT_OK.
        */
        mirrPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.3. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                                    non-NULL enablePtr.
            Expected: NOT GT_OK.
        */
        mirrPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.4. Call function with mirrPort [0],
                                    enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        mirrPort = 0;

        st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxPortSet
(
    IN  GT_U8   dev,
    IN  GT_U8   mirrPort,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxPortSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mirrPort [from 0 to numOfVirtPorts(dev) -1]
                            and enable [GT_FALSE] - to be sure that we don't have mirrored ports.
    Expected: GT_OK.
    1.2. Call function cpssDxChMirrorTxPortGet mirrPort [from 0 to numOfVirtPorts(dev) -1] and non-NULL enablePtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with mirrPort [from 0 to PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2]
                            and enable [GT_TRUE].
    Expected: GT_OK.
    1.4. Call function with out of range mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                            enable [GT_TRUE].
    Expected: NOT GT_OK.
    1.5. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS],
                            enable [GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             mirrPort = 0;
    GT_BOOL                 state    = GT_FALSE;
    GT_BOOL                 stateGet = GT_FALSE;

    GT_U32                  index = 0;
    GT_U32                  indexGet = 0;
    GT_U32      validPorts[PRV_CPSS_MIRROR_MAX_PORTS_CNS - 1];
    GT_U32      ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&mirrPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For each active device (devNum) id go over all active virtual portNums. */
        while (GT_OK == prvUtfNextVirtPortGet(&mirrPort, GT_TRUE))
        {
            /*
                1.1. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and enable [GT_TRUE].
                Expected: GT_OK.
            */
            state = GT_TRUE;

            st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_FALSE, state, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call function cpssDxChMirrorTxPortGet mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS - 1] and non-NULL enablePtr.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_FALSE, &stateGet, &indexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);

            /*
                1.1. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and enable [GT_FALSE] - to be sure that we don't have mirrored ports.
                Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_FALSE, state, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call function cpssDxChMirrorTxPortGet mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS - 1] and non-NULL enablePtr.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_FALSE, &stateGet, &indexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);
        }

        st = prvUtfNextTxqPortReset(&mirrPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For each active device (devNum) id go over all active physical portNums. */
        while (GT_OK == prvUtfNextTxqPortGet(&mirrPort, GT_TRUE))
        {
            /*
                1.1. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and enable [GT_TRUE].
                Expected: GT_OK.
            */
            state = GT_TRUE;

            st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call function cpssDxChMirrorTxPortGet mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS - 1] and non-NULL enablePtr.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &stateGet, &indexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);

            /*
                1.1. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and enable [GT_FALSE] - to be sure that we don't have mirrored ports.
                Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /*
                1.2. Call function cpssDxChMirrorTxPortGet mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS - 1] and non-NULL enablePtr.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &stateGet, &indexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);

            /*
                1.3. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and enable [GT_FALSE] and out of range index [7].
                Expected: GT_OK.
            */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                index = 7;

                st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, mirrPort, state, index);

                index = 0;
            }
        }

        /*
            1.3. Call function with mirrPort [from 0 to PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2]
                                    and enable [GT_TRUE].
            Expected: GT_OK.
        */
        state = GT_TRUE;

        /* get ports with TXQ */
        prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_TXQ_E,
            0  /*start port */, &validPorts[0] , PRV_CPSS_MIRROR_MAX_PORTS_CNS - 1);


        for (ii = 0; ii <= PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2; ii++)
        {
            mirrPort = validPorts[ii];
            st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);

            /* 1.2. for mirrPort [from 0 to PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2] and enable set to [GT_TRUE]. */
            st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &stateGet, &indexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);

            /*
                1.3. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and enable [GT_TRUE] and out of range index [7].
                Expected: GT_OK.
            */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                index = 7;

                st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, mirrPort, state, index);

                index = 0;
            }
        }

        /*
            1.4. Call function with out of range mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                                    enable [GT_TRUE].
            Expected: NOT GT_OK.
        */
        mirrPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        index = 0;

        st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.5. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS],
                                    enable [GT_TRUE].
            Expected: GT_OK.
        */
        mirrPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /* 1.2. for mirrPort [from 0 to CPSS_CPU_PORT_NUM_CNS = 63] and enable set to [GT_TRUE]. */
        st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &stateGet, &indexGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                 "cpssDxChMirrorTxPortGet: %d, %d", dev, mirrPort);
        UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                                 "get another enable than was set: %d, %d", dev, mirrPort);

        /* to clear mirrored ports */
        state = GT_FALSE;
        for (ii = 0; ii <= PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2; ii++)
        {
            mirrPort = validPorts[ii];
            st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, state);
        }
    }

    state    = GT_FALSE;
    mirrPort = 0;
    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, state, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxPortGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    IN  GT_BOOL                 isPhysicalPort,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxPortGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mirrPort [0],
                            non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                            non-NULL enablePtr.
    Expected: NOT GT_OK.
    1.3. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS]
                            and non-null enablePtr.
    Expected: GT_OK.
    1.4. Call function with mirrPort [0],
                            enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             mirrPort = 0;
    GT_BOOL                 state    = GT_FALSE;
    GT_U32                  index = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mirrPort [0],
                                    non-NULL enablePtr.
            Expected: GT_OK.
        */
        mirrPort = 0;

        st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.2. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                                    non-NULL enablePtr.
            Expected: NOT GT_OK.
        */
        mirrPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.3. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS]
                                    and non-null enablePtr.
            Expected: GT_OK.
        */
        mirrPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.4. Call function with mirrPort [0],
                                    enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        mirrPort = 0;

        st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &state, &index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrRxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    dev,
    IN GT_U32   ratio
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrRxStatMirroringToAnalyzerRatioSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with ratio [0 / 2047 ].
    Expected: GT_OK.
    1.2. Call function cpssDxChMirrRxStatMirroringToAnalyzerRatioGet with non-NULL ratioPtr.
    Expected: GT_OK and same value as written.
    1.3. Call function with ratio [ 2048 ].
    Expected: NOT GT_OK.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_U32    ratio    = 0;
    GT_U32    ratioGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ratio [0 / 2047 ].
            Expected: GT_OK.
        */

        /* Call function with ratio [0] */
        ratio = 0;

        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);

        /* Call function with enable [2047] */
        ratio = 2047;

        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);

        /*
            1.2. Call function cpssDxChMirrRxStatMirroringToAnalyzerRatioGet with non-NULL ratioPtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioGet(dev, &ratioGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrRxStatMirroringToAnalyzerRatioGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(ratio, ratioGet,
                                     "get another ratio than was set: %d", dev);

        /*
            1.3. Call function with ratio [ 2048 ].
            Expected: NOT GT_OK.
        */
        ratio = 2048;

        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);
    }

    ratio = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE)) 
    {
        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(dev, ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrRxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    dev,
    OUT GT_U32   *ratioPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrRxStatMirroringToAnalyzerRatioGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL ratioPtr.
    Expected: GT_OK.
    1.2. Call function with ratioPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    ratio = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL ratioPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioGet(dev, &ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with ratioPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ratioPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrRxStatMirroringToAnalyzerRatioGet(dev, &ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrRxStatMirroringToAnalyzerRatioGet(dev, &ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrRxStatMirrorToAnalyzerEnable
(
    IN GT_U8     dev,
    IN GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrRxStatMirrorToAnalyzerEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrRxStatMirrorToAnalyzerEnableGet with non-NULL enablePtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_BOOL   state    = GT_FALSE;
    GT_BOOL   stateGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call function with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChMirrRxStatMirrorToAnalyzerEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /* Call function with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChMirrRxStatMirrorToAnalyzerEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChMirrRxStatMirrorToAnalyzerEnableGet with non-NULL enablePtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrRxStatMirrorToAnalyzerEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrRxStatMirrorToAnalyzerEnableGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d", dev);
    }

    state = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrRxStatMirrorToAnalyzerEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrRxStatMirrorToAnalyzerEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrRxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     dev,
    OUT GT_BOOL   *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrRxStatMirrorToAnalyzerEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_BOOL   state = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrRxStatMirrorToAnalyzerEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrRxStatMirrorToAnalyzerEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrRxStatMirrorToAnalyzerEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrRxStatMirrorToAnalyzerEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrRxAnalyzerDpTcSet
(
    IN GT_U8             dev,
    IN CPSS_DP_LEVEL_ENT analyzerDp,
    IN GT_U8             analyzerTc
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrRxAnalyzerDpTcSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with analyzerDp [CPSS_DP_RED_E /
                                        CPSS_DP_GREEN_E],
                            analyzerTc [CPSS_TC_RANGE_CNS - 1].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrRxAnalyzerDpTcGet with non-NULL analyzerDpPtr and analyzerTcPtr.
    Expected: GT_OK and same values as written.
    1.3. Call with out of range analyzerDp [5 / wrong enum values],
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range analyzerDp [CPSS_DP_YELLOW_E], analyzerTc [CPSS_TC_RANGE_CNS - 1].
    Expected: NON GT_OK.
    1.5. Call with out of range analyzerTc [CPSS_TC_RANGE_CNS],
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS         st            = GT_OK;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp    = CPSS_DP_GREEN_E;
    GT_U8             analyzerTc    = 0;
    CPSS_DP_LEVEL_ENT analyzerDpGet = CPSS_DP_GREEN_E;
    GT_U8             analyzerTcGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with analyzerDp [CPSS_DP_RED_E /
                                                CPSS_DP_YELLOW_E /
                                                CPSS_DP_GREEN_E],
                                    analyzerTc [CPSS_TC_RANGE_CNS - 1].
            Expected: GT_OK.
        */
        analyzerTc = CPSS_TC_RANGE_CNS - 1;

        /* Call with analyzerDp [CPSS_DP_RED_E] */
        analyzerDp = CPSS_DP_RED_E;

        st = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);

        /* Call with analyzerDp [CPSS_DP_GREEN_E] */
        analyzerDp = CPSS_DP_GREEN_E;

        st = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);

        /*
            1.2. Call cpssDxChMirrRxAnalyzerDpTcGet with non-NULL analyzerDpPtr and analyzerTcPtr.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChMirrRxAnalyzerDpTcGet(dev, &analyzerDpGet, &analyzerTcGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrRxAnalyzerDpTcGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerDp, analyzerDpGet,
                                     "get another analyzerDp than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerTc, analyzerTcGet,
                                     "get another analyzerTc than was set: %d", dev);

        /*
            1.3. Call with out of range analyzerDp [5 / wrong enum values],
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        analyzerTc = CPSS_TC_RANGE_CNS - 1;

        UTF_ENUMS_CHECK_MAC(cpssDxChMirrRxAnalyzerDpTcSet
                            (dev, analyzerDp, analyzerTc),
                            analyzerDp);

        /* 1.4. Call with out of range analyzerDp [CPSS_DP_YELLOW_E],
        analyzerTc [CPSS_TC_RANGE_CNS - 1]. Expected: NON GT_OK. */
        analyzerDp = CPSS_DP_YELLOW_E;

        st = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(
                GT_OK, st, dev, analyzerDp, analyzerTc);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(
                GT_OK, st, dev, analyzerDp, analyzerTc);
        }

        /*
            1.5. Call with out of range analyzerTc [CPSS_TC_RANGE_CNS],
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        analyzerDp = CPSS_DP_RED_E;
        analyzerTc = CPSS_TC_RANGE_CNS;

        st = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, analyzerTc = %d", dev, analyzerTc);

        /*
            1.6. Call function with analyzerDp [CPSS_DP_YELLOW_E ,
                                    analyzerTc 0.
            Expected: GT_OK.
        */
        analyzerTc = 0;

        /* Call with analyzerDp [CPSS_DP_YELLOW_E] */
        analyzerDp = CPSS_DP_YELLOW_E;

        st = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);
        }

        if (st == GT_OK)
        {
            st = cpssDxChMirrRxAnalyzerDpTcGet(dev, &analyzerDpGet, &analyzerTcGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChMirrRxAnalyzerDpTcGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(analyzerDp, analyzerDpGet,
                                         "get another analyzerDp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(analyzerTc, analyzerTcGet,
                                         "get another analyzerTc than was set: %d", dev);
        }
    }

    analyzerTc = CPSS_TC_RANGE_CNS - 1;
    analyzerDp = CPSS_DP_RED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrRxAnalyzerDpTcGet
(
    IN  GT_U8              dev,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrRxAnalyzerDpTcGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL analyzerDpPtr
                            and non-NULL analyzerTcPtr.
    Expected: GT_OK.
    1.2. Call function with analyzerDpPtr [NULL]
                            and non-NULL analyzerTcPtr.
    Expected: GT_BAD_PTR.
    1.3. Call function with non-NULL analyzerDpPtr
                            and analyzerTcPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS         st         = GT_OK;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp = CPSS_DP_GREEN_E;
    GT_U8             analyzerTc = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL analyzerDpPtr
                                    and non-NULL analyzerTcPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrRxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with analyzerDpPtr [NULL]
                                    and non-NULL analyzerTcPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrRxAnalyzerDpTcGet(dev, NULL, &analyzerTc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerDpPtr = NULL", dev);

        /*
            1.3. Call function with non-NULL analyzerDpPtr
                                    and analyzerTcPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrRxAnalyzerDpTcGet(dev, &analyzerDp, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerTcPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrRxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrRxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrTxAnalyzerDpTcSet
(
    IN GT_U8              dev,
    IN CPSS_DP_LEVEL_ENT  analyzerDp,
    IN GT_U8              analyzerTc
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrTxAnalyzerDpTcSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with analyzerDp [CPSS_DP_RED_E /
                                        CPSS_DP_GREEN_E],
                            analyzerTc [CPSS_TC_RANGE_CNS - 1].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrTxAnalyzerDpTcGet with non-NULL analyzerDpPtr and analyzerTcPtr.
    Expected: GT_OK and same values as written.
    1.3. Call with out of range analyzerDp [5 / ],
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range analyzerDp [CPSS_DP_YELLOW_E],
    analyzerTc [CPSS_TC_RANGE_CNS - 1].  Expected: NON GT_OK.
    1.5. Call with out of range analyzerTc [CPSS_TC_RANGE_CNS],
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS         st            = GT_OK;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp    = CPSS_DP_GREEN_E;
    GT_U8             analyzerTc    = 0;
    CPSS_DP_LEVEL_ENT analyzerDpGet = CPSS_DP_GREEN_E;
    GT_U8             analyzerTcGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with analyzerDp [CPSS_DP_RED_E /
                                                CPSS_DP_YELLOW_E /
                                                CPSS_DP_GREEN_E],
                                    analyzerTc [CPSS_TC_RANGE_CNS - 1].
            Expected: GT_OK.
        */
        analyzerTc = CPSS_TC_RANGE_CNS - 1;

        /* Call with analyzerDp [CPSS_DP_RED_E] */
        analyzerDp = CPSS_DP_RED_E;

        st = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);

        /* Call with analyzerDp [CPSS_DP_GREEN_E] */
        analyzerDp = CPSS_DP_GREEN_E;

        st = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);

        /*
            1.2. Call cpssDxChMirrTxAnalyzerDpTcGet with non-NULL analyzerDpPtr and analyzerTcPtr.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChMirrTxAnalyzerDpTcGet(dev, &analyzerDpGet, &analyzerTcGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrTxAnalyzerDpTcGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerDp, analyzerDpGet,
                                     "get another analyzerDp than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(analyzerTc, analyzerTcGet,
                                     "get another analyzerTc than was set: %d", dev);

        /*
            1.3. Call with out of range analyzerDp [5 / wrong enum values],
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        analyzerTc = CPSS_TC_RANGE_CNS - 1;

        UTF_ENUMS_CHECK_MAC(cpssDxChMirrTxAnalyzerDpTcSet
                            (dev, analyzerDp, analyzerTc),
                            analyzerDp);

        /* 1.4. Call with out of range analyzerDp [CPSS_DP_YELLOW_E],
        analyzerTc [CPSS_TC_RANGE_CNS - 1].  Expected: NON GT_OK. */
        analyzerDp = CPSS_DP_YELLOW_E;

        st = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(
                GT_OK, st, dev, analyzerDp, analyzerTc);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(
                GT_OK, st, dev, analyzerDp, analyzerTc);
        }

        /*
            1.5. Call with out of range analyzerTc [CPSS_TC_RANGE_CNS],
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        analyzerDp = CPSS_DP_RED_E;
        analyzerTc = CPSS_TC_RANGE_CNS;

        st = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, analyzerTc = %d", dev, analyzerTc);

        /*
            1.6. Call function with analyzerDp [CPSS_DP_YELLOW_E ,
                                    analyzerTc 0.
            Expected: GT_OK.
        */
        analyzerTc = 0;

        /* Call with analyzerDp [CPSS_DP_YELLOW_E] */
        analyzerDp = CPSS_DP_YELLOW_E;

        st = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, analyzerDp, analyzerTc);
        }

        if (st == GT_OK)
        {
            st = cpssDxChMirrTxAnalyzerDpTcGet(dev, &analyzerDpGet, &analyzerTcGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChMirrTxAnalyzerDpTcGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(analyzerDp, analyzerDpGet,
                                         "get another analyzerDp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(analyzerTc, analyzerTcGet,
                                         "get another analyzerTc than was set: %d", dev);
        }

    }

    analyzerTc = CPSS_TC_RANGE_CNS - 1;
    analyzerDp = CPSS_DP_RED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrTxAnalyzerDpTcGet
(
    IN  GT_U8              dev,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrTxAnalyzerDpTcGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL analyzerDpPtr
                            and non-NULL analyzerTcPtr.
    Expected: GT_OK.
    1.2. Call function with analyzerDpPtr [NULL]
                            and non-NULL analyzerTcPtr.
    Expected: GT_BAD_PTR.
    1.3. Call function with non-NULL analyzerDpPtr
                            and analyzerTcPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS         st         = GT_OK;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp = CPSS_DP_GREEN_E;
    GT_U8             analyzerTc = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL analyzerDpPtr
                                    and non-NULL analyzerTcPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrTxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with analyzerDpPtr [NULL]
                                    and non-NULL analyzerTcPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrTxAnalyzerDpTcGet(dev, NULL, &analyzerTc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerDpPtr = NULL", dev);

        /*
            1.3. Call function with non-NULL analyzerDpPtr
                                    and analyzerTcPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrTxAnalyzerDpTcGet(dev, &analyzerDp, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, analyzerTcPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrTxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrTxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrTxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    dev,
    IN GT_U32   ratio
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrTxStatMirroringToAnalyzerRatioSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with ratio [0 / 2047 ]. Expected: GT_OK.
    1.2. Call function cpssDxChMirrTxStatMirroringToAnalyzerRatioGet with non-NULL ratioPtr.
    Expected: GT_OK and same value as written.
    1.3. Call function with ratio [ 2048 ]. Expected: NOT GT_OK.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_U32    ratio    = 0;
    GT_U32    ratioGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ratio [0 / 2047 ].
            Expected: GT_OK.
        */

        /* Call function with ratio [0] */
        ratio = 0;

        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);

        /* Call function with enable [2047] */
        ratio = 2047;

        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);

        /*
            1.2. Call function cpssDxChMirrTxStatMirroringToAnalyzerRatioGet with non-NULL ratioPtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioGet(dev, &ratioGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrTxStatMirroringToAnalyzerRatioGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(ratio, ratioGet,
                                     "get another ratio than was set: %d", dev);

        /*
            1.3. Call function with ratio [ 2048 ].
            Expected: NOT GT_OK.
        */
        ratio = 2048;

        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);
    }

    ratio = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(dev, ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(dev, ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrTxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    dev,
    OUT GT_U32   *ratioPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrTxStatMirroringToAnalyzerRatioGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL ratioPtr.
    Expected: GT_OK.
    1.2. Call function with ratioPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    ratio = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL ratioPtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioGet(dev, &ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with ratioPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ratioPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrTxStatMirroringToAnalyzerRatioGet(dev, &ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrTxStatMirroringToAnalyzerRatioGet(dev, &ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrTxStatMirrorToAnalyzerEnable
(
    IN GT_U8     dev,
    IN GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrTxStatMirrorToAnalyzerEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrTxStatMirrorToAnalyzerEnableGet with non-NULL enablePtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_BOOL   state    = GT_FALSE;
    GT_BOOL   stateGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call function with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChMirrTxStatMirrorToAnalyzerEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /* Call function with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChMirrTxStatMirrorToAnalyzerEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChMirrTxStatMirrorToAnalyzerEnableGet with non-NULL enablePtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrTxStatMirrorToAnalyzerEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrTxStatMirrorToAnalyzerEnableGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d", dev);
    }

    state = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrTxStatMirrorToAnalyzerEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrTxStatMirrorToAnalyzerEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrTxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     dev,
    OUT GT_BOOL   *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrTxStatMirrorToAnalyzerEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_BOOL   state = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrTxStatMirrorToAnalyzerEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrTxStatMirrorToAnalyzerEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrTxStatMirrorToAnalyzerEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrTxStatMirrorToAnalyzerEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerInterfaceSet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerInterfaceSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with index [1 / 6].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorAnalyzerInterfaceGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3. Call function with out of range index [7].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    index = 0;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC interface;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC interfaceGet;

    GT_BOOL   failureWas = GT_TRUE;

    cpssOsBzero((GT_VOID*)&interface, sizeof(interface));
    cpssOsBzero((GT_VOID*)&interfaceGet, sizeof(interfaceGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [1 / 6].
            Expected: GT_OK.
        */
        /*call with index [1]*/
        index = 1;

        interface.interface.type = CPSS_INTERFACE_PORT_E;
        interface.interface.devPort.hwDevNum = dev;
        interface.interface.devPort.portNum = MIRROR_VALID_VIRT_PORT_CNS ;

        st = cpssDxChMirrorAnalyzerInterfaceSet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorAnalyzerInterfaceGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, &interfaceGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if (GT_OK == st)
        {
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&interface,
                                            (const GT_VOID*)&interfaceGet,
                                            sizeof(interface)))
                         ? GT_FALSE : GT_TRUE;

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, failureWas,
                "get another interface than was written [dev = %d, index =%d]",
                dev, index);
        }

        /*
            1.1. Call function with index [1 / 6].
            Expected: GT_OK.
        */
        cpssOsBzero((GT_VOID*)&interface, sizeof(interface));
        cpssOsBzero((GT_VOID*)&interfaceGet, sizeof(interfaceGet));

        /*call with index [6]*/
        index = 6;

        interface.interface.type = CPSS_INTERFACE_PORT_E;
        interface.interface.devPort.hwDevNum = dev;
        interface.interface.devPort.portNum = MIRROR_VALID_VIRT_PORT_CNS ;

        st = cpssDxChMirrorAnalyzerInterfaceSet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorAnalyzerInterfaceGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, &interfaceGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if (GT_OK == st)
        {
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&interface,
                                            (const GT_VOID*)&interfaceGet,
                                            sizeof(interface)))
                         ? GT_FALSE : GT_TRUE;

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, failureWas,
                "get another interface than was written [dev = %d, index =%d]",
                dev, index);
        }

        /*
            1.3. Call function with out of range index [7].
            Expected: GT_BAD_PARAM.
        */
        cpssOsBzero((GT_VOID*)&interface, sizeof(interface));

        /*call with index [7]*/
        index = 7;

        interface.interface.type = CPSS_INTERFACE_PORT_E;
        interface.interface.devPort.hwDevNum = dev;
        interface.interface.devPort.portNum = MIRROR_VALID_VIRT_PORT_CNS ;

        st = cpssDxChMirrorAnalyzerInterfaceSet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerInterfaceSet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerInterfaceSet(dev, index, &interface);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerInterfaceGet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerInterfaceGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with index [1 / 6].
    Expected: GT_OK.
    1.2. Call function with out of range index [7].
    Expected: GT_BAD_PARAM.
    1.3. Call with bad interfacePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    index = 0;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC interface;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [1 / 6].
            Expected: GT_OK.
        */
        /*call with index [1]*/
        index = 1;

        st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*call with index [6]*/
        index = 6;

        st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with out of range index [7].
            Expected: GT_BAD_PARAM.
        */

        /*call with index [7]*/
        index = 7;

        st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call with bad interfacePtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, &interface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerInterfaceGet(dev, index, &interface);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with index [1 / 6], and enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet function.
    Expected: GT_OK and the same value.
    1.3. Call function with out of range index [7].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_BOOL   enable = GT_TRUE;
    GT_U32    index = 0;
    GT_BOOL   enableGet;
    GT_U32    indexGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [1 / 6], and enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /*call with index [1] and enable [GT_TRUE]*/
        index = 1;
        enable = GT_TRUE;

        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet function.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                                    "get another index than was set: %d, %d", dev);


        /*call with index [6] and enable [GT_FALSE]*/
        index = 6;
        enable = GT_FALSE;

        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call function with out of range index [7].
            Expected: NOT GT_OK.
        */

        /*call with index [7]*/
        index = 7;

        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 1;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with bad enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call bad indexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_BOOL   enableGet;
    GT_U32    indexGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet: %d, %d", dev);

        /*
            1.2. Call with bad enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(dev, NULL, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);

        /*
            1.3. Call with bad indexPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, indexPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorToAnalyzerForwardingModeSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorToAnalyzerForwardingModeSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with mode
         [CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E /
          CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorToAnalyzerForwardingModeSetGet with non-NULL modePtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode = 0;
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT modeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode
                 [CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E /
                  CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E].
            Expected: GT_OK.
        */

        /* Call function with mode [CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E] */
        mode = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

        st = cpssDxChMirrorToAnalyzerForwardingModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);


        /*
            1.2. Call cpssDxChMirrorToAnalyzerForwardingModeGet with non-NULL modePtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrorToAnalyzerForwardingModeGet(dev, &modeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChMirrorToAnalyzerForwardingModeSetGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);


        /* Call function with mode [CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E] */
        mode = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E;

        st = cpssDxChMirrorToAnalyzerForwardingModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorToAnalyzerForwardingModeGet with non-NULL modePtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrorToAnalyzerForwardingModeGet(dev, &modeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChMirrorToAnalyzerForwardingModeSetGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMirrorToAnalyzerForwardingModeSet
                            (dev, mode),
                            mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorToAnalyzerForwardingModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorToAnalyzerForwardingModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorToAnalyzerForwardingModeGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorToAnalyzerForwardingModeGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with non-NULL modePtr.
    Expected: GT_OK.
    1.2. Call function with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with non-null modePtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorToAnalyzerForwardingModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorToAnalyzerForwardingModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorToAnalyzerForwardingModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorToAnalyzerForwardingModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with index [1 / 6], and enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet function.
    Expected: GT_OK and the same value.
    1.3. Call function with out of range index [7].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_BOOL   enable = GT_TRUE;
    GT_U32    index  = 0;
    GT_BOOL   enableGet;
    GT_U32    indexGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [1 / 6], and enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /*call with index [1] and enable [GT_TRUE]*/
        index = 1;
        enable = GT_TRUE;

        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet function.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                                    "get another index than was set: %d, %d", dev);


        /*call with index [6] and enable [GT_FALSE]*/
        index = 6;
        enable = GT_FALSE;

        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call function with out of range index [7].
            Expected: NOT GT_OK.
        */

        /*call with index [7]*/
        index = 7;

        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 1;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(dev, enable, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with bad enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call bad indexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_BOOL   enableGet;
    GT_U32    indexGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet: %d", dev);

        /*
            1.2. Call with bad enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(dev, NULL, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);

        /*
            1.3. Call with bad indexPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, indexPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(dev, &enableGet, &indexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerMirrorOnDropEnableSet
(
    IN  GT_U8   dev,
    IN  GT_U32  index,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerMirrorOnDropEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with index [0 - 6]
                            and enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call function cpssDxChMirrorAnalyzerMirrorOnDropEnableGet
         with the same params.
    Expected: GT_OK and same values as written.
    1.3. Call function with out of range index [7], enable [GT_TRUE].
    Expected: NOT GT_OK.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_U32                  index = 0;
    GT_BOOL                 state    = GT_FALSE;
    GT_BOOL                 stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [0 - 6]
                                    and enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        state = GT_FALSE;

        for (index = 0; index < 7; index++)
        {
            st = cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(dev, index, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, state);

            /*
                1.2. Call function cpssDxChMirrorAnalyzerMirrorOnDropEnableGet
                     with the same params.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(dev, index, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
               "cpssDxChMirrorAnalyzerMirrorOnDropEnableGet: %d, %d", dev, index);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
               "get another enable than was set: %d, %d", dev, index);
        }

        /*
            1.3. Call function with out of range index [7], enable [GT_TRUE].
            Expected: NOT GT_OK.
        */
        index = 7;

        st = cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(dev, index, state);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(dev, index, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(dev, index, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerMirrorOnDropEnableGet
(
    IN  GT_U8    dev,
    IN  GT_U32   index,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerMirrorOnDropEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with index [0], non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with index [7], non-NULL enablePtr.
    Expected: NOT GT_OK.
    1.3. Call function with index [0], enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_U32                  index = 0;
    GT_BOOL                 state    = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [0], non-NULL enablePtr.
            Expected: GT_OK.
        */
        index = 0;

        st = cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(dev, index, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call function with wrong index [7], non-NULL enablePtr.
            Expected: NOT GT_OK.
        */
        index = 7;

        st = cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(dev, index, &state);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 0;

        /*
            1.3. Call function with index [0], enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(dev, index, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(dev, index, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet
(
    IN GT_U8     devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1.1. Call with enable [GT_TRUE/ GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet with  non-NULL enablePtr.
    Expected: GT_OK  and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE/ GT_FALSE].
                Expected: GT_OK.
            */

            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet with  non-NULL enablePtr.
                Expected: GT_OK  and the same enable.
            */
            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable value than was set: %d, %d", dev, port);

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet with  non-NULL enablePtr.
                Expected: GT_OK  and the same enable.
            */
            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable value than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_TRUE;
    port   = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet
(
    IN GT_U8      devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL   *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorEnhancedMirroringPriorityModeGet
(
    IN GT_U8                                         devNum,
    OUT CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorEnhancedMirroringPriorityModeGet)
{
/*
    ITERATE_DEVICES (xCat_A3, xCat3)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with bad modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;

    CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT   mode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                    UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                    UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* supported only for xCAt_A3 & xCat3*/
        if (GT_TRUE == PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            /*
                1.1. Call with not null pointers.
                Expected: GT_OK.
            */
            st = cpssDxChMirrorEnhancedMirroringPriorityModeGet(dev, &mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call with bad modePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChMirrorEnhancedMirroringPriorityModeGet(dev, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
        }
        else
        {
            st = cpssDxChMirrorEnhancedMirroringPriorityModeGet(dev, &mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                    UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                    UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorEnhancedMirroringPriorityModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorEnhancedMirroringPriorityModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorEnhancedMirroringPriorityModeSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorEnhancedMirroringPriorityModeSet)
{
/*
    ITERATE_DEVICES (xCat_A3, xCat3)
    1.1. Call function with mode
         [CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_TDM_E /
          CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E ].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorEnhancedMirroringPriorityModeGet with non-NULL modePtr.
    Expected: GT_OK and same value as written.
    1.3. Call with wrong enum values mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;

    CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT    mode    = 0;
    CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT    modeGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                    UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                    UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* supported only for xCAt_A3 & xCat3 */
        if (GT_TRUE == PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            /*
                1.1. Call function with mode
                     [CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_TDM_E /
                      CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E].
                Expected: GT_OK.
            */

            /* Call function with mode [CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT] */
            mode = CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_TDM_E;

            st = cpssDxChMirrorEnhancedMirroringPriorityModeSet(dev, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

            /*
                1.2. Call cpssDxChMirrorEnhancedMirroringPriorityModeGet with non-NULL modePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorEnhancedMirroringPriorityModeGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChMirrorEnhancedMirroringPriorityModeGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                         "get another mode than was set: %d", dev);

            /* Call function with mode [CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E] */
            mode = CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E;

            st = cpssDxChMirrorEnhancedMirroringPriorityModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChMirrorEnhancedMirroringPriorityModeGet with non-NULL modePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorEnhancedMirroringPriorityModeGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChMirrorEnhancedMirroringPriorityModeGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                         "get another mode than was set: %d", dev);

            /*
                1.3. Call with wrong enum values mode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMirrorEnhancedMirroringPriorityModeSet
                                (dev, mode),
                                mode);
        }
        else
        {
            st = cpssDxChMirrorEnhancedMirroringPriorityModeSet(dev, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, mode);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                    UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                    UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorEnhancedMirroringPriorityModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorEnhancedMirroringPriorityModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode
);
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;

    CPSS_NET_RX_CPU_CODE_ENT    dropCode;
    CPSS_NET_RX_CPU_CODE_ENT    dropCodeGet;

    CPSS_NET_RX_CPU_CODE_ENT testedDropCodesArr[] =
    {
        CPSS_NET_ETH_BRIDGED_LLT_E,
        CPSS_NET_IPV4_TT_OPTION_FRAG_ERROR_E,
        CPSS_NET_ETH_BRIDGED_LLT_E,
        CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 100,
        CPSS_NET_BAD_ANALYZER_INDEX_DROP_ERROR_E,
        CPSS_NET_LAST_USER_DEFINED_E,

        CPSS_NET_IPV4_6_SIP_FILTERING_E                        ,
        CPSS_NET_IPV4_6_SIP_IS_ZERO_E                          ,
        CPSS_NET_ACCESS_MATRIX_E                               ,
        CPSS_NET_FCOE_DIP_LOOKUP_NOT_FOUND_OR_FCOE_EXCEPTION_E ,
        CPSS_NET_FCOE_SIP_NOT_FOUND_E                          ,

        CPSS_NET_IP_MC_ROUTE_BIDIR_RPF_FAIL_E                  ,
        CPSS_NET_MET_RED_DROP_CODE_E                           ,
        CPSS_NET_ECC_DROP_CODE_E                               ,

        CPSS_NET_RXDMA_DROP_E                                  ,
        CPSS_NET_TARGET_NULL_PORT_E                            ,
        CPSS_NET_TARGET_EPORT_MTU_EXCEEDED_OR_RX_PROTECTION_SWITCHING_OR_INGRESS_FORWARDING_RESTRICTIONS_E,

        CPSS_NET_UNTAGGED_OVERSIZE_DROP_CODE_E,
        CPSS_NET_IPV6_TT_UNSUP_GRE_ERROR_E,
        CPSS_NET_CCFC_PKT_DROP_E,
        CPSS_NET_BAD_FC_PKT_DROP_E,

        0xFFFFFFFF/* must be last*/
    };
    GT_U32  ii;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,(~UTF_CPSS_PP_E_ARCH_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* loop some 'choosen' values that suppose to be ok. */
        for(ii = 0 ; testedDropCodesArr[ii] != 0xFFFFFFFF ;ii++)
        {
            dropCode = testedDropCodesArr[ii];

            st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet(dev, dropCode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCode);

            /*
                1.2. Call cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet with non-NULL modePtr.
                Expected: GT_OK and same value as written.
            */
            st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet(dev, &dropCodeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet: %d", dev);

            UTF_VERIFY_EQUAL2_STRING_MAC(dropCode, dropCodeGet,
                                         "get another mode than was set: %d iteration %d",
                                         dev,ii);
        }

        dropCode = testedDropCodesArr[0];
        /*
            1.3. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet(dev, dropCode),
                            dropCode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,(~UTF_CPSS_PP_E_ARCH_CNS));

    dropCode = testedDropCodesArr[0];

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet(dev, dropCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet(dev, dropCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;

    CPSS_NET_RX_CPU_CODE_ENT    dropCodeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,(~UTF_CPSS_PP_E_ARCH_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet(dev, &dropCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with bad modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,(~UTF_CPSS_PP_E_ARCH_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet(dev, &dropCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet(dev, &dropCodeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxPortVlanEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxPortVlanEnableSet)
{
    GT_STATUS st  = GT_OK;
    GT_U8                   dev = 0;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL                 enableArr[2] = {GT_TRUE , GT_FALSE};
    GT_BOOL                 enable,enableGet;
    GT_U32                  ii;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, (~UTF_CPSS_PP_E_ARCH_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for(ii = 0 ; ii < 2 ; ii++)
            {
                enable = enableArr[ii];
                st = cpssDxChMirrorTxPortVlanEnableSet(dev,port,enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st,dev,port);

                st = cpssDxChMirrorTxPortVlanEnableGet(dev,port,&enableGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st,dev,port);

                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                             "get another mode than was set:dev  %d port %d",
                                             dev,port);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all NOT available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            for(ii = 0 ; ii < 2 ; ii++)
            {
                enable = enableArr[ii];
                st = cpssDxChMirrorTxPortVlanEnableSet(dev,port,enable);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st,dev,port);

                st = cpssDxChMirrorTxPortVlanEnableGet(dev,port,&enableGet);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st,dev,port);
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,(~UTF_CPSS_PP_E_ARCH_CNS));

    enable = GT_TRUE;
    port = 0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChMirrorTxPortVlanEnableSet(dev, port,enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
        }
    }

    port = 0;

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxPortVlanEnableSet(dev, port,enable);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, port,enable);


}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxPortVlanEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxPortVlanEnableGet)
{
    GT_STATUS st  = GT_OK;
    GT_U8                   dev = 0;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL                 enableGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, (~UTF_CPSS_PP_E_ARCH_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChMirrorTxPortVlanEnableGet(dev,port,&enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st,dev,port);

            st = cpssDxChMirrorTxPortVlanEnableGet(dev,port,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st,dev,port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all NOT available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChMirrorTxPortVlanEnableGet(dev,port,NULL);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st,dev,port);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,(~UTF_CPSS_PP_E_ARCH_CNS));

    port = 0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChMirrorTxPortVlanEnableGet(dev, port,&enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
        }
    }

    port = 0;

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxPortVlanEnableGet(dev, port,&enableGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);


}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    IN GT_BOOL                          enable,
    IN GT_U32                           index
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with index [1 / 6], egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E] and enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet function.
    Expected: GT_OK and the same value.
    1.3. Call function with out of range index [7].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;
    GT_BOOL   enable = GT_TRUE;
    GT_U32    index  = 0;
    GT_BOOL   enableGet;
    GT_U32    indexGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [1 / 6], egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E] and enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /*call with index [1], egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E] and enable [GT_TRUE]*/        
        index = 1;
        enable = GT_TRUE;

        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet(dev, egressMirroringMode, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet function.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(dev, egressMirroringMode, &enableGet, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                                    "get another index than was set: %d, %d", dev);


        /*call with index [6], egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E] and enable [GT_FALSE]*/
        index = 6;
        enable = GT_FALSE;

        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet(dev, egressMirroringMode, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call function with out of range index [7].
            Expected: NOT GT_OK.
        */

        /*call with index [7]*/
        index = 7;

        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet(dev, egressMirroringMode, enable, index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 1;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet(dev, egressMirroringMode, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet(dev, egressMirroringMode, enable, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    OUT GT_BOOL                         *enablePtr,
    OUT GT_U32                          *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with bad enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call bad indexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;
    GT_BOOL   enableGet;
    GT_U32    indexGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(dev, egressMirroringMode, &enableGet, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet: %d", dev);

        /*
            1.2. Call with bad enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(dev, egressMirroringMode, NULL, &indexGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);

        /*
            1.3. Call with bad indexPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(dev, egressMirroringMode, &enableGet, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, indexPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(dev, egressMirroringMode, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(dev, egressMirroringMode, &enableGet, &indexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_BOOL  enable,
    IN GT_U32   ratio
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet)
{
/*
    ITERATE_DEVICES Falcon)
    1.1. Call function with index [1 / 6], enable [GT_FALSE / GT_TRUE] and ratio [1].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet function.
    Expected: GT_OK and the same value.
    1.3. Call function with out of range index [7].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_BOOL   enable = GT_TRUE;
    GT_U32    index = 0;
    GT_U32    ratio = 1;
    GT_BOOL   enableGet;
    GT_U32    ratioGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [1 / 6], enable [GT_FALSE / GT_TRUE] and ratio [1].
            Expected: GT_OK.
        */

        /*call with index [1], enable [GT_TRUE] and ratio [1]*/
        index = 1;
        enable = GT_TRUE;

        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev, index, enable, ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet function.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, index, &enableGet, &ratioGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ratio, ratioGet,
                                    "get another ratio than was set: %d, %d", dev);


        /*call with index [6], enable [GT_FALSE] and ratio [1]*/
        index = 6;
        enable = GT_FALSE;

        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev, index, enable, ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call function with out of range index [7].
            Expected: NOT GT_OK.
        */

        /*call with index [7]*/
        index = 7;

        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev, index, enable, ratio);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 1;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev, index, enable, ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev, index, enable, ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *ratioPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with bad enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call bad ratioPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_U32    index  = 0;
    GT_BOOL   enableGet;
    GT_U32    ratioGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, index, &enableGet, &ratioGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet: %d", dev);

        /*
            1.2. Call with bad enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, index, NULL, &ratioGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);

        /*
            1.3. Call with bad ratioPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, index, &enableGet, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ratioPtr = NULL", dev);

        /*
            1.3. Call with bad index.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, 8, &enableGet, &ratioGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, index 8 is bad", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, index, &enableGet, &ratioGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, index, &enableGet, &ratioGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorToAnalyzerTruncateSet
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_BOOL  truncate
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorToAnalyzerTruncateSet)
{
/*
    ITERATE_DEVICES Falcon)
    1.1. Call function with index [1 / 6], truncate [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorToAnalyzerTruncateGet function.
    Expected: GT_OK and the same value.
    1.3. Call function with out of range index [7].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_BOOL   truncate = GT_TRUE;
    GT_U32    index = 1;    
    GT_BOOL   truncateGet;    

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [1 / 6], truncate [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /*call with index [1] and truncate [GT_TRUE]*/
        index = 1;
        truncate = GT_TRUE;

        st = cpssDxChMirrorToAnalyzerTruncateSet(dev, index, truncate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call internal_cpssDxChMirrorToAnalyzerTruncateGet function.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChMirrorToAnalyzerTruncateGet(dev, index, &truncateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorToAnalyzerTruncateGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(truncate, truncateGet,
                                   "get another enable than was set: %d, %d", dev);        


        /*call with index [6] and truncate [GT_FALSE]*/
        index = 6;
        truncate = GT_FALSE;

        st = cpssDxChMirrorToAnalyzerTruncateSet(dev, index, truncate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call function with out of range index [7].
            Expected: NOT GT_OK.
        */

        /*call with index [7]*/
        index = 7;

        st = cpssDxChMirrorToAnalyzerTruncateSet(dev, index, truncate);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 1;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorToAnalyzerTruncateSet(dev, index, truncate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorToAnalyzerTruncateSet(dev, index, truncate);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorToAnalyzerTruncateGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    index,
    OUT GT_BOOL   *truncatePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorToAnalyzerTruncateGet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with bad truncatePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call bad index [8].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_U32    index  = 0;
    GT_BOOL   truncateGet;    

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorToAnalyzerTruncateGet(dev, index, &truncateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChMirrorToAnalyzerTruncateGet: %d", dev);

        /*
            1.2. Call with bad enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorToAnalyzerTruncateGet(dev, index, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, truncatePtr = NULL", dev);

        /*
            1.3. Call with bad index [8].
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMirrorToAnalyzerTruncateGet(dev, 8, &truncateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, index 8 is bad", dev);        
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorToAnalyzerTruncateGet(dev, index, &truncateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorToAnalyzerTruncateGet(dev, index, &truncateGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxPortModeSet
(
    IN  GT_U8                           dev,
    IN  GT_PORT_NUM                     mirrPort,
    IN  GT_BOOL                         isPhysicalPort,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxPortModeSet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with mirrPort [from 0 to numOfVirtPorts(dev) -1]
                            and enable [GT_FALSE] - to be sure that we don't have mirrored ports.
    Expected: GT_OK.
    1.2. Call function cpssDxChMirrorTxPortModeGet mirrPort [from 0 to numOfVirtPorts(dev) -1] and non-NULL enablePtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with mirrPort [from 0 to PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2]
                            and enable [GT_TRUE].
    Expected: GT_OK.
    1.4. Call function with out of range mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                            enable [GT_TRUE].
    Expected: NOT GT_OK.
    1.5. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS],
                            enable [GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             mirrPort = 0;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringModeGet;
    
    GT_U32      validPorts[PRV_CPSS_MIRROR_MAX_PORTS_CNS - 1];
    GT_U32      ii;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&mirrPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For each active device (devNum) id go over all active virtual portNums. */
        while (GT_OK == prvUtfNextVirtPortGet(&mirrPort, GT_TRUE))
        {
            /*
                1.1. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E].
                Expected: GT_OK.
            */
            egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;

            st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_FALSE, egressMirroringMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, egressMirroringMode);

            /*
                1.2. Call function cpssDxChMirrorTxPortModeGet mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS - 1] and non-NULL egressMirroringModePtr.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_FALSE, &egressMirroringModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortModeGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressMirroringMode, egressMirroringModeGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);            
        }

        st = prvUtfNextTxqPortReset(&mirrPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For each active device (devNum) id go over all active physical portNums. */
        while (GT_OK == prvUtfNextTxqPortGet(&mirrPort, GT_TRUE))
        {
            /*
                1.1. Call function with mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS-1]
                                        and egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E].
                Expected: GT_OK.
            */
           egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E;

            st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_TRUE, egressMirroringMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, egressMirroringMode);

            /*
                1.2. Call function cpssDxChMirrorTxPortGet mirrPort [from 0 to UTF_CPSS_PP_MAX_PORTS_NUM_CNS - 1] and non-NULL egressMirroringModePtr.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortModeGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressMirroringMode, egressMirroringModeGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);                        
        }

        /*
            1.3. Call function with mirrPort [from 0 to PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2]
                                    and egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E].
            Expected: GT_OK.
        */
        egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E;

        /* get ports with TXQ */
        prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_TXQ_E,
            0  /*start port */, &validPorts[0] , PRV_CPSS_MIRROR_MAX_PORTS_CNS - 1);


        for (ii = 0; ii <= PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2; ii++)
        {
            mirrPort = validPorts[ii];
            st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_TRUE, egressMirroringMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, egressMirroringMode);

            /* 1.2. for mirrPort [from 0 to PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2] and egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E]. */
            st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorTxPortModeGet: %d, %d", dev, mirrPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressMirroringMode, egressMirroringModeGet,
                                     "get another enable than was set: %d, %d", dev, mirrPort);            
        }

        /*
            1.4. Call function with out of range mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                                    egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E].
            Expected: NOT GT_OK.
        */
        mirrPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E;

        st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_TRUE, egressMirroringMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.5. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS],
                                    egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E].
            Expected: GT_OK.
        */
        mirrPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_TRUE, egressMirroringMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);
        
        st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringModeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                 "cpssDxChMirrorTxPortModeGet: %d, %d", dev, mirrPort);
        UTF_VERIFY_EQUAL2_STRING_MAC(egressMirroringMode, egressMirroringModeGet,
                                 "get another enable than was set: %d, %d", dev, mirrPort);

        /* to clear mirrored ports */
        egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E;
        for (ii = 0; ii <= PRV_CPSS_MIRROR_MAX_PORTS_CNS - 2; ii++)
        {
            mirrPort = validPorts[ii];
            st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_TRUE, egressMirroringMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mirrPort, egressMirroringMode);
        }
    }
    
    mirrPort = 0;
    egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE)) 
    {
        st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_TRUE, egressMirroringMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxPortModeSet(dev, mirrPort, GT_TRUE, egressMirroringMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorTxPortModeGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      mirrPort,
    IN  GT_BOOL                          isPhysicalPort,
    OUT CPSS_DXCH_MIRROR_EGRESS_MODE_ENT *egressMirroringModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorTxPortModeGet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with mirrPort [0],
                            non-NULL egressMirroringModePtr.
    Expected: GT_OK.
    1.2. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                            non-NULL egressMirroringModePtr.
    Expected: NOT GT_OK.
    1.3. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS]
                            and non-null egressMirroringModePtr.
    Expected: GT_OK.
    1.4. Call function with mirrPort [0],
                            egressMirroringModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st       = GT_OK;

    GT_U8                   dev;
    GT_PORT_NUM             mirrPort = 0;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mirrPort [0],
                                    non-NULL egressMirroringModePtr.
            Expected: GT_OK.
        */
        mirrPort = 0;

        st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.2. Call function with mirrPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS = 64],
                                    non-NULL egressMirroringModePtr.
            Expected: NOT GT_OK.
        */
        mirrPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.3. Call function with mirrPort = CPU port [CPSS_CPU_PORT_NUM_CNS]
                                    and non-null egressMirroringModePtr.
            Expected: GT_OK.
        */
        mirrPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mirrPort);

        /*
            1.4. Call function with mirrPort [0],
                                    egressMirroringModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        mirrPort = 0;

        st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, egressMirroringModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorTxPortModeGet(dev, mirrPort, GT_TRUE, &egressMirroringMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorOnCongestionModeSet
(
    IN  GT_U8   dev,
    IN  CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT congestionMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorOnCongestionModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with congestion mode as [CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E /
                                                CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_ECN_E].
    Expected: GT_OK.
    1.2. Call cpssDxChMirrorOnCongestionModeGet with non-NULL congestionModePtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT   congestionMode    = CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E;
    CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT   congestionModeGet = CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ((UTF_CPSS_PP_E_ARCH_CNS & ~UTF_CPSS_PP_ALL_SIP6_CNS) | UTF_LION2_E | UTF_XCAT3_E));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            Call function with congestion mode [CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E /
                                                     CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E].
            Expected: GT_OK.
        */

        /*  1.1. Call function with congestion mode [CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E] */
        congestionMode = CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E;

        st = cpssDxChMirrorOnCongestionModeSet(dev, congestionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, congestionMode);

        /*
            Call cpssDxChMirrorOnCongestionModeGet with non-NULL congestionModeGetPtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrorOnCongestionModeGet(dev, &congestionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorOnCongestionModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(congestionMode, congestionModeGet,
                                     "get another congestion mode than was set: %d", dev);

        /*  1.2. Call function with congestion mode [CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E] */
        congestionMode = CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E;

        st = cpssDxChMirrorOnCongestionModeSet(dev, congestionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, congestionMode);

        /*
            Call cpssDxChMirrorOnCongestionModeGet with non-NULL congestionModeGetPtr.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChMirrorOnCongestionModeGet(dev, &congestionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChMirrorOnCongestionModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(congestionMode, congestionModeGet,
                                     "get another congestion mode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values congestionMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMirrorOnCongestionModeSet
                            (dev, congestionMode),
                            congestionMode);


    }

    congestionMode = CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ((UTF_CPSS_PP_E_ARCH_CNS & ~UTF_CPSS_PP_ALL_SIP6_CNS) | UTF_LION2_E | UTF_XCAT3_E));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorOnCongestionModeSet(dev, congestionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorOnCongestionModeSet(dev, congestionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMirrorOnCongestionModeGet
(
    IN   GT_U8   dev,
    OUT  CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT *congestionModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMirrorOnCongestionModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL congestionModePtr.
    Expected: GT_OK.
    1.2. Call function with congestionModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT   congestionMode = CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ((UTF_CPSS_PP_E_ARCH_CNS & ~UTF_CPSS_PP_ALL_SIP6_CNS) | UTF_LION2_E | UTF_XCAT3_E));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with non-null congestionModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChMirrorOnCongestionModeGet(dev, &congestionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with congestionModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMirrorOnCongestionModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, congestionModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ((UTF_CPSS_PP_E_ARCH_CNS & ~UTF_CPSS_PP_ALL_SIP6_CNS) | UTF_LION2_E | UTF_XCAT3_E));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMirrorOnCongestionModeGet(dev, &congestionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMirrorOnCongestionModeGet(dev, &congestionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChMirror suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChMirror)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerVlanTagEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerVlanTagEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxAnalyzerVlanTagConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxAnalyzerVlanTagConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxAnalyzerVlanTagConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxAnalyzerVlanTagConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxAnalyzerPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxAnalyzerPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxAnalyzerPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxAnalyzerPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxCascadeMonitorEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxCascadeMonitorEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxPortModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxPortModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrRxStatMirroringToAnalyzerRatioSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrRxStatMirroringToAnalyzerRatioGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrRxStatMirrorToAnalyzerEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrRxStatMirrorToAnalyzerEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorToAnalyzerTruncateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorToAnalyzerTruncateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrRxAnalyzerDpTcSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrRxAnalyzerDpTcGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrTxAnalyzerDpTcSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrTxAnalyzerDpTcGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrTxStatMirroringToAnalyzerRatioSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrTxStatMirroringToAnalyzerRatioGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrTxStatMirrorToAnalyzerEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrTxStatMirrorToAnalyzerEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerInterfaceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerInterfaceGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorToAnalyzerForwardingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorToAnalyzerForwardingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerMirrorOnDropEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerMirrorOnDropEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorEnhancedMirroringPriorityModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorEnhancedMirroringPriorityModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxPortVlanEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorTxPortVlanEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorOnCongestionModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMirrorOnCongestionModeGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChMirror)


