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
* @file cstTunnelMplsTests.h
*
* @brief test1: tunnel start unknown uc flooding
* MPLS configuration of device with 1 port in ETHERNET domain, and 3 ports in MPLS domain.
* test check 'flooding' from ethernet port to the other 3 mpls ports.(TS to the MPLS domain)
*
* ePort A is port to Ethernet domain
* eport B is port to MPLS domain with 1 label
* eport C is port to MPLS domain with 2 labels
* eport D is port to MPLS domain with 3 labels and PW label + FLOW label + CW label
*
* test2: transit tunnel termination of known uc (2 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
* ePort A is port to MPLS domain with 2 labels
* eport B is port to MPLS domain with 2 labels. Label 1 is swapped.
* eport C is port to MPLS domain with 2 labels. Label 1 is popped & new label is pushed.
*
* test3: transit tunnel termination of known uc (2 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
* ePort B is port to MPLS domain with 2 labels
* eport A is port to MPLS domain with 1 label. Label 1 is pop_swap.
* eport D is port to MPLS domain with 1 label. Label 1 is popped & swap.
*
* test4: transit tunnel termination of known uc (3 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
* ePort A is port to MPLS domain with 3 labels
* eport B is port to MPLS domain with 1 label. Labels 1,2 are pop2.
* eport C is port to MPLS domain with 1 label. Label 1 is pop1 & label 2 is pop1.
* eport D is port to MPLS domain with 1 label. Labels 1,2 are pop2 & label 2 is pushed.
*
* test5: transit tunnel termination of known uc (3 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
* ePort B is port to MPLS domain with 3 labels
* eport C is port to MPLS domain with 2 label. Label 1 is pop1.
* eport D is port to MPLS domain with 2 label. Label 1,2 are pop2 & new label 2 is push.
* eport A is port to MPLS domain with 2 label. Label 1 is pop1 & label 2 is pop_swap.
*
* test6: transit tunnel termination of known uc (4 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 4 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
* ePort A is port to MPLS domain with 4 labels.
* eport B is port to MPLS domain with 1 label. Labels 1,2,3 are pop3
* eport C is port to MPLS domain with 1 label. Label 1,2 are pop2 & label 3 is pop1
* eport D is port to MPLS domain with 1 label. Label 1,2,3 are pop3 & pushed
*
* @version   5
********************************************************************************
*/

#ifndef __cstTunnelMplsTestsh
#define __cstTunnelMplsTestsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal cstTunnelStartMplsUnknownUcFloodingConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingConfig
(
    GT_VOID
);

/**
* @internal cstTunnelStartMplsUnknownUcFloodingTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingTrafficGenerate
(
    GT_VOID
);

/**
* @internal cstTunnelStartMplsUnknownUcFloodingRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingRestore
(
    GT_VOID
);

/**
* @internal cstTunnelStartMplsUnknownUcFloodingTest function
* @endinternal
*
* @brief   test1: tunnel start unknown uc flooding
*         MPLS configuration of device with 1 port in ETHERNET domain, and 3 ports in MPLS domain.
*         test check 'flooding' from ethernet port to the other 3 mpls ports.(TS to the MPLS domain)
*         ePort A is port to Ethernet domain
*         eport B is port to MPLS domain with 1 label
*         eport C is port to MPLS domain with 2 labels
*         eport D is port to MPLS domain with 3 labels and PW label + FLOW label + CW label
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingTest
(
    GT_VOID
);


/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsConfig
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsTrafficGenerate
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsRestore
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsTest function
* @endinternal
*
* @brief   test2: transit tunnel termination of known uc (2 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
*         ePort A is port to MPLS domain with 2 labels
*         eport B is port to MPLS domain with 2 labels. Label 1 is swapped.
*         eport C is port to MPLS domain with 2 labels. Label 1 is popped & new label is pushed.
*         test3: transit tunnel termination of known uc (2 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
*         ePort B is port to MPLS domain with 2 labels
*         eport A is port to MPLS domain with 1 label. Label 1 is pop_swap.
*         eport D is port to MPLS domain with 1 label. Label 1 is popped & swap.
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsTest
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsConfig
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsTrafficGenerate
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsRestore
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsTest function
* @endinternal
*
* @brief   test4: transit tunnel termination of known uc (3 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
*         ePort A is port to MPLS domain with 3 labels
*         eport B is port to MPLS domain with 1 label. Labels 1,2 are pop2.
*         eport C is port to MPLS domain with 1 label. Label 1 is pop1 & label 2 is pop1.
*         eport D is port to MPLS domain with 1 label. Labels 1,2 are pop2 & label 2 is pushed.
*         test5: transit tunnel termination of known uc (3 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
*         ePort B is port to MPLS domain with 3 labels
*         eport C is port to MPLS domain with 2 label. Label 1 is pop1.
*         eport D is port to MPLS domain with 2 label. Label 1,2 are pop2 & new label 2 is push.
*         eport A is port to MPLS domain with 2 label. Label 1 is pop1 & label 2 is pop_swap.
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsTest
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsConfig
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsTrafficGenerate
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsRestore
(
    GT_VOID
);

/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsTest function
* @endinternal
*
* @brief   test6: transit tunnel termination of known uc (4 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 4 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
*         ePort A is port to MPLS domain with 4 labels.
*         eport B is port to MPLS domain with 1 label. Labels 1,2,3 are pop3
*         eport C is port to MPLS domain with 1 label. Label 1,2 are pop2 & label 3 is pop1
*         eport D is port to MPLS domain with 1 label. Label 1,2,3 are pop3 & pushed
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsTest
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __cstTunnelMplsTestsh */

