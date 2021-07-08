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
* @file tgfCommonMirrorUT.c
*
* @brief Enhanced UTs for CPSS Mirror
*
* @version   18
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <mirror/prvTgfMirror.h>
#include <common/tgfCscdGen.h>
/*
parameter relevant to next tests:
    prvTgfIngressMirrorHopByHop
    prvTgfIngressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer
    prvTgfIngressMirrorSourceBased
    prvTgfIngressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer
    prvTgfIngressMirrorEndToEnd
    prvTgfIngressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer
    prvTgfEgressMirrorHopByHop
    prvTgfEgressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer
    prvTgfEgressMirrorSourceBased
    prvTgfEgressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer
    prvTgfEgressMirrorEndToEnd
    prvTgfEgressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer
*/
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
static GT_BOOL                  currentDsaTagEnable = GT_FALSE;
static TGF_DSA_TYPE_ENT         currentDsaType = TGF_DSA_2_WORD_TYPE_E;

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress mirroring functionality in hop-by-hop mode:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Rx mirroring for HopByHop mode;
    send BC traffic and verify expected mirrored traffic on analyzer port;
    disable global Rx mirroring;
    send BC traffic and verify expected original flood traffic on all ports;
    enable global Rx mirroring;
    verify that if a packet is mirrored by both the port-based ingress mirroring
    mechanism, and one of the other ingress mirroring mechanisms,
    the selected analyzer is the one with the higher index.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorHopByHop)
{
/********************************************************************
    Test 8.1 - Ingress mirroring functionality in hop-by-hop mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is oriented to hop-by-hop */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
    }

    /* Set configuration */
    prvTgfIngressMirrorHopByHopConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorHopByHopTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorHopByHopConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorHopByHop , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to hop-by-hop */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfIngressMirrorHopByHop */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorHopByHop);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfIngressMirrorHopByHop */
        UTF_TEST_CALL_MAC(prvTgfIngressMirrorHopByHop);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress mirroring functionality in source-based mode:
    configure VLAN, FDB entries;
    configure Source-Based forwarding mode for mirroring to analyzer;
    configure 2 analyzer interfaces and enable Rx mirroring;
    send BC traffic to each configured Rx analyzer port;
        verify expected mirrored traffic on analyzer port and flood traffic on all ports;
        disable Rx mirroring;
    send BC traffic to each configured Rx analyzer port;
    verify expected original flood traffic on all ports.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorSourceBased)
{
/********************************************************************
    Test 8.2 - Ingress mirroring functionality in source-based mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfIngressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorSourceBasedConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorSourceBased , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to 'source based' mirroring - related to ingress processing of DSA fields */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfIngressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorSourceBased);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfIngressMirrorSourceBased */
        UTF_TEST_CALL_MAC(prvTgfIngressMirrorSourceBased);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

}

/*----------------------------------------------------------------------------*/
/*
    prvTgfIngressMirrorEndToEnd
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorEndToEnd)
{
/********************************************************************
    Ingress mirroring functionality in End-To-End mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                                      UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable, currentDsaType, currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfIngressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorSourceBasedConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorEndToEnd , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                                      UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
#if 0  /* 2 words  DSA not hold parameters of this mode */
    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfIngressMirrorEndToEnd */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorEndToEnd);
#endif /*0*/
    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfIngressMirrorEndToEnd */
        UTF_TEST_CALL_MAC(prvTgfIngressMirrorEndToEnd);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Port source-based mirroring and other ingress mirroring mechanisms:
    configure VLAN, FDB entries;
    configure Source-Based forwarding mode for mirroring to analyzer;
    configure 2 analyzer interfaces and enable Rx mirroring;
    send BC traffic to each configured Rx analyzer port;
        verify expected mirrored traffic on analyzer port and flood traffic on all ports;
        set analyzer interface with lower index and enable Rx mirroring;
        send BC traffic to Rx analyzer port;
    verify expected mirrored traffic on analyzer with higher index.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorHighestIndexSelection)
{
/********************************************************************
    Test 8.3 - Port source-based mirroring and other ingress mirroring mechanisms.
               (Selected analyzer is the one with the higher index.)

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    CPSS_TBD_BOOKMARK_FALCON
    /* the falcon not supports mirroring from FDB entry ..
       so test need to change to use other mirroring cause */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E);


    /* Set configuration */
    prvTgfIngressMirrorHighestIndexSelectionConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorHighestIndexSelectionTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorHighestIndexSelectionConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test egress mirroring functionality in hop-by-hop mode:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Tx mirroring for HopByHop mode;
    send BC traffic and verify expected mirrored traffic on analyzer port;
    disable global Tx mirroring;
    send BC traffic and verify expected original flood traffic on all ports;
    enable Tx mirroring on each analyzer;
    send BC traffic to each enabled Tx analyzer port;
        verify expected mirrored traffic on analyzer port and flood traffic on all ports.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorHopByHop)
{
/********************************************************************
    Test 8.4 - Egress mirroring functionality in hop-by-hop mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is oriented to 'hop-by-hop' mirroring */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);


    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
    }

    /* Set configuration */
    prvTgfEgressMirrorHopByHopConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorHopByHopTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorHopByHopConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorHopByHop , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to 'hop-by-hop' mirroring */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfEgressMirrorHopByHop */
    UTF_TEST_CALL_MAC(prvTgfEgressMirrorHopByHop);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfEgressMirrorHopByHop */
        UTF_TEST_CALL_MAC(prvTgfEgressMirrorHopByHop);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test egress mirroring functionality in source-based mode:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and set Source-Based forwarding mode;
    send traffic and verify expected mirrored traffic on analyzer ports;
    disable Tx mirroring on analyzer ports;
    send traffic and verify no traffic on analyzer ports.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorSourceBased)
{
/********************************************************************
    Test 8.5 - Egress mirroring functionality in source-based mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfEgressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorSourceBasedConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorSourceBased , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to 'src-based' mirroring - behavior of ingress pipe for DSA fields */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfEgressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfEgressMirrorSourceBased);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfEgressMirrorSourceBased */
        UTF_TEST_CALL_MAC(prvTgfEgressMirrorSourceBased);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*
    prvTgfEgressMirrorEndToEnd
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorEndToEnd)
{
/********************************************************************
    Egress mirroring functionality in End-To-End mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                                      UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable, currentDsaType, currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfEgressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorSourceBasedConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorEndToEnd , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                                      UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

#if 0  /* 2 words  DSA not hold parameters of this mode */
    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfEgressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfEgressMirrorEndToEnd);
