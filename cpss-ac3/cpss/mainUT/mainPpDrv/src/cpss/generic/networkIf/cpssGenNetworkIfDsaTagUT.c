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
* @file cpssGenNetworkIfDsaTagUT.c
*
* @brief Unit tests for cpssGenNetworkIf, that provides
* CPSS build and parsing of DSA tag
*
* @version   1.7
********************************************************************************
*/

/* includes */
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfDsaTag.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* defines */

/* Invalid enum */
#define NET_IF_INVALID_ENUM_CNS    0x5AAAAAA5


/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssNetDsaTagBuild
(
    IN PRV_CPSS_NET_DSA_TYPE_ENT    dsaTagType,
    IN PRV_CPSS_NET_DSA_PARAMS_STC  *dsaParamsPtr,
    OUT GT_U32                      *dsaTagPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssNetDsaTagBuild)
{
/*
    1. Call with dsaTagType [PRV_CPSS_NET_DSA_TYPE_REGULAR_E /
                             PRV_CPSS_NET_DSA_TYPE_EXTENDED_E],
                 dsaParamsPtr {numberOfWords [0 / 1],
                               vpt [0 / 0xFF],
                               vid [100 / 4095],
                               cfi [0 / 1],
                               dsaCmd [PRV_CPSS_NET_DSA_CMD_FORWARD_E / 
                                       PRV_CPSS_NET_DSA_CMD_FROM_CPU_E],
                               dsaFormat.forward{srcTagged [GT_FALSE / GT_TRUE],
                                                 srcDevNum [0 / 8],
                                                 srcIsTrunk [GT_FALSE / GT_TRUE],
                                                 source{srcPortNum [0], srcTrunkId [2]}
                                                 }
                               dsaFormat.fromCpu { dstTagged [GT_FALSE / GT_TRUE],
                                                       tc [0 / 7],
                                                       useVidx [GT_FALSE / GT_TRUE],
                                                       dest {devPort {tgtDev[0], tgtPort[0]}, vidx [100]}
                                                      }
                              }.
    Expected: GT_OK.
    2. Call with out of range dsaTagType [0x5AAAAAA5]
                 and other params from 1.
    Expected: GT_BAD_PARAM.
    3. Call with out of range dsaParamsPtr->dsaCmd [0x5AAAAAA5]
                 and other params from 1.
    Expected: GT_BAD_PARAM.
    4. Call with null dsaParamsPtr [NULL]
                 and other params from 1.
    Expected: GT_BAD_PTR.
    5. Call with null dsaTagPtr [NULL]
                 and other params from 1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    PRV_CPSS_NET_DSA_TYPE_ENT    dsaTagType = PRV_CPSS_NET_DSA_TYPE_REGULAR_E;
    PRV_CPSS_NET_DSA_PARAMS_STC  dsaParams;
    GT_U32                       dsaTag     = 0;


    /*
        1. Call with dsaTagType [PRV_CPSS_NET_DSA_TYPE_REGULAR_E /
                                 PRV_CPSS_NET_DSA_TYPE_EXTENDED_E],
                     dsaParamsPtr {numberOfWords [0 / 1],
                                   vpt [0 / 0xFF],
                                   vid [100 / 4095],
                                   cfi [0 / 1],
                                   dsaCmd [PRV_CPSS_NET_DSA_CMD_FORWARD_E / 
                                           PRV_CPSS_NET_DSA_CMD_FROM_CPU_E],
                                   dsaFormat.forward { srcTagged [GT_TRUE],
                                                       srcDevNum [0],
                                                       srcIsTrunk [GT_FALSE],
                                                       source{srcPortNum [0]}
                                                      }
                                   dsaFormat.fromCpu { dstTagged [GT_FALSE],
                                                       tc [5],
                                                       useVidx [GT_TRUE],
                                                       dest.vidx [100]
                                                      }
                                  }.
        Expected: GT_OK.
    */

    /* call with dsaTagType = PRV_CPSS_NET_DSA_TYPE_REGULAR_E */
    dsaTagType = PRV_CPSS_NET_DSA_TYPE_REGULAR_E;

    dsaParams.numberOfWords = 0;
    dsaParams.vpt           = 0;
    dsaParams.vid           = 100;
    dsaParams.cfi           = 0;
    dsaParams.dsaCmd        = PRV_CPSS_NET_DSA_CMD_FORWARD_E;

    dsaParams.dsaFormat.forward.srcTagged         = GT_TRUE;
    dsaParams.dsaFormat.forward.srcDevNum         = 0;
    dsaParams.dsaFormat.forward.srcIsTrunk        = GT_FALSE;
    dsaParams.dsaFormat.forward.source.srcPortNum = 0;
    
    st = prvCpssNetDsaTagBuild(dsaTagType, &dsaParams, &dsaTag);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dsaTagType);

    /* call with dsaTagType = PRV_CPSS_NET_DSA_TYPE_EXTENDED_E */
    dsaTagType = PRV_CPSS_NET_DSA_TYPE_EXTENDED_E;

    dsaParams.numberOfWords = 1;
    dsaParams.vpt           = 0xFF;
    dsaParams.vid           = 4095;
    dsaParams.cfi           = 1;
    dsaParams.dsaCmd        = PRV_CPSS_NET_DSA_CMD_FROM_CPU_E;

    dsaParams.dsaFormat.fromCpu.dstTagged = GT_FALSE;
    dsaParams.dsaFormat.fromCpu.tc        = 5;
    dsaParams.dsaFormat.fromCpu.useVidx   = GT_TRUE;
    dsaParams.dsaFormat.fromCpu.dest.vidx = 100;

    st = prvCpssNetDsaTagBuild(dsaTagType, &dsaParams, &dsaTag);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dsaTagType);

    /*
        2. Call with out of range dsaTagType [0x5AAAAAA5]
                     and other params from 1.
        Expected: GT_BAD_PARAM.
    */
    dsaTagType = NET_IF_INVALID_ENUM_CNS;

    st = prvCpssNetDsaTagBuild(dsaTagType, &dsaParams, &dsaTag);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dsaTagType);

    dsaTagType = PRV_CPSS_NET_DSA_TYPE_REGULAR_E;

    /*
        3. Call with out of range dsaParamsPtr->dsaCmd [0x5AAAAAA5]
                 and other params from 1.
        Expected: GT_BAD_PARAM.
    */
    dsaParams.dsaCmd = NET_IF_INVALID_ENUM_CNS;

    st = prvCpssNetDsaTagBuild(dsaTagType, &dsaParams, &dsaTag);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "dsaParamsPtr->dsaCmd = %d", dsaParams.dsaCmd);

    dsaParams.dsaCmd = PRV_CPSS_NET_DSA_CMD_FROM_CPU_E;

    /*
        4. Call with null dsaParamsPtr [NULL]
                     and other params from 1.
        Expected: GT_BAD_PTR.
    */
    st = prvCpssNetDsaTagBuild(dsaTagType, NULL, &dsaTag);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "dsaParamsPtr = NULL");

    /*
        5. Call with null dsaTagPtr [NULL]
                     and other params from 1.
        Expected: GT_BAD_PTR.
    */
    st = prvCpssNetDsaTagBuild(dsaTagType, &dsaParams, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "dsaTagPtr = NULL");
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssGenNetworkIf suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssGenNetworkIf)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssNetDsaTagBuild)
UTF_SUIT_END_TESTS_MAC(cpssGenNetworkIf)


