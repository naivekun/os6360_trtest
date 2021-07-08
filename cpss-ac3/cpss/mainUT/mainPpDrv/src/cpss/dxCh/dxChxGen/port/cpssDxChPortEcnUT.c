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
* @file cpssDxChPortEcnUT.c
*
* @brief Unit tests for cpssDxChPortEcn, that provides
* CPSS implementation for Explicit Congestion Notification (ECN) Marking.
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/

#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortEcn.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/* 
GT_STATUS cpssDxChPortEcnMarkingEnableSet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN GT_BOOL                             enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEcnMarkingEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with state [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    1.1.2. Call cpssDxChPortEcnMarkingEnableSet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
        */

        /* Call function with enable = GT_FALSE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        enable = GT_FALSE;

        st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssDxChPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssDxChPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssDxChPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssDxChPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);



        /* Call function with enable = GT_FALSE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        enable = GT_FALSE;

        st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssDxChPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssDxChPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssDxChPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssDxChPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_FALSE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        enable = GT_FALSE;

        st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssDxChPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortEcnMarkingEnableGet(dev, CPSS_IP_PROTOCOL_IPV6_E, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssDxChPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssDxChPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortEcnMarkingEnableGet(dev, CPSS_IP_PROTOCOL_IPV6_E, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssDxChPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        UTF_ENUMS_CHECK_MAC(cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable),protocolStack);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEcnMarkingEnableSet(dev, protocolStack, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/* 
GT_STATUS cpssDxChPortEcnMarkingEnableGet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    OUT GT_BOOL                            *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortEcnMarkingEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with state [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    1.1.2. Call cpssDxChPortEcnMarkingEnableSet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with stack [IPv4, IPv6, IPv46].
           Expected: GT_OK.
        */

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, protocolStack);

        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, protocolStack);

        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, protocolStack);

        UTF_ENUMS_CHECK_MAC(cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enable),protocolStack);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortEcnMarkingEnableGet(dev, protocolStack, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEcnMarkingTailDropProfileEnableSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT        tdProfile,
    IN  CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEcnMarkingTailDropProfileEnableSet)
{
/*
    ITERATE_DEVICES(SIP5)
    1.1. Call with TD profile[CPSS_PORT_TX_DROP_PROFILE_1_E /
                              CPSS_PORT_TX_DROP_PROFILE_2_E /
                              CPSS_PORT_TX_DROP_PROFILE_3_E /
                              CPSS_PORT_TX_DROP_PROFILE_4_E]
           enablers.tcDpLimit       [GT_TRUE / GT_FALSE],
           enablers.portLimit       [GT_TRUE / GT_FALSE],
           enablers.tcLimit         [GT_TRUE / GT_FALSE],
           enablers.sharedPoolLimit [GT_TRUE / GT_FALSE],

    Expected: GT_OK.
    1.2. Call cpssDxChPortEcnMarkingTailDropProfileEnableGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong pfSet [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.4. Call api with wrong enablers [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_BOOL   isEqual = GT_FALSE;

    GT_U8                                     dev;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT         profile = CPSS_PORT_TX_DROP_PROFILE_1_E;
    CPSS_DXCH_PORT_ECN_ENABLERS_STC           enablers;
    CPSS_DXCH_PORT_ECN_ENABLERS_STC           enablersGet;

    cpssOsBzero((GT_VOID*) &enablers, sizeof(enablers));
    cpssOsBzero((GT_VOID*) &enablersGet, sizeof(enablersGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with TD profile[CPSS_PORT_TX_DROP_PROFILE_1_E /
                                      CPSS_PORT_TX_DROP_PROFILE_2_E /
                                      CPSS_PORT_TX_DROP_PROFILE_3_E /
                                      CPSS_PORT_TX_DROP_PROFILE_4_E]
                    enablers.tcDpLimit       [GT_TRUE / GT_FALSE],
                    enablers.portLimit       [GT_TRUE / GT_FALSE],
                    enablers.tcLimit         [GT_TRUE / GT_FALSE],
                    enablers.sharedPoolLimit [GT_TRUE / GT_FALSE],

            Expected: GT_OK.
        */

        /* call with profile[CPSS_PORT_TX_DROP_PROFILE_1_E]
                       enablers.tcDpLimit       = GT_TRUE;
                       enablers.portLimit       = GT_TRUE;
                       enablers.tcLimit         = GT_TRUE;
                       enablers.sharedPoolLimit = GT_TRUE;*/
        profile = CPSS_PORT_TX_DROP_PROFILE_1_E;
        enablers.tcDpLimit       = GT_TRUE;
        enablers.portLimit       = GT_TRUE;
        enablers.tcLimit         = GT_TRUE;
        enablers.sharedPoolLimit = GT_TRUE;

        st = cpssDxChPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(st == GT_OK)
        {
            /*
                1.2. Call cpssDxChPortEcnMarkingTailDropProfileEnableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablersGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChPortEcnMarkingTailDropProfileEnableGet: %d ", dev);

            /* Verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&enablers, (GT_VOID*)&enablersGet,
                                         sizeof (enablers))) ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another enablers than was set: %d", dev);
        }

        /* call with profile[CPSS_PORT_TX_DROP_PROFILE_2_E]
                       enablers.tcDpLimit       = GT_FALSE;
                       enablers.portLimit       = GT_FALSE;
                       enablers.tcLimit         = GT_FALSE;
                       enablers.sharedPoolLimit = GT_FALSE;*/
        profile = CPSS_PORT_TX_DROP_PROFILE_2_E;
        enablers.tcDpLimit       = GT_FALSE;
        enablers.portLimit       = GT_FALSE;
        enablers.tcLimit         = GT_FALSE;
        enablers.sharedPoolLimit = GT_FALSE;

        st = cpssDxChPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(st == GT_OK)
        {
            /*
                1.2. Call cpssDxChPortEcnMarkingTailDropProfileEnableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablersGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChPortEcnMarkingTailDropProfileEnableGet: %d ", dev);

            /* Verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&enablers, (GT_VOID*)&enablersGet,
                                         sizeof (enablers))) ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another enablers than was set: %d", dev);
        }

        /* call with profile[CPSS_PORT_TX_DROP_PROFILE_3_E]
                       enablers.tcDpLimit       = GT_FALSE;
                       enablers.portLimit       = GT_TRUE;
                       enablers.tcLimit         = GT_FALSE;
                       enablers.sharedPoolLimit = GT_TRUE;*/
        profile = CPSS_PORT_TX_DROP_PROFILE_3_E;
        enablers.tcDpLimit       = GT_FALSE;
        enablers.portLimit       = GT_TRUE;
        enablers.tcLimit         = GT_FALSE;
        enablers.sharedPoolLimit = GT_TRUE;

        st = cpssDxChPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(st == GT_OK)
        {
            /*
                1.2. Call cpssDxChPortEcnMarkingTailDropProfileEnableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablersGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChPortEcnMarkingTailDropProfileEnableGet: %d ", dev);

            /* Verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&enablers, (GT_VOID*)&enablersGet,
                                         sizeof (enablers))) ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another enablers than was set: %d", dev);
        }

        /*
            1.3. Call api with wrong pfSet [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortEcnMarkingTailDropProfileEnableSet
                            (dev, profile, &enablers), profile);

        /*
            1.4. Call api with wrong enablersPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortEcnMarkingTailDropProfileEnableSet(dev, profile, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "enablersPtr is NULL", dev);
    }

    /* restore correct values */
    profile = CPSS_PORT_TX_DROP_PROFILE_1_E;
    enablers.tcDpLimit = GT_TRUE;
    enablers.portLimit = GT_TRUE;
    enablers.tcLimit = GT_TRUE;
    enablers.sharedPoolLimit = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEcnMarkingTailDropProfileEnableGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT        tdProfile,
    OUT CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEcnMarkingTailDropProfileEnableGet)
{
/*
    ITERATE_DEVICES(SIP5)
    1.1. Call with TD profile[CPSS_PORT_TX_DROP_PROFILE_1_E /
                              CPSS_PORT_TX_DROP_PROFILE_2_E /
                              CPSS_PORT_TX_DROP_PROFILE_3_E /
                              CPSS_PORT_TX_DROP_PROFILE_4_E]

    Expected: GT_OK.
    1.2. Call api with wrong profile [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.3. Call api with wrong enablers [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                     dev;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT         profile = CPSS_PORT_TX_DROP_PROFILE_1_E;
    CPSS_DXCH_PORT_ECN_ENABLERS_STC           enablers;

    cpssOsBzero((GT_VOID*) &enablers, sizeof(enablers));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with TD profile[CPSS_PORT_TX_DROP_PROFILE_1_E /
                                      CPSS_PORT_TX_DROP_PROFILE_2_E /
                                      CPSS_PORT_TX_DROP_PROFILE_3_E /
                                      CPSS_PORT_TX_DROP_PROFILE_4_E]

            Expected: GT_OK.
        */
        for(profile = CPSS_PORT_TX_DROP_PROFILE_1_E; profile <= CPSS_PORT_TX_DROP_PROFILE_16_E; profile++)
        {
            st = cpssDxChPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablers);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);
        }

        /*
            1.2. Call api with wrong pfSet [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortEcnMarkingTailDropProfileEnableGet
                            (dev, profile, &enablers), profile);

        /*
            1.3. Call api with wrong enablersPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortEcnMarkingTailDropProfileEnableGet(dev, profile, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "enablersPtr is NULL", dev);
    }

    /* restore correct values */
    profile = CPSS_PORT_TX_DROP_PROFILE_1_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E); 

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChPortMapping suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortEcn)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEcnMarkingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEcnMarkingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEcnMarkingTailDropProfileEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEcnMarkingTailDropProfileEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortEcn)


