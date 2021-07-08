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
* @file tgfCommonExactMatch.c
*
* @brief Enhanced UTs for CPSS Exact Match
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfPclGen.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionPcl.h>
#include <exactMatch/prvTgfExactMatchBasicExpandedActionPcl.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionTti.h>
#include <exactMatch/prvTgfExactMatchUdbPcl.h>
#include <exactMatch/prvTgfExactMatchDefaultActionUdbPcl.h>
#include <exactMatch/prvTgfExactMatchExpandedActionUdbPcl.h>
#include <exactMatch/prvTgfExactMatchTtiPclFullPath.h>
#include <exactMatch/prvTgfExactMatchBasicExpandedActionTti.h>
#include <exactMatch/prvTgfExactMatchReducedActionTti.h>


/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchBasicDefaultActionPcl

    configure VLAN, FDB entries
    configure PCL rules - to trap
    send traffic
    verify traffic is trapped

    Invalidate PCL Rule
    Set Exact Match Default Action configuration
    verify traffic is drop
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicDefaultActionPcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL configuration
        3. Generate traffic - expect match in TCAM - traffic trapped
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match Default entry hit - traffic drop
        6. Restore PCL configuration
        7. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl configuration */
    prvTgfExactMatchBasicDefaultActionPclGenericConfig();

    /* Set Exact Match Default Action PCL configuration */
    prvTgfExactMatchBasicDefaultActionPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicDefaultActionPclTrafficGenerate(GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchBasicDefaultActionPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Default Action */
    prvTgfExactMatchBasicDefaultActionPclTrafficGenerate(GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchBasicDefaultActionPclGenericRestore();

    /* Restore Exact Match Default Action PCL configuration */
    prvTgfExactMatchBasicDefaultActionPclConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchBasicExpandedActionPcl

    configure VLAN, FDB entries
    configure PCL rules - to trap
    send traffic
    verify traffic is trapped

    Invalidate PCL Rule - to prevent a hit in TCAM
    Set Exact Match Profile key parameters
    Set Exact Match Expanded Action configuration
    Set Exact Match Entry configuration
    verify traffic is drop

    Invalidate the exact match entry
    verify traffic is forwarded (FDB)
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicExpandedActionPcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL configuration
        3. Generate traffic - expect match in TCAM - traffic trapped
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic drop
        6. Invalidate Exact Match Entry
        7. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
        8. Restore PCL configuration
        9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl configuration */
    prvTgfExactMatchBasicExpandedActionPclGenericConfig();

    /* Set Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchBasicExpandedActionPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchBasicExpandedActionPclExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* set back TCAM priotity over Exact Match */
    prvTgfExactMatchBasicExpandedActionPclExactMatchPriorityConfig(GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchBasicExpandedActionPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchBasicExpandedActionPclInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_FALSE,GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchBasicExpandedActionPclGenericRestore();

    /* Restore Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchBasicExpandedActionPclConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchDefaultActionUdb40EthOthreL2Pcl

    configure VLAN, FDB entries;
    configure PCL rule and different UDBs - forward to specific port;

    Test on Ingress PCL UDB40 L2 offset type on Ethernet Other packet.
    Fields: 40 first bytes from L2 header beginning.
    UDB mapping 10-49 => L2+0 - L2+39

    send traffic to match PCL rules and UDBs;
    verify traffic is forwarded with trigger.

    Invalidate PCL Rule
    Set Exact Match Default Action configuration
    verify traffic is drop
*/
UTF_TEST_CASE_MAC(tgfExactMatchDefaultActionUdb40EthOthreL2Pcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL UDB configuration
        3. Generate traffic - expect match in TCAM - traffic forwarded with trigger
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match Default entry hit - traffic drop
        6. Restore PCL configuration
        7. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl UDB configuration
       Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet */
    prvTgfExactMatchUdb40EthOthreL2PclGenericConfig();

    /* Set Exact Match Default Action PCL configuration */
    prvTgfExactMatchDefaultActionUdbPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchDefaultActionUdbPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Default Action */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchUdb40EthOthreL2PclGenericRestore();

    /* Restore Exact Match Default Action PCL configuration */
    prvTgfExactMatchDefaultActionUdbPclConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchExpandedActionUdb40EthOthreL2Pcl

    configure VLAN, FDB entries
    configure PCL rules - forward to specific port

    Test on Ingress PCL UDB40 L2 offset type on Ethernet Other packet.
    Fields: 40 first bytes from L2 header beginning.
    UDB mapping 10-49 => L2+0 - L2+39

    send traffic
    verify traffic is forwarded with trigger.

    Invalidate PCL Rule - to prevent a hit in TCAM
    Set Exact Match Profile key parameters
    Set Exact Match Expanded Action configuration
    Set Exact Match Entry configuration
    verify traffic is drop

    Invalidate the exact match entry
    verify traffic is forwarded (FDB) - with specific VLAN

*/
UTF_TEST_CASE_MAC(tgfExactMatchExpandedActionUdb40EthOthreL2Pcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL UDB configuration
        3. Generate traffic - expect match in TCAM - traffic forward
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic drop
        6. Invalidate Exact Match Entry
        7. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
        8. Restore PCL configuration
        9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl UDB configuration
       Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet */
    prvTgfExactMatchUdb40EthOthreL2PclGenericConfig();

    /* Set Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchExpandedActionUdbPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* set back TCAM priotity over Exact Match */
    prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig(GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE, GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchExpandedActionUdbPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_TRUE, GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchExpandedActionUdbPclInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchUdb40EthOthreL2PclGenericRestore();

    /* Restore Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchExpandedActionUdbPclConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPath:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry

*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPath)
{
    /*
    1. Set Exact Match Non Default Action configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic drop
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet();

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigSet(CPSS_PACKET_CMD_TRAP_TO_CPU_E);

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_TRUE,GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPort:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPort)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 3 from Expanded action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1 (from FDB)
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet();

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigSet(CPSS_PACKET_CMD_FORWARD_E);

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReduced:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReduced)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet();

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedNonZeroKeyStart:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   Key Start parameter different from 0
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonZeroKeyStart)
{
    /*
    1. Set Exact Match configuration - Key Start parameter different from 0
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet();

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams.keySize to be != 0
       UDB0 should be removed from TCAM key -
       and we should get a hit on the Exact Match*/
    prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet();

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedNonFullKeyMask:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   Key Start parameter different from 0
   Key Mask parameter different from 0xFF
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonFullKeyMask)
{
    /*
    1. Set Exact Match configuration -
       Key Start parameter different from 0
       Key Mask parameter different from 0xFF
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet();

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams.keySize to be != 0
       UDB0 should be removed from TCAM key -
       and we should get a hit on the Exact Match*/
    prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet();

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet();

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathUdb40ConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams
       keyParams.keySize = 47 bytes
       keyParams.keyStart = 1
       keyParams.mask != 0xff
       UDB0 should be removed from TCAM key and we should get a hit on the Exact Match */
    prvTgfExactMatchTtiPclFullPathUdb47ConfigSet();

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47CheckActivity:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   check activity bit update
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47CheckActivity)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. check Activity bit = 0
    5. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    6. check Activity bit = 1
    7  clear Activity
    8. Invalidate Exact Match Entry
    9. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    10. check Activity bit = 0
    11. Restore TTI configuration
    12. Restore PCL configuration
    13. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet();

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathUdb47ConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams
       keyParams.keySize = 47 bytes
       keyParams.keyStart = 1
       keyParams.mask != 0xff
       UDB0 should be removed from TCAM key and we should get a hit on the Exact Match */
    prvTgfExactMatchTtiPclFullPathUdb47ConfigSet();

    /* no traffic hit - expect activity =0 */
    prvTgfExactMatchTtiPclFullPathActivitySet(GT_FALSE);

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* traffic hit - expect activity = 1 */
    prvTgfExactMatchTtiPclFullPathActivitySet(GT_TRUE);

    /* disable global activity bit */
    prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,GT_FALSE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* traffic hit - expect activity = 0, since it was globaly disabled */
    prvTgfExactMatchTtiPclFullPathActivitySet(GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}
/*----------------------------------------------------------------------------*/
/*
   tgfExactMatchDefaultActionTti :
   Test functionality of Exact Match default action ,
   base on tgfTunnelTermLlcNonSnapType
   Exact Match default action gets active incase no hit in Tcam or Exact Match lookup
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicDefaultActionTti)
{
    /*
    1. Set Exact match profileId and Default Action
    2. Set TTI Ether Type Key Base configuration
    3. Generate traffic -- check traffic pass
    4. disable TTI rule
    5. Generate traffic -- check traffic block
    6. change Default Action for Trap
    7. Generate traffic -- check traffic block and cpuCode
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigSet();

    /* Set Exact Match Default Action TTI basic configuration */
    prvTgfExactMatchBasicDefaultActionTtiConfigSet();

    /* Generate traffic - expect to be forwarded */
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* disable TTI rule */
    prvTgfExactMatchBasicDefaultActionTtiRuleValidStatusSet(GT_FALSE);

    /* Generate traffic -- check no traffic and no cpuCode */
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_FALSE,GT_FALSE);

    /* change Default Action for Trap */
    prvTgfExactMatchBasicDefaultActionTtiCommandTrap();

    /* Generate traffic -- check no traffic and cpuCode 502*/
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_TRUE,GT_FALSE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore();

    /* Restore Exact match Default Action TTI Configuration */
    prvTgfExactMatchBasicDefaultActionTtiConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchBasicExpandedActionTti :
   Test functionality of Exact Match Expanded Action ,

   Exact Match Expanded action gets active incase no hit in Tcam or
   proity bit in TTI action is set

   Test on PRV_TGF_TTI_RULE_UDB_30_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
   first 4 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicExpandedActionTti)
{
      /*
        1. Set TTI configuration
        2. Set Exact Match Expanded Action TTI configuration -Trap
        3. Generate traffic - expect match in TCAM - traffic forwarded
        4. set Exact Match priority over TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic Trap
        6. set Exact Match priority over TCAM back to FALSE
        7. Generate traffic - expect match in TCAM - traffic forwarded
        8. Invalidate TTI Rule - to prevent a hit in TCAM
        9. Generate traffic - expect Exact Match entry hit - traffic Trap
        10. Invalidate Exact Match Entry
        11. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
        12. Restore TTI configuration
        13. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI configuration */
    prvTgfExactMatchBasicExpandedActionTtiGenericConfigSet();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchBasicExpandedActionTtiConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchBasicExpandedActionTtiExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_TRUE, GT_FALSE);

    /* set back TCAM priotity over Exact Match */
    prvTgfExactMatchBasicExpandedActionTtiExactMatchPriorityConfig(GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_FALSE, GT_TRUE);

    /* invalidate TTI Rule */
    prvTgfExactMatchBasicExpandedActionTtiRuleValidStatusSet(GT_FALSE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_TRUE, GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchBasicExpandedActionTtiInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore();

    /* Restore Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchBasicExpandedActionTtiConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchReducedActionTti :
   Test functionality of Exact Match Reduced Action ,

    Test on PRV_TGF_TTI_RULE_UDB_10_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
    first 7 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchReducedActionTti)
{
      /*
        1. Set TTI configuration
        2. Set Exact Match Expanded Action TTI configuration -Trap with CPU code 503
        3. Generate traffic - expect match in TCAM - traffic forwarded
        4. set Exact Match priority over TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic Trap code 503
        6. change Reduced Action to Trap with CPUcode 505 and Expanded Action to forward
        8. Generate traffic - expect Exact Match entry hit - traffic Trap with CPU code 505
        9. Restore TTI configuration
        10. Restore Exact Match configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigSet();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_TRUE,GT_FALSE,0);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode 503
    */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE,503);

    /*change Reduced Action to Trap with CPUcode 505
      and Expanded Action to forward
    */
    prvTgfExactMatchReducedActionTtiReducedActionSet();

    /* Generate traffic - check no traffic - CPUcode 505
    */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE, GT_TRUE,505);

    /* Restore TTI generic configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigRestore();

    /* Restore Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigRestore();
}

/*
 * Configuration of tgfExactMatch suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfExactMatch)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicDefaultActionPcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicExpandedActionPcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchDefaultActionUdb40EthOthreL2Pcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchExpandedActionUdb40EthOthreL2Pcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPath)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReduced)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonZeroKeyStart)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonFullKeyMask)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47CheckActivity)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicDefaultActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicExpandedActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchReducedActionTti)
UTF_SUIT_END_TESTS_MAC(tgfExactMatch)

















