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
* @file prvCpssGenNetworkIfDsaTag.h
* @version   6
********************************************************************************
*/

#ifndef __prvCpssGenNetworkIfDsaTagh
#define __prvCpssGenNetworkIfDsaTagh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>


#define PRV_CPSS_DSA_TAG_PCKT_OFFSET_CNS        12
#define PRV_CPSS_VLAN_ETHR_TYPE_MSB_CNS         0x81
#define PRV_CPSS_VLAN_ETHR_TYPE_LSB_CNS         0x00

/* offset in original packet for un/tagged frames */
#define PRV_CPSS_BUFF_PCKT_UNTAGGED_OFFSET_CNS  12
#define PRV_CPSS_BUFF_PCKT_TAGGED_OFFSET_CNS    16

/**
* @enum PRV_CPSS_NET_DSA_TYPE_ENT
 *
 * @brief Enumeration of DSA tag types
*/
typedef enum{

    /** regular DSA tag (single word) */
    PRV_CPSS_NET_DSA_TYPE_REGULAR_E,

    /** extended DSA tag (two words) */
    PRV_CPSS_NET_DSA_TYPE_EXTENDED_E

} PRV_CPSS_NET_DSA_TYPE_ENT;


/**
* @enum PRV_CPSS_NET_DSA_CMD_ENT
 *
 * @brief Enumeration of DSA tag commands
*/
typedef enum{

    /** DSA command is "To CPU" */
    PRV_CPSS_NET_DSA_CMD_TO_CPU_E = 0 ,

    /** DSA command is "From CPU" */
    PRV_CPSS_NET_DSA_CMD_FROM_CPU_E    ,

    /** DSA command is "To Analyzer" */
    PRV_CPSS_NET_DSA_CMD_TO_ANALYZER_E ,

    /** DSA command is "Forward" */
    PRV_CPSS_NET_DSA_CMD_FORWARD_E

} PRV_CPSS_NET_DSA_CMD_ENT;




/**
* @struct PRV_CPSS_NET_DSA_PARAMS_STC
 *
 * @brief structure of DSA tag parameters
*/
typedef struct{
    GT_U32                      numberOfWords;
    GT_U8                       vpt;
    GT_U16                      vid;
    GT_U8                       cfi;
    PRV_CPSS_NET_DSA_CMD_ENT    dsaCmd;
    union{
        struct{
            GT_BOOL     srcTagged;
            GT_U8       srcDevNum;
            GT_BOOL     srcIsTrunk;
            union
            {
                GT_TRUNK_ID srcTrunkId;
                GT_U8       srcPortNum;
            }source;
        }forward;

        struct{
            GT_BOOL     dstTagged;
            GT_U8       tc;

            GT_BOOL     useVidx;
            union
            {
                GT_U16      vidx;
                struct
                {
                    GT_U8   tgtDev;
                    GT_U8   tgtPort;
                }devPort;
            }dest;
        }fromCpu;

        struct{
            GT_U32  dummy;/* not implemented yet (not needed yet)*/
        }toAnalyzer;

        struct{
            GT_U32  dummy;/* not implemented yet (not needed yet)*/
        }toCpu;

    }dsaFormat;

}PRV_CPSS_NET_DSA_PARAMS_STC;

/**
* @internal prvCpssNetDsaTagBuild function
* @endinternal
*
* @brief   Builds the DSA Tag .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaTagType               - the type of DSA tag to build (regular/extended)
* @param[in] dsaParamsPtr             - (pointer to) dsa parameters.
*
* @param[out] dsaTagPtr                - (pointer to) the DSA tag.
*
* @retval GT_OK                    - if successful, or
* @retval GT_NOT_IMPLEMENTED       - the request is not implemented yet.
* @retval GT_BAD_PARAM             - one of the parameters is not valid
* @retval GT_BAD_PTR               - one of the pointers is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssNetDsaTagBuild
(
    IN PRV_CPSS_NET_DSA_TYPE_ENT    dsaTagType,
    IN PRV_CPSS_NET_DSA_PARAMS_STC  *dsaParamsPtr,
    OUT GT_U32                      *dsaTagPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssGenNetworkIfDsaTagh */



