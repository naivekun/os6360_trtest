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
* @file cpssPxPortEcnUT.c
*
* @brief Unit tests for cpssPxPortEcn, that provides
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

#include <cpss/px/port/cpssPxPortEcn.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/* 
GT_STATUS cpssPxPortEcnMarkingEnableSet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN GT_BOOL                             enable
);
*/
UTF_TEST_CASE_MAC(cpssPxPortEcnMarkingEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Pxx)
    1.1.1. Call with state [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    1.1.2. Call cpssPxPortEcnMarkingEnableSet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
        */

        /* Call function with enable = GT_FALSE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        enable = GT_FALSE;

        st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssPxPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssPxPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssPxPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssPxPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);



        /* Call function with enable = GT_FALSE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        enable = GT_FALSE;

        st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssPxPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssPxPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssPxPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssPxPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_FALSE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        enable = GT_FALSE;

        st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssPxPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortEcnMarkingEnableGet(dev, CPSS_IP_PROTOCOL_IPV6_E, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssPxPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocolStack, enable);

        /* 1.1.2. Call cpssPxPortEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortEcnMarkingEnableGet(dev, CPSS_IP_PROTOCOL_IPV6_E, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "[cpssPxPortEcnMarkingEnableGet]: %d, %d", dev, protocolStack);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                     "get another enable value than was set: %d, %d", dev, protocolStack);


        UTF_ENUMS_CHECK_MAC(cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable),protocolStack);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortEcnMarkingEnableSet(dev, protocolStack, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/* 
GT_STATUS cpssPxPortEcnMarkingEnableGet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    OUT GT_BOOL                            *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxPortEcnMarkingEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Pxx)
    1.1.1. Call with state [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    1.1.2. Call cpssPxPortEcnMarkingEnableSet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with stack [IPv4, IPv6, IPv46].
           Expected: GT_OK.
        */

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, protocolStack);

        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, protocolStack);

        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, protocolStack);

        UTF_ENUMS_CHECK_MAC(cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enable),protocolStack);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortEcnMarkingEnableGet(dev, protocolStack, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEcnMarkingTailDropProfileEnableSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   tdProfile,
    IN  CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEcnMarkingTailDropProfileEnableSet)
{
/*
    ITERATE_DEVICES(SIP5)
    1.1. Call with TD profile[0 /1 /2 /3]
           enablers.tcDpLimit       [GT_TRUE / GT_FALSE],
           enablers.portLimit       [GT_TRUE / GT_FALSE],
           enablers.tcLimit         [GT_TRUE / GT_FALSE],
           enablers.sharedPoolLimit [GT_TRUE / GT_FALSE],

    Expected: GT_OK.
    1.2. Call cpssPxPortEcnMarkingTailDropProfileEnableGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong pfSet [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.4. Call api with wrong enablers [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_BOOL   isEqual = GT_FALSE;

    GT_U8                                       dev;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_ECN_ENABLERS_STC               enablers;
    CPSS_PX_PORT_ECN_ENABLERS_STC               enablersGet;

    cpssOsBzero((GT_VOID*) &enablers, sizeof(enablers));
    cpssOsBzero((GT_VOID*) &enablersGet, sizeof(enablersGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with TD profile[0 /1 /2 /3]
                   enablers.tcDpLimit       [GT_TRUE / GT_FALSE],
                   enablers.portLimit       [GT_TRUE / GT_FALSE],
                   enablers.tcLimit         [GT_TRUE / GT_FALSE],
                   enablers.sharedPoolLimit [GT_TRUE / GT_FALSE],

                Expected: GT_OK.
        */
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        enablers.tcDpLimit       = GT_TRUE;
        enablers.portLimit       = GT_TRUE;
        enablers.tcLimit         = GT_TRUE;
        enablers.sharedPoolLimit = GT_TRUE;

        st = cpssPxPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(st == GT_OK)
        {
            /*
                1.2. Call cpssPxPortEcnMarkingTailDropProfileEnableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablersGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssPxPortEcnMarkingTailDropProfileEnableGet: %d ", dev);

            /* Verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&enablers, (GT_VOID*)&enablersGet,
                                         sizeof (enablers))) ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another enablers than was set: %d", dev);
        }

        /* call with profile[1]
                       enablers.tcDpLimit       = GT_FALSE;
                       enablers.portLimit       = GT_FALSE;
                       enablers.tcLimit         = GT_FALSE;
                       enablers.sharedPoolLimit = GT_FALSE;*/
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_1_E;
        enablers.tcDpLimit       = GT_FALSE;
        enablers.portLimit       = GT_FALSE;
        enablers.tcLimit         = GT_FALSE;
        enablers.sharedPoolLimit = GT_FALSE;

        st = cpssPxPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(st == GT_OK)
        {
            /*
                1.2. Call cpssPxPortEcnMarkingTailDropProfileEnableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablersGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssPxPortEcnMarkingTailDropProfileEnableGet: %d ", dev);

            /* Verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&enablers, (GT_VOID*)&enablersGet,
                                         sizeof (enablers))) ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another enablers than was set: %d", dev);
        }

        /* call with profile[2]
                       enablers.tcDpLimit       = GT_FALSE;
                       enablers.portLimit       = GT_TRUE;
                       enablers.tcLimit         = GT_FALSE;
                       enablers.sharedPoolLimit = GT_TRUE;*/
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_2_E;
        enablers.tcDpLimit       = GT_FALSE;
        enablers.portLimit       = GT_TRUE;
        enablers.tcLimit         = GT_FALSE;
        enablers.sharedPoolLimit = GT_TRUE;

        st = cpssPxPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(st == GT_OK)
        {
            /*
                1.2. Call cpssPxPortEcnMarkingTailDropProfileEnableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablersGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssPxPortEcnMarkingTailDropProfileEnableGet: %d ", dev);

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
        UTF_ENUMS_CHECK_MAC(cpssPxPortEcnMarkingTailDropProfileEnableSet
                            (dev, profile, &enablers), profile);

        /*
            1.4. Call api with wrong enablersPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortEcnMarkingTailDropProfileEnableSet(dev, profile, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "enablersPtr is NULL", dev);
    }

    /* restore correct values */
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
    enablers.tcDpLimit = GT_TRUE;
    enablers.portLimit = GT_TRUE;
    enablers.tcLimit = GT_TRUE;
    enablers.sharedPoolLimit = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEcnMarkingTailDropProfileEnableSet(dev, profile, &enablers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEcnMarkingTailDropProfileEnableGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   tdProfile,
    OUT CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEcnMarkingTailDropProfileEnableGet)
{
/*
    ITERATE_DEVICES(SIP5)
    1.1. Call with TD profile[0..15]

    Expected: GT_OK.
    1.2. Call api with wrong profile [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.3. Call api with wrong enablers [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                       dev;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_ECN_ENABLERS_STC               enablers;

    cpssOsBzero((GT_VOID*) &enablers, sizeof(enablers));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with TD profile[0..15]

            Expected: GT_OK.
        */
        for(profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E; profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E; profile++)
        {
            st = cpssPxPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablers);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);
        }

        /*
            1.2. Call api with wrong pfSet [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortEcnMarkingTailDropProfileEnableGet
                            (dev, profile, &enablers), profile);

        /*
            1.3. Call api with wrong enablersPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortEcnMarkingTailDropProfileEnableGet(dev, profile, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "enablersPtr is NULL", dev);
    }

    /* restore correct values */
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEcnMarkingTailDropProfileEnableGet(dev, profile, &enablers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssPxPortEcn suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortEcn)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEcnMarkingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEcnMarkingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEcnMarkingTailDropProfileEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEcnMarkingTailDropProfileEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortEcn)