#endif /*0*/

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfEgressMirrorSourceBased */
        UTF_TEST_CALL_MAC(prvTgfEgressMirrorEndToEnd);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test transforming from HopByHop mode to source-based mode for ingress mirroring:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Rx mirroring for HopByHop mode;
    send traffic and verify expected mirrored traffic on analyzer ports;
    set Hop-by-Hop forwarding mode;
    send traffic and verify expected mirrored traffic on analyzer ports.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorForwardingModeChange)
{
/********************************************************************
    Test 8.6 - Transforming from hop-by-hop mode to source-based mode and back
               to hop-by-hop mode for ingress mirroring.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfIngressMirrorForwardingModeChangeConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorForwardingModeChangeTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorForwardingModeChangeConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test transforming from HopByHop mode to source-based mode for egress mirroring:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Tx mirroring for HopByHop mode;
    send traffic and verify expected mirrored traffic on analyzer ports;
    set source-based forwarding mode;
    send traffic and verify expected mirrored traffic on analyzer ports.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorForwardingModeChange)
{
/********************************************************************
    Test 8.7 - Transforming from hop-by-hop mode to source-based mode and back
               to hop-by-hop mode for egress mirroring.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfEgressMirrorForwardingModeChangeConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorForwardingModeChangeTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorForwardingModeChangeConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN tag removal for ingress mirrored traffic:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Rx mirroring for HopByHop mode;
    enable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored UnTagged traffic;
    disable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored Tagged traffic.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorVlanTagRemoval)
{
/********************************************************************
    Test 8.8 - VLAN Tag Removal of ingress Mirrored Traffic.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfIngressMirrorVlanTagRemovalConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorVlanTagRemovalTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorVlanTagRemovalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN tag removal for egress mirrored traffic:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Tx mirroring for HopByHop mode;
    enable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored UnTagged traffic;
    disable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored Tagged traffic.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorVlanTagRemoval)
{
/********************************************************************
    Test 8.9 - VLAN Tag Removal of egress Mirrored Traffic.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfEgressMirrorVlanTagRemovalConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorVlanTagRemovalTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorVlanTagRemovalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic ingress mirroring for device with single Rx analyzer:
    configure VLAN, FDB entries;
    configure 1 analyzer interface and enable Rx mirroring;
    send traffic and verify expected mirrored traffic on analyzer ports;
    disable Rx mirroring;
    send traffic and verify expected traffic on all ports.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorSingleAnalyzer)
{
/********************************************************************
    Test 10 - basic ingress mirroring for device with single Rx analyzer.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfIngressMirrorSingleAnalyzerConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorSingleAnalyzerTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorSingleAnalyzerConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic ingress mirroring for device with single Tx analyzer:
    configure VLAN, FDB entries;
    configure 1 analyzer interface and enable Tx mirroring;
    send traffic and verify expected mirrored traffic on analyzer ports;
    disable Tx mirroring;
    send traffic and verify expected traffic on all ports.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorSingleAnalyzer)
{
/********************************************************************
    Test 11 - basic Egress mirroring for device with single Tx analyzer.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfEgressMirrorSingleAnalyzerConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorSingleAnalyzerTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorSingleAnalyzerConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCombined0MirrorSingleAnalyzer
*/
UTF_TEST_CASE_MAC(prvTgfCombined0MirrorSingleAnalyzer)
{
/********************************************************************
    Test 11 - basic 1 combined Ingress + Egress mirroring for device with single Rx,Tx analyzer.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfCombinedMirrorSingleAnalyzerConfigurationSet(0);

    /* Generate traffic */
    prvTgfCombinedMirrorSingleAnalyzerTrafficGenerate();

    /* Restore configuration */
    prvTgfCombinedMirrorSingleAnalyzerConfigurationRestore();
}
/*----------------------------------------------------------------------------*/
/*
    prvTgfCombined1MirrorSingleAnalyzer
*/
UTF_TEST_CASE_MAC(prvTgfCombined1MirrorSingleAnalyzer)
{
/********************************************************************
    Test 12 - basic 2 combined Ingress + Egress mirroring for device with single Rx,Tx analyzer.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfCombinedMirrorSingleAnalyzerConfigurationSet(1);

    /* Generate traffic */
    prvTgfCombinedMirrorSingleAnalyzerTrafficGenerate();

    /* Restore configuration */
    prvTgfCombinedMirrorSingleAnalyzerConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test egress mirroring according to VLAN, Egress port, STC highest index:
    configure VLAN, FDB entries;
    configure egress analyzer interface and enable Tx mirroring;
    send traffic and verify expected mirrored traffic on analyzer ports;
    set analyzer index for egress mirrored traffic according to port\VLAN\STC;
        send traffic and verify expected mirrored traffic according to port\VLAN\STC.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorAnalyzerIndex)
{
/********************************************************************
    Test 13 - Egress mirroring according to VLAN, Egress port, STC highest index.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA3_E);

    /* Set configuration */
    prvTgfEgressMirrorAnalyzerIndexConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorAnalyzerIndexTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorAnalyzerIndexConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test egress mirroring to CPU, egress port highest index:
    configure VLAN, FDB entries;
    configure egress analyzer interface and enable Tx mirroring for CPU port;
    set analyzer index for egress mirrored traffic for CPU port;
    configure IPv4 MC routing;
    send IPv4 MC traffic and verify expected mirrored traffic and Rx packet's CPU code.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorAnalyzerIndexToCpu)
{
/********************************************************************
    Test 14 - Egress mirroring to CPU, Egress port highest index.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA3_E);


    /* Set configuration */
    prvTgfEgressMirrorAnalyzerIndexToCpuConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorAnalyzerIndexToCpuTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorAnalyzerIndexToCpuConfigurationRestore();
}

/* AUTODOC: Test EPort Vs Physical port ingress mirroring.
    set eport e1 , ePort e2 both associated with physical port pA.
    use tti to classify eVlan = 0x10 from port pA as e1
    use tti to classify eVlan = 0x20 from port pA as e2
    (all others keep the default eport and not get new assignment)


case 1:
    set pA with rx mirror (analyzer 3)
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 4.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA generate mirroring --> to analyzer 3.

case 2:
    unset e1 from rx mirror
    --> check:
        traffic from e1 generate mirroring --> to analyzer 3(due to physical).
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA generate mirroring --> to analyzer 3.

case 3:
    unset e2 from rx mirror
    --> check:
        traffic from e1 generate mirroring --> to analyzer 3(due to physical).
        traffic from e2 generate mirroring --> to analyzer 3(due to physical).
        other traffic from pA generate mirroring --> to analyzer 3.

case 4:
    set pA with rx mirror (analyzer 6)
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 6(due to physical).
        traffic from e2 generate mirroring --> to analyzer 6(due to physical).
        other traffic from pA generate mirroring --> to analyzer 3.

case 5:
    unset pA from rx mirror
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 4.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA NOT generate mirroring.

case 6:
    unset e1 from rx mirror
    --> check:
        traffic from e1 NOT generate mirroring.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA NOT generate mirroring.

case 7:
    unset e2 from rx mirror
    --> check:
        traffic from e1 NOT generate mirroring.
        traffic from e2 NOT generate mirroring.
        other traffic from pA NOT generate mirroring.

*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorEportVsPhysicalPort)
{
    /* eports relevant only to eArch devices */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfIngressMirrorEportVsPhysicalPortTest();
}

/* AUTODOC: Test EPort Vs Physical port egress mirroring.
    Test similar to prvTgfIngressMirrorEportVsPhysicalPort , but for 'egress mirror'
    and not for 'ingress mirror'
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorEportVsPhysicalPort)
{
    /* eports relevant only to eArch devices */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfEgressMirrorEportVsPhysicalPortTest();
}

