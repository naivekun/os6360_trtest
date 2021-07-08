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
* @file prvTgfPclMiscellanous.h
*
* @brief Miscellanous definitions.
*
* @version   13
********************************************************************************
*/
#ifndef __prvTgfPclMiscellanoush
#define __prvTgfPclMiscellanoush

#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* prvTgfPclLookup00And01
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclLookup00And01
(
    GT_VOID
);

/*******************************************************************************
* prvTgfPclPortListMode
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListMode
(
    GT_VOID
);

/*******************************************************************************
* prvTgfPclWriteRuleInvalid
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclWriteRuleInvalid
(
    GT_VOID
);

/*******************************************************************************
* prvTgfPclFourLookups
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclFourLookups
(
    GT_VOID
);

/*******************************************************************************
* prvTgfPclPortListCrossIpclTest
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListCrossIpclTest
(
    GT_VOID
);

/*******************************************************************************
* prvTgfPclPortListCrossEpclTest
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListCrossEpclTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEportEvlanIpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL extension srcPort (ePort) field test
*/
GT_VOID prvTgfPclEportEvlanIpclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEportEvlanEpclTrafficTest function
* @endinternal
*
* @brief   Egress PCL extension srcPort and trgPort(ePort) fields test
*/
GT_VOID prvTgfPclEportEvlanEpclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEvlanMatchIpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL Rule+extension vid field test
*/
GT_VOID prvTgfPclEvlanMatchIpclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEvlanMatchEpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL Rule+extension vid field test
*/
GT_VOID prvTgfPclEvlanMatchEpclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEArchRedirectIpclEPortTest function
* @endinternal
*
* @brief   Ingress PCL redirection to ePort test
*/
GT_VOID prvTgfPclEArchRedirectIpclEPortTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEArchRedirectIpclEVidTest function
* @endinternal
*
* @brief   Ingress PCL redirection to eVid test
*/
GT_VOID prvTgfPclEArchRedirectIpclEVidTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEArchRedirectIpclEVidxTest function
* @endinternal
*
* @brief   Ingress PCL redirection to eVidx test
*/
GT_VOID prvTgfPclEArchRedirectIpclEVidxTest
(
    GT_VOID
);

/**
* @internal prvTgfPclTag1MatchIpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL extension Tag1 fields field test
*/
GT_VOID prvTgfPclTag1MatchIpclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclTag1MatchEpclTrafficTest function
* @endinternal
*
* @brief   Egress PCL extension Tag1 fields test
*/
GT_VOID prvTgfPclTag1MatchEpclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclSrcTrunkIdMatchIpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL SrcTrunkId field test
*/
GT_VOID prvTgfPclSrcTrunkIdMatchIpclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclLion2UdbOverrideIpclQosProfileTrafficTest function
* @endinternal
*
* @brief   Ingress PCL SrcTrunkId field test
*/
GT_VOID prvTgfPclLion2UdbOverrideIpclQosProfileTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclLion2UdbOverrideIpclTag1TrafficTest function
* @endinternal
*
* @brief   Ingress PCL SrcTrunkId field test
*/
GT_VOID prvTgfPclLion2UdbOverrideIpclTag1TrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclFWSCncTest function
* @endinternal
*
* @brief   Full wire speed PCL and CNC test:
*         1. Generate FWS traffic
*         2. Check FWS traffic
*         3. Configure PCL and CNC
*         4. Check CNC counters
*/
GT_VOID prvTgfPclFWSCncTest
(
    GT_VOID
);

/**
* @internal prvTgfPclFWSCncRestore function
* @endinternal
*
* @brief   Restore configuration after Full wire speed PCL and CNC test
*/
GT_VOID prvTgfPclFWSCncRestore
(
     IN GT_BOOL useVirtTcam
);

/**
* @internal prvTgfPclFWSMoveTest function
* @endinternal
*
* @brief   Full wire speed PCL Move test:
*         1. Generate FWS traffic
*         2. Check FWS traffic
*         3. Configure PCL and CNC
*         4. Check CNC counters rate to be like Rx Port rate
*         5. Stop traffic, reset MAC and CNC counters
*         6. Start traffic
*         7. Move PCEs up and down several times
*         8. Stop traffic and compare CNC and MAC Rx counters.
* @param[in] useVirtTcam              - GT_TRUE enables using vTCAMs insted of port configuration
* @param[in] prioBasedVTcam           - GT_TRUE enables creating priority based vTCAMs
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
*                                       None
*/
GT_VOID prvTgfPclFWSMoveTest
(
    IN  GT_BOOL    useVirtTcam,
    IN  GT_BOOL    prioBasedVTcam,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
);

/**
* @internal prvTgfPclPumaPortGroupTestUnaware function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPumaPortGroupTestUnaware
(
    GT_VOID
);

/**
* @internal prvTgfPclPumaPortGroupTestAware function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPumaPortGroupTestAware
(
    GT_VOID
);

/**
* @internal prvTgfPclEArchSourcePortTest function
* @endinternal
*
* @brief   PCL source port assignment test
*/
GT_VOID prvTgfPclEArchSourcePortTest
(
    GT_VOID
);
/**
* @internal prvTgfPclTrunkEportRedirectCustomerTest function
* @endinternal
*
* @brief   Trunk and Eport reassignment in FDB learning test
*/
GT_VOID prvTgfPclTrunkEportRedirectCustomerTest
(
    GT_VOID
);

/**
* @internal prvTgfPclVplsVirtSrcPortAssign function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsVirtSrcPortAssign
(
    GT_VOID
);

/**
* @internal prvTgfPclVplsUnknownSaCmd function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsUnknownSaCmd
(
    GT_VOID
);

/**
* @internal prvTgfPclVplsVirtSrcPortAssignAndAcEnable function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsVirtSrcPortAssignAndAcEnable
(
    GT_VOID
);

/**
* @internal prvTgfPclVplsMeteringToFloodedOnlyEnable function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsMeteringToFloodedOnlyEnable
(
    GT_VOID
);

/**
* @internal prvTgfPclVplsBillingToFloodedOnlyEnable function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsBillingToFloodedOnlyEnable
(
    GT_VOID
);

/**
* @internal prvTgfPclVplsVirtualSrcPortMeshIdAssign function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsVirtualSrcPortMeshIdAssign
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclMiscellanoush */


