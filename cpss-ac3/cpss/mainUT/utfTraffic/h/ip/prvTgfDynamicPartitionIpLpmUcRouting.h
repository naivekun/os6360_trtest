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
* @file prvTgfDynamicPartitionIpLpmUcRouting.h
*
* @brief IP LPM Engine
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfDynamicPartitionIpLpmUcRoutingh
#define __prvTgfDynamicPartitionIpLpmUcRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfDynamicPartitionIpLpmDbConfigurationSet function
* @endinternal
*
* @brief   Set IP LPM DB Base Configuration
*
* @param[in] lastLineIndex            - index of the last line in the TCAM,
*                                      used to create the new LPP DB
*                                       None
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbConfigurationSet
(
    GT_U32 lastLineIndex
);

/**
* @internal prvTgfDynamicPartitionIpLpmDbConfigurationRestore function
* @endinternal
*
* @brief   Restore IP LPM DB Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcBaseConfigurationSet
(
    GT_VOID
);
/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet
(
    GT_VOID
);



/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcRoutingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcRoutingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationCostumerTestScenarioSet function
* @endinternal
*
* @brief   Set Costumer Test Scenario Route Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationCostumerTestScenarioSet
(
    GT_VOID
);


/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A specific test that fit a 8 lines TCAM
*         So in order for the test to check what we want we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=7
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenario
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A random IP v4/v6 prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=1000
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomCostumerTestScenario
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomTestScenario function
* @endinternal
*
* @brief   Test scenario
*         A random IP v4/v6 prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=1000
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomTestScenario
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenarioTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenarioTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioRestore function
* @endinternal
*
* @brief   Clean test scenario from costumer
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDynamicPartitionIpLpmUcRoutingh */