/* AUTODOC: Test eVlan based egress mirroring
    two cases:
    1. UC traffic
    2. Multi-destination traffic e.g. flooding (several ports send mirror replications)
*/
UTF_TEST_CASE_MAC(prvTgfEgressVlanMirroring)
{
    /* egress vlan mirroring supported on Puma3 and eArch , but test not implemented for Puma3 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfEgressVlanMirroringTest();
}

/* Test Vlan based ingress mirroring */
UTF_TEST_CASE_MAC(prvTgfIngressVlanMirroring)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (UTF_CH1_E | UTF_CH2_E | UTF_CH3_E));

    prvTgfIngressVlanMirroringTest();
}

/* AUTODOC: Test ingress analyzer that is EPort that is mapped by the E2Phy to physical VIDX.
    With analyzer (egress ports) vidx members in STP blocking/forward .
    test also check that vidx analyzer ports must be in vlan ... otherwise filtered.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfIngressMirrorAnalyzerIsEportIsVidxTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorAnalyzerIsEportIsVidx , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('eDSA')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* state that we run in DSA tag mode - 4 words */
    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

    if(GT_FALSE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* run the test - prvTgfEgressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx);

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorAnalyzerIsEportIsVidx , with the 'ingress port'
            that set as (for ingress and egress) cascade port, and get traffic with DSA tag 'forward' ('eDSA')
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* state that we run in DSA tag mode - 4 words */
    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

    if(GT_FALSE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* run the test */
    prvTgfEgressMirrorAnalyzerIsEportIsVidxTest();

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: This Test will check for sniff outgoing counter.
            Configuration sets the vlan egress mirror to analyzer enabled/disabled.
            Packet is send and verify the sniff outgoing counter in the EREP unit
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorToAnalyzerCounter)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));
    prvTgfEgressMirrorToAnalyzerCounterTest();
}

/*
 * Configuration of tgfMirror suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfMirror)
    /*for device with multiple Rx/Tx analyzer */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorHopByHop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorSourceBased)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorEndToEnd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorHighestIndexSelection)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorHopByHop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorSourceBased)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorEndToEnd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorForwardingModeChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorForwardingModeChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorVlanTagRemoval)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorVlanTagRemoval)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorEportVsPhysicalPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorEportVsPhysicalPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressVlanMirroring)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressVlanMirroring)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)

    /*for device with single Rx/Tx analyzer */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorSingleAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorSingleAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCombined0MirrorSingleAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCombined1MirrorSingleAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorAnalyzerIndex)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorAnalyzerIndexToCpu)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorToAnalyzerCounter)


UTF_SUIT_END_TESTS_MAC(tgfMirror)

