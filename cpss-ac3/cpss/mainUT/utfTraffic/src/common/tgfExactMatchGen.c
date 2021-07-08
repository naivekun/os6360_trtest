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
* @file tgfExactMatchGen.c
*
* @brief Generic API implementation for Exact Match
*
* @version   1
********************************************************************************
*/

#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <../../cpssEnabler/mainCmd/h/cmdShell/common/cmdExtServices.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <common/tgfExactMatchGen.h>
#include <common/tgfPclGen.h>
#endif /*CHX_FAMILY*/
/******************************************************************************\
 *                              Macro definitions                             *
\******************************************************************************/

#ifdef CHX_FAMILY

extern GT_STATUS prvTgfConvertGenericAction2ToDxChTtiAction2
(
    IN  PRV_TGF_TTI_ACTION_2_STC     *actionPtr,
    OUT CPSS_DXCH_TTI_ACTION_2_STC   *dxChTtiActionPtr
);

extern GT_STATUS prvTgfConvertDxChAction2ToGenericTtiAction2
(
    IN  CPSS_DXCH_TTI_ACTION_2_STC   *dxChTtiActionPtr,
    OUT PRV_TGF_TTI_ACTION_2_STC     *actionPtr
);

/* convert ExactMatch key size into device specific format */
#define PRV_TGF_S2D_KEY_SIZE_CONVERT_MAC(dstKeySize, srcKeySize)               \
    do                                                                         \
    {                                                                          \
        switch (srcKeySize)                                                    \
        {                                                                      \
            case PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E:                            \
                dstKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;              \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E:                           \
                dstKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;             \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E:                           \
                dstKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;             \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E:                           \
                dstKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;             \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* convert ExactMatch key size from device specific format */
#define PRV_TGF_D2S_KEY_SIZE_CONVERT_MAC(dstKeySize, srcKeySize)               \
    do                                                                         \
    {                                                                          \
        switch (dstKeySize)                                                    \
        {                                                                      \
            case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:                          \
                srcKeySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;                \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:                         \
                srcKeySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;               \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:                         \
                srcKeySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;               \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:                         \
                srcKeySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;               \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert ExactMatch client type into device specific format */
#define PRV_TGF_S2D_CLIENT_TYPE_CONVERT_MAC(dstClientType, srcClientType)      \
    do                                                                         \
    {                                                                          \
        switch (srcClientType)                                                 \
        {                                                                      \
            case PRV_TGF_EXACT_MATCH_CLIENT_TTI_E:                             \
                dstClientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;            \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E:                          \
                dstClientType = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;         \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_1_E:                          \
                dstClientType = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E;         \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E:                            \
                dstClientType = CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E;           \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* convert ExactMatch client type from device specific format */
#define PRV_TGF_D2S_CLIENT_TYPE_CONVERT_MAC(dstClientType, srcClientType)      \
    do                                                                         \
    {                                                                          \
        switch (dstClientType)                                                 \
        {                                                                      \
            case CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E:                           \
                srcClientType = PRV_TGF_EXACT_MATCH_CLIENT_TTI_E;              \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E:                        \
                srcClientType = PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;           \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E:                        \
                srcClientType = PRV_TGF_EXACT_MATCH_CLIENT_IPCL_1_E;           \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E:                          \
                srcClientType = PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E;             \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* convert ExactMatch action type into device specific format */
#define PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dstActionType, srcActionType)       \
    do                                                                          \
    {                                                                           \
        switch (srcActionType)                                                  \
        {                                                                       \
        case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:                                  \
            dstActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;                   \
            break;                                                              \
        case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:                                  \
            dstActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;                   \
            break;                                                              \
        case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:                                 \
            dstActionType=CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;                  \
            break;                                                              \
        default:                                                                \
            return GT_BAD_PARAM;                                                \
        }                                                                       \
    } while (0)

/* convert device specific ExactMatch action type */
#define PRV_TGF_D2S_ACTION_TYPE_CONVERT_MAC(dstActionType, srcActionType)       \
    do                                                                          \
    {                                                                           \
        switch (dstActionType)                                                  \
        {                                                                       \
        case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:                                  \
            srcActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;                   \
            break;                                                              \
        case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:                                  \
            srcActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;                   \
            break;                                                              \
        case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:                                 \
            srcActionType=CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;                  \
            break;                                                              \
        default:                                                                \
            return GT_BAD_PARAM;                                                \
        }                                                                       \
    } while (0)

/* convert ExactMatch lookup number into device specific format */
#define PRV_TGF_S2D_LOOKUP_NUM_CONVERT_MAC(dstLookupNum, srcLookupNum)         \
    do                                                                         \
    {                                                                          \
        switch (srcLookupNum)                                                  \
        {                                                                      \
            case PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E:                           \
                dstLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;           \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_LOOKUP_SECOND_E:                          \
                dstLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;          \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

#endif /* CHX_FAMILY */



/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY

/**
* @internal prvTgfExactMatchTtiProfileIdSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile Id for TTI keyType
*
* NOTE: Client lookup for given ttiLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger EMlookup.
*                           GT_FALSE: Do not trigger EM lookup.
* @param[in] profileId    - Exact Match profile identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_OUT_OF_RANGE       - parameter not in valid range.
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*
*/
GT_STATUS  prvTgfExactMatchTtiProfileIdSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT            keyType,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      ttiLookupNum,
    IN  GT_BOOL                             enableExactMatchLookup,
    IN  GT_U32                              profileId
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChExactMatchTtiProfileIdSet(devNum,
                                           (CPSS_DXCH_TTI_KEY_TYPE_ENT)keyType,
                                           (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)ttiLookupNum,
                                           enableExactMatchLookup,profileId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCExactMatchTtiProfileIdSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(ttiLookupNum);
    TGF_PARAM_NOT_USED(enableExactMatchLookup);
    TGF_PARAM_NOT_USED(profileId);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfExactMatchTtiProfileIdGet function
* @endinternal
*
* @brief   Gets the  Exact Match Profile Id form TTI keyType
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger EMlookup.
*                           GT_FALSE: Do not trigger EM lookup.
* @param[out] profileIdPtr  - (pointer to) Exact Match profile
*                             identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                  - on success
* @retval GT_HW_ERROR            - on hardware error
* @retval GT_OUT_OF_RANGE        - parameter not in valid range.
* @retval GT_BAD_PARAM           - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* * NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*/
GT_STATUS prvTgfExactMatchTtiProfileIdGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT            keyType,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      ttiLookupNum,
    OUT GT_BOOL                             *enableExactMatchLookupPtr,
    OUT GT_U32                              *profileIdPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChExactMatchTtiProfileIdGet(devNum, (CPSS_DXCH_TTI_KEY_TYPE_ENT)keyType,
                                           (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)ttiLookupNum,
                                           enableExactMatchLookupPtr,profileIdPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCExactMatchTtiProfileIdGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(ttiLookupNum);
    TGF_PARAM_NOT_USED(profileIdPtr);
    TGF_PARAM_NOT_USED(enableExactMatchLookupPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchPclProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for PCL packet type
*
* NOTE: Client lookup for given pclLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger EM Lookup.
*                           GT_FALSE: Do not trigger EM lookup.
* @param[in] profileId    - Exact Match profile identifier
*       (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  PRV_TGF_PCL_PACKET_TYPE_ENT         packetType,
   IN  GT_U32                              subProfileId,
   IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum,
   IN  GT_BOOL                             enableExactMatchLookup,
   IN  GT_U32                              profileId
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT       dxChPacketType;

    /* convert generic into device specific packetType */
    PRV_TGF_S2D_PCL_UDB_PACKET_TYPE_CONVERT_MAC(dxChPacketType, packetType);

    rc = cpssDxChExactMatchPclProfileIdSet(devNum, direction, dxChPacketType,subProfileId,
                                           (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)pclLookupNum,
                                           enableExactMatchLookup,profileId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchPclProfileIdSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(packetType);
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(subProfileId);
    TGF_PARAM_NOT_USED(pclLookupNum);
    TGF_PARAM_NOT_USED(profileId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchPclProfileIdGet function
* @endinternal
*
* @brief  Gets the Exact Match Profile Id form PCL packet type
*
* @param[in] devNum                   - device number
* @param[in] packetType               - PCL packet type
* @param[in] direction                - ingress/egress
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger EM Lookup.
*                           GT_FALSE: Do not trigger EM lookup.
* @param[in] profileIdPtr - (pointer to)Exact Match profile
*       identifier (APPLICABLE RANGES: 1..15)
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  PRV_TGF_PCL_PACKET_TYPE_ENT         packetType,
   IN  GT_U32                              subProfileId,
   IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum,
   OUT GT_BOOL                             *enableExactMatchLookupPtr,
   OUT GT_U32                              *profileIdPtr
)
{
    #ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT       dxChPacketType;

    /* convert generic into device specific packetType */
    PRV_TGF_S2D_PCL_UDB_PACKET_TYPE_CONVERT_MAC(dxChPacketType, packetType);

    rc = cpssDxChExactMatchPclProfileIdGet(devNum, direction, dxChPacketType,subProfileId,
                                           (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)pclLookupNum,
                                           enableExactMatchLookupPtr,profileIdPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchPclProfileIdGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(packetType);
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(subProfileId);
    TGF_PARAM_NOT_USED(pclLookupNum);
    TGF_PARAM_NOT_USED(enableExactMatchLookupPtr);
    TGF_PARAM_NOT_USED(profileIdPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfConvertGenericToDxChExactMatchKeyParams
*           function
* @endinternal
*
* @brief   Convert generic Exact Match key params into
*          device specific Exact Match key params
*
* @param[in] keyParamsPtr       - (pointer to) Exact Match
*       key params
* @param[out] dxChKeyParamsPtr  - (pointer to) DxCh Exact Match
*       key params
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChExactMatchKeyParams
(
    IN  PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      *keyParamsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *dxChKeyParamsPtr
)
{
    GT_U32 i=0;

    cpssOsMemSet((GT_VOID*) dxChKeyParamsPtr, 0, sizeof(*dxChKeyParamsPtr));

    PRV_TGF_S2D_KEY_SIZE_CONVERT_MAC(dxChKeyParamsPtr->keySize,keyParamsPtr->keySize);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChKeyParamsPtr, keyParamsPtr, keyStart);

    for(i=0;i<PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
    {
        dxChKeyParamsPtr->mask[i]= keyParamsPtr->mask[i];
    }

    return GT_OK;
}


/**
* @internal prvTgfConvertDxChToGenericExactMatchKeyParams
*           function
* @endinternal
*
* @brief   Convert specific Exact Match key params into device
*          generic Exact Match key params
*
* @param[in] dxChKeyParamsPtr  - (pointer to) DxCh Exact Match
*       key params
* @param[out] keyParamsPtr     - (pointer to) Exact Match key
*       params
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericExactMatchKeyParams
(
    IN  CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *dxChKeyParamsPtr,
    OUT PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      *keyParamsPtr
)
{
    GT_U32 i=0;

    cpssOsMemSet((GT_VOID*) keyParamsPtr, 0, sizeof(*keyParamsPtr));

    PRV_TGF_D2S_KEY_SIZE_CONVERT_MAC(dxChKeyParamsPtr->keySize,keyParamsPtr->keySize);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChKeyParamsPtr, keyParamsPtr, keyStart);

    for(i=0;i<PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
    {
        keyParamsPtr->mask[i]=dxChKeyParamsPtr->mask[i];
    }

    return GT_OK;
}

#endif /* CHX_FAMILY */

/**
* @internal prvTgfExactMatchClientLookupSet function
* @endinternal
*
* @brief   Set global configuration to determine the client of
*          the first EM lookup and the client of the second EM
*          lookup.
*
*   NOTE: this API should be called before configuring
*      Exact Match Profile Id for TTI keyType (cpssDxChExactMatchTtiProfileIdSet)or
*      Exact Match Profile Id for PCL/EPCL packet type (cpssDxChExactMatchPclProfileIdSet)
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientType           - client type (TTI/EPCL/IPCL)
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfExactMatchClientLookupSet
(
    IN  GT_U8                                     devNum,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  PRV_TGF_EXACT_MATCH_CLIENT_ENT            clientType
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            dxChClientType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            dxChExactMatchLookupNum;

    PRV_TGF_S2D_CLIENT_TYPE_CONVERT_MAC(dxChClientType,clientType);
    PRV_TGF_S2D_LOOKUP_NUM_CONVERT_MAC(dxChExactMatchLookupNum, exactMatchLookupNum);

    rc = cpssDxChExactMatchClientLookupSet(devNum, dxChExactMatchLookupNum, dxChClientType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchClientLookupSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchLookupNum);
    TGF_PARAM_NOT_USED(clientType);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchClientLookupGet function
* @endinternal
*
* @brief   Get global configuration that determine the client of
*          the first EM lookup and the client of the second EM
*          lookup.
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientTypePtr        - (pointer to)client type
*                                 (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfExactMatchClientLookupGet
(
    IN  GT_U8                                     devNum,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT  PRV_TGF_EXACT_MATCH_CLIENT_ENT           *clientTypePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            dxChClientType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            dxChExactMatchLookupNum;

    PRV_TGF_S2D_LOOKUP_NUM_CONVERT_MAC(dxChExactMatchLookupNum, exactMatchLookupNum);

    rc = cpssDxChExactMatchClientLookupGet(devNum, dxChExactMatchLookupNum, &dxChClientType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchClientLookupGet FAILED, rc = [%d]", rc);
    }

    PRV_TGF_D2S_CLIENT_TYPE_CONVERT_MAC(dxChClientType,(*clientTypePtr));

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchLookupNum);
    TGF_PARAM_NOT_USED(clientTypePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchActivityBitEnableSet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @param[in] devNum              - device number
* @param[in] enable              - GT_TRUE - enable refreshing
*                                  GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchActivityBitEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    rc = cpssDxChExactMatchActivityBitEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchActivityBitEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchActivityBitEnableGet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @param[in] devNum             - device number
* @param[in] enablePtr          - (pointer to)
* @param[in] enablePtr          - (pointer to)
*                               GT_TRUE - enable refreshing
*                               GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchActivityBitEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChExactMatchActivityBitEnableGet(devNum,enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchActivityBitEnableGet FAILED rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchActivityStatusGet function
* @endinternal
*
* @brief   Return the Exact Match activity bit for a given
*          entry.The bit is set by the device when the entry is
*          matched in the EM Lookup. The bit is reset by the CPU
*          as part of the aging process.
*
* @param[in]  devNum             - device number
* @param[in] portGroupsBmp       - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in]  exactMatchEntryIndex - EM entry index
* @param[in]  exactMatchClearActivity - set activity bit to 0
* @param[out] exactMatchActivityStatusPtr  - (pointer to)
*                  GT_FALSE = Not Refreshed; next_age_pass;
*                  Entry was matched since the last reset;
*                  GT_TRUE = Refreshed; two_age_pass; Entry was
*                  not matched since the last reset;
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchActivityStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  exactMatchEntryIndex,
    IN  GT_BOOL                 exactMatchClearActivity,
    OUT GT_BOOL                 *exactMatchActivityStatusPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,exactMatchEntryIndex,
                                             exactMatchClearActivity,exactMatchActivityStatusPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchActivityStatusGet FAILED rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(exactMatchEntryIndex);
    TGF_PARAM_NOT_USED(exactMatchClearActivity);
    TGF_PARAM_NOT_USED(exactMatchActivityStatusPtr);
    return GT_BAD_STATE;
#endif
}
/**
* @internal prvTgfExactMatchProfileKeyParamsSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile key parameters
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match
*            profile identifier (APPLICABLE RANGES: 1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileKeyParamsSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       exactMatchProfileIndex,
    IN PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC   *keyParamsPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC     dxChKeyParams;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChKeyParams, 0, sizeof(dxChKeyParams));

    rc = prvTgfConvertGenericToDxChExactMatchKeyParams(keyParamsPtr, &dxChKeyParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChKeyPatrams FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &dxChKeyParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchProfileKeyParamsSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchProfileIndex);
    TGF_PARAM_NOT_USED(keyParamsPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchProfileKeyParamsGet function
* @endinternal
*
* @brief   Gets the Exact Match Profile key parameters
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                 identifier (APPLICABLE RANGES:1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileKeyParamsGet
(
    IN  GT_U8                                         devNum,
    IN  GT_U32                                        exactMatchProfileIndex,
    OUT PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    dxChKeyParams;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChKeyParams, 0, sizeof(dxChKeyParams));

    rc = cpssDxChExactMatchProfileKeyParamsGet(devNum,exactMatchProfileIndex,&dxChKeyParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchProfileKeyParamsGet FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertDxChToGenericExactMatchKeyParams(&dxChKeyParams,keyParamsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericExactMatchKeyParams FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchProfileIndex);
    TGF_PARAM_NOT_USED(keyParamsPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchProfileDefaultActionSet function
* @endinternal
*
* @brief   Sets the default Action in case there is no match in the
*          Exact Match lookup and in the TCAM lookup
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                    identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEn        - Enable using Profile
*                               Table default Action in case
*                               there is no match in the Exact
*                               Match lookup and in the TCAM
*                               lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileDefaultActionSet
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 exactMatchProfileIndex,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN PRV_TGF_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                defActionEn
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    dxChActionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         dxChActionData;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChActionData, 0, sizeof(dxChActionData));

    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChActionType, actionType);

    if (defActionEn==GT_TRUE)
    {
        switch (actionType)
        {
        case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
            /* convert tti action type2 into device specific format */
            rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(actionPtr->ttiAction), &dxChActionData.ttiAction);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
        case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
            rc = prvTgfConvertGenericToDxChRuleAction(&(actionPtr->pclAction), &dxChActionData.pclAction);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        default:
            return GT_BAD_PARAM;
        }
    }

    rc = cpssDxChExactMatchProfileDefaultActionSet(devNum, exactMatchProfileIndex,
                                                   dxChActionType, &dxChActionData,
                                                   defActionEn);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchProfileDefaultActionSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchProfileIndex);
    TGF_PARAM_NOT_USED(actionType);
    TGF_PARAM_NOT_USED(actionPtr);
    TGF_PARAM_NOT_USED(defActionEn);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchProfileDefaultActionGet function
* @endinternal
*
* @brief   Gets the default Action in case there is no match in
*          the Exact Match lookup and in the TCAM lookup
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                     identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[out] actionPtr         - (pointer to)Exact Match Action
* @param[out] defActionEnPtr    - (pointer to) Enable using
*                               Profile Table default Action in
*                               case there is no match in the
*                               Exact Match lookup and in the
*                               TCAM lookup
*
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_BAD_PTR            - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileDefaultActionGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  exactMatchProfileIndex,
    IN  PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType,
    OUT PRV_TGF_EXACT_MATCH_ACTION_UNT          *actionPtr,
    OUT GT_BOOL                                 *defActionEnPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    dxChActionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         dxChActionData;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChActionData, 0, sizeof(dxChActionData));

    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChActionType, actionType);

    rc = cpssDxChExactMatchProfileDefaultActionGet(devNum,
                                                   exactMatchProfileIndex,
                                                   dxChActionType,
                                                   &dxChActionData,
                                                   defActionEnPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchProfileDefaultActionGet FAILED, rc = [%d]", rc);
    }

    switch(actionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        /* convert tti action type2 into device specific format */
        rc = prvTgfConvertDxChAction2ToGenericTtiAction2(&dxChActionData.ttiAction,&(actionPtr->ttiAction));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        rc = prvTgfConvertDxChToGenericRuleAction(&dxChActionData.pclAction,&(actionPtr->pclAction));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    default:
        return GT_BAD_PARAM;
    }


    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchProfileIndex);
    TGF_PARAM_NOT_USED(actionType);
    TGF_PARAM_NOT_USED(actionPtr);
    TGF_PARAM_NOT_USED(defActionEnPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfConvertGenericUseExpandedToDxChUseExpanded
*           function
* @endinternal
*
* @brief   Convert generic Expander flags into device specific
*          Expander flags
*
* @param[in] actionType       - TTI/PCL/EPCL
* @param[in] expandedActionOriginPtr       - (pointer to) expander flags
* @param[out] dxChExpandedActionOriginPtr  - (pointer to) dxCh expander flags
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericUseExpandedToDxChUseExpanded
(
    IN  PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT                 actionType,
    IN  PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT      *expandedActionOriginPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *dxChExpandedActionOriginPtr
)
{
    cpssOsMemSet((GT_VOID*) dxChExpandedActionOriginPtr, 0, sizeof(*dxChExpandedActionOriginPtr));

    switch(actionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCommand);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionUserDefinedCpuCode);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMirrorToIngressAnalyzerIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRedirectCommand);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionEgressInterface);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionVrfId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionFlowId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRouterLttPtr);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIpclUdbConfigTable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionVntl2Echo);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelStart);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionArpPtr);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPcl0OverrideConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPcl1OverrideConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPcl2OverrideConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIpclConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTsPassengerPacketType);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyMacDa);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyMacSa);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBindToCentralCounter);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCentralCounterIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBindToPolicerMeter);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBindToPolicer);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPolicerIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceIdSetEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionActionStop);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBridgeBypass);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIngressPipeBypass);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0VlanPrecedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionNestedVlanEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0VlanCmd);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0VlanId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1VlanCmd);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1VlanId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosPrecedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosProfile);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyDscp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyTag0);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionKeepPreviousQoS);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTrustUp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTrustDscp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTrustExp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRemapDscp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0Up);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1UpCommand);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1Up);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTtPassengerPacketType);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCopyTtlExpFromTunnelHeader);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelTerminate);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMplsCommand);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionHashMaskIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosMappingTableIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMplsLLspQoSProfileEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMplsTtl);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionEnableDecrementTtl);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceEPortAssignmentEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceEPort);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTtHeaderLength);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIsPtpPacket);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamTimeStampEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPtpTriggerType);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPtpOffset);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamOffsetIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamProcessEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCwBasedPw);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTtlExpiryVccvEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPwe3FlowLabelExist);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPwCwBasedETreeEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosUseUpAsIndexEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRxIsProtectionPath);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRxProtectionSwitchEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSetMacToMe);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamProfile);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionApplyNonDataCwCommand);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionContinueToNextTtiLookup);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSkipFdbSaLookupEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelHeaderStartL4Enable);
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUserDefinedCpuCode);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPktCmd);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionRedirectCommand);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionEgressInterface);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMacSa);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionRouterLttPtr);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVntl2Echo);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTunnelStart);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionArpPtr);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTunnelIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTsPassengerPacketType);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVrfId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionActionStop);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSetMacToMe);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPCL1OverrideConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPCL2OverrideConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionIPCLConfigurationIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPolicerEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPolicerIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMirrorMode);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMirror);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyMacDA);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyMacSA);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionBypassBridge);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionBypassIngressPipe);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionNestedVlanEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlanPrecedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan0Command);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan0);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan1Command);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan1);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourceIdEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourceId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMatchCounterEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMatchCounterIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQosProfileMakingEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQosPrecedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQoSProfile);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQoSModifyDSCP);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUp1Command);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyUp1);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUp1);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionDscpExp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUp0);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyUp0);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamTimestampEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamOffsetIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamProcessingEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamProfile);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionFlowId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourcePortEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourcePort);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionLatencyMonitor);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionLatencyMonitorEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSkipFdbSaLookup);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTriggerInterrupt);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaMetadataAssignEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaMetadata);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaThreadNumberAssignEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaThreadNumber);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionCutThroughTerminateId);
        break;
    default:
        return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal prvTgfConvertDxChUseExpandedToGenericUseExpanded
*           function
* @endinternal
*
* @brief   Convert device specific Expander flags into generic
*          Expander flags
*
* @param[in]  actionType                   - TTI/PCL/EPCL
* @param[in]  dxChExpandedActionOriginPtr  - (pointer to) dxCh expander flags
* @param[out] expandedActionOriginPtr      - (pointer to) expander flags
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertDxChUseExpandedToGenericUseExpanded
(
    IN  PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT                 actionType,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *dxChExpandedActionOriginPtr,
    OUT PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT      *expandedActionOriginPtr
)
{
    cpssOsMemSet((GT_VOID*) expandedActionOriginPtr, 0, sizeof(*expandedActionOriginPtr));

    switch(actionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCommand);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionUserDefinedCpuCode);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMirrorToIngressAnalyzerIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRedirectCommand);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionEgressInterface);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionVrfId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionFlowId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRouterLttPtr);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIpclUdbConfigTable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionVntl2Echo);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelStart);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionArpPtr);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPcl0OverrideConfigIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPcl1OverrideConfigIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPcl2OverrideConfigIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIpclConfigIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTsPassengerPacketType);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyMacDa);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyMacSa);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBindToCentralCounter);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCentralCounterIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBindToPolicerMeter);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBindToPolicer);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPolicerIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceIdSetEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionActionStop);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionBridgeBypass);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIngressPipeBypass);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0VlanPrecedence);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionNestedVlanEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0VlanCmd);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0VlanId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1VlanCmd);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1VlanId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosPrecedence);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosProfile);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyDscp);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionModifyTag0);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionKeepPreviousQoS);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTrustUp);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTrustDscp);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTrustExp);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRemapDscp);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag0Up);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1UpCommand);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTag1Up);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTtPassengerPacketType);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCopyTtlExpFromTunnelHeader);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelTerminate);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMplsCommand);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionHashMaskIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosMappingTableIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMplsLLspQoSProfileEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionMplsTtl);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionEnableDecrementTtl);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceEPortAssignmentEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSourceEPort);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTtHeaderLength);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIsPtpPacket);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamTimeStampEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPtpTriggerType);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPtpOffset);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamOffsetIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamProcessEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionCwBasedPw);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTtlExpiryVccvEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPwe3FlowLabelExist);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPwCwBasedETreeEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionQosUseUpAsIndexEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRxIsProtectionPath);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionRxProtectionSwitchEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSetMacToMe);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamProfile);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionApplyNonDataCwCommand);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionContinueToNextTtiLookup);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionSkipFdbSaLookupEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->ttiExpandedActionOrigin, &expandedActionOriginPtr->ttiExpandedActionOrigin, overwriteExpandedActionTunnelHeaderStartL4Enable);
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUserDefinedCpuCode);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPktCmd);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionRedirectCommand);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionEgressInterface);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMacSa);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionRouterLttPtr);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVntl2Echo);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTunnelStart);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionArpPtr);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTunnelIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTsPassengerPacketType);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVrfId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionActionStop);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSetMacToMe);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPCL1OverrideConfigIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPCL2OverrideConfigIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionIPCLConfigurationIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPolicerEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPolicerIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMirrorMode);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMirror);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyMacDA);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyMacSA);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionBypassBridge);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionBypassIngressPipe);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionNestedVlanEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlanPrecedence);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan0Command);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan0);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan1Command);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionVlan1);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourceIdEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourceId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMatchCounterEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionMatchCounterIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQosProfileMakingEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQosPrecedence);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQoSProfile);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionQoSModifyDSCP);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUp1Command);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyUp1);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUp1);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionDscpExp);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionUp0);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionModifyUp0);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamTimestampEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamOffsetIndex);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamProcessingEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamProfile);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionFlowId);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourcePortEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSourcePort);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionLatencyMonitor);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionLatencyMonitorEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionSkipFdbSaLookup);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionTriggerInterrupt);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaMetadataAssignEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaMetadata);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaThreadNumberAssignEnable);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionPhaThreadNumber);
        PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChExpandedActionOriginPtr->pclExpandedActionOrigin, &expandedActionOriginPtr->pclExpandedActionOrigin, overwriteExpandedActionCutThroughTerminateId);
        break;
    default:
        return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal prvTgfExactMatchExpandedActionSet function
* @endinternal
*
* @brief  Sets the expanded action for Exact Match Profile. The
*         API also sets for each action attribute whether to
*         take it from the Exact Match entry action or from
*         the Exact Match profile
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: This API must be called before configuration of
*         Exact Match Entry by API cpssDxChExactMatchEntrySet
*
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*/
GT_STATUS prvTgfExactMatchExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT              actionType,
    IN PRV_TGF_EXACT_MATCH_ACTION_UNT                   *actionPtr,
    IN PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   *expandedActionOriginPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               dxChActionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    dxChActionData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    dxChExpandedActionOrigin;


    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChActionData, 0, sizeof(dxChActionData));
    cpssOsMemSet((GT_VOID*) &dxChExpandedActionOrigin, 0, sizeof(dxChExpandedActionOrigin));

    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChActionType, actionType);

    switch(actionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        /* convert tti action type2 into device specific format */
        rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(actionPtr->ttiAction), &dxChActionData.ttiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        rc = prvTgfConvertGenericToDxChRuleAction(&(actionPtr->pclAction), &dxChActionData.pclAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    default:
        return GT_BAD_PARAM;
    }

    rc = prvTgfConvertGenericUseExpandedToDxChUseExpanded(actionType,expandedActionOriginPtr,&dxChExpandedActionOrigin);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericUseExpandedToDxChUseExpanded FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChExactMatchExpandedActionSet(devNum, expandedActionIndex,
                                             dxChActionType, &dxChActionData,
                                             &dxChExpandedActionOrigin);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchExpandedActionSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchProfileIndex);
    TGF_PARAM_NOT_USED(actionType);
    TGF_PARAM_NOT_USED(actionPtr);
    TGF_PARAM_NOT_USED(expandedActionOriginPtr);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchExpandedActionGet function
* @endinternal
*
* @brief   Gets the action for Exact Match in case of a match in
*          Exact Match lookup The API also sets for each action
*          attribute whether to take it from the Exact Match
*          entry action or from the Exact Match profile
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         -(pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchExpandedActionGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT              actionType,
    OUT PRV_TGF_EXACT_MATCH_ACTION_UNT                  *actionPtr,
    OUT PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  *expandedActionOriginPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               dxChActionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    dxChActionData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    dxChExpandedActionOrigin;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChActionData, 0, sizeof(dxChActionData));
    cpssOsMemSet((GT_VOID*) &dxChExpandedActionOrigin, 0, sizeof(dxChExpandedActionOrigin));

    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChActionType, actionType);

    rc = cpssDxChExactMatchExpandedActionGet(devNum,
                                             expandedActionIndex,
                                             dxChActionType,
                                             &dxChActionData,
                                             &dxChExpandedActionOrigin);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchExpandedActionGet FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertDxChUseExpandedToGenericUseExpanded(actionType,&dxChExpandedActionOrigin, expandedActionOriginPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChUseExpandedToGenericUseExpanded FAILED, rc = [%d]", rc);
    }

    switch(actionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        /* convert tti action type2 into device specific format */
        rc = prvTgfConvertDxChAction2ToGenericTtiAction2(&dxChActionData.ttiAction,&(actionPtr->ttiAction));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        rc = prvTgfConvertDxChToGenericRuleAction(&dxChActionData.pclAction,&(actionPtr->pclAction));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    default:
        return GT_BAD_PARAM;
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchProfileIndex);
    TGF_PARAM_NOT_USED(actionType);
    TGF_PARAM_NOT_USED(actionPtr);
    TGF_PARAM_NOT_USED(expandedActionOriginPtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal
*           prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry
*           function
* @endinternal
*
* @brief   Convert generic exact match into device specific
*          exact match
*
* @param[in]  entryPtr     - (pointer to) exact match entry
* @param[out] dxChEntryPtr - (pointer to) dxCh exact match entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry
(
    IN  PRV_TGF_EXACT_MATCH_ENTRY_STC       *entryPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC     *dxChEntryPtr
)
{
    cpssOsMemSet((GT_VOID*) dxChEntryPtr, 0, sizeof(*dxChEntryPtr));

    switch (entryPtr->lookupNum)
    {
    case PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E:
        dxChEntryPtr->lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        break;
    case PRV_TGF_EXACT_MATCH_LOOKUP_SECOND_E:
        dxChEntryPtr->lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch (entryPtr->key.keySize)
    {
    case PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E:
        dxChEntryPtr->key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        break;
    case PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E:
        dxChEntryPtr->key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        break;
    case PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E:
        dxChEntryPtr->key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
        break;
    case PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E:
        dxChEntryPtr->key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    cpssOsMemCpy(dxChEntryPtr->key.pattern,entryPtr->key.pattern, sizeof((entryPtr->key.pattern)));

    return GT_OK;
}
/**
* @internal
*           prvTgfConvertDxChExactMatchEntryToGenericExactMatchEntry
*           function
* @endinternal
*
* @brief   Convert specific exact match into device generic
*          exact match
*
* @param[in]  dxChEntryPtr - (pointer to) dxCh exact match entry
* @param[out] entryPtr     - (pointer to) exact match entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertDxChExactMatchEntryToGenericExactMatchEntry
(
    IN  CPSS_DXCH_EXACT_MATCH_ENTRY_STC   *dxChEntryPtr,
    OUT PRV_TGF_EXACT_MATCH_ENTRY_STC     *entryPtr
)
{
    cpssOsMemSet((GT_VOID*) entryPtr, 0, sizeof(*entryPtr));

    switch (dxChEntryPtr->lookupNum)
    {
    case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
        entryPtr->lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
        break;
    case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
        entryPtr->lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_SECOND_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch (dxChEntryPtr->key.keySize)
    {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        entryPtr->key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        entryPtr->key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        entryPtr->key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        entryPtr->key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    cpssOsMemCpy(entryPtr->key.pattern, dxChEntryPtr->key.pattern, sizeof((entryPtr->key.pattern)));

    return GT_OK;
}


/**
* @internal prvTgfExactMatchPortGroupEntrySet function
* @endinternal
*
* @brief   Sets the exact match entry and its action
*
*   NOTE: this API should be called when there is a valid entry
*   in expandedActionProfile in the Profile Expander Table
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the exact match table
* @param[in] expandedActionIndex-Exact Match profile identifier
*                     (APPLICABLE RANGES:1..15)
* @param[in] entryPtr      - (pointer to)Exact Match entry
* @param[in] actionType    - Exact Match Action Type(TTI or PCL)
* @param[in] actionPtr     - (pointer to)Exact Match Action (TTI
*                            Action or PCL Action)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPortGroupEntrySet
(
    IN GT_U8                                  devNum,
    IN GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN GT_U32                                 index,
    IN GT_U32                                 expandedActionIndex,
    IN PRV_TGF_EXACT_MATCH_ENTRY_STC          *entryPtr,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN PRV_TGF_EXACT_MATCH_ACTION_UNT         *actionPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC           dxChEntryData;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT     dxChActionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT          dxChActionData;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChEntryData, 0, sizeof(dxChEntryData));
    cpssOsMemSet((GT_VOID*) &dxChActionData, 0, sizeof(dxChActionData));


    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChActionType, actionType);

    switch(actionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        /* convert tti action type2 into device specific format */
        rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(actionPtr->ttiAction), &dxChActionData.ttiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        rc = prvTgfConvertGenericToDxChRuleAction(&(actionPtr->pclAction), &dxChActionData.pclAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    default:
        return GT_BAD_PARAM;
    }

    rc = prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry(entryPtr,&dxChEntryData);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChExactMatchPortGroupEntrySet(devNum,
                                             portGroupsBmp,
                                             index,
                                             expandedActionIndex,
                                             &dxChEntryData,
                                             dxChActionType,
                                             &dxChActionData);


    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchPortGroupEntrySet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(expandedActionIndex);
    TGF_PARAM_NOT_USED(entryPtr);
    TGF_PARAM_NOT_USED(actionType);
    TGF_PARAM_NOT_USED(actionPtr);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchPortGroupEntryGet function
* @endinternal
*
* @brief   Gets the exact match entry and its action
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                 NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] index - entry index in the exact match table
* @param[out]validPtr   - (pointer to) is the entry valid or not
* @param[in] actionType - Exact Match Action Type(TTI or PCL)
* @param[out] actionPtr - (pointer to)Exact Match Action
*                         (TTI Action or PCL Action)
* @param[out] entryPtr  - (pointer to)Exact Match entry
* @param[out] expandedActionIndexPtr-(pointer to)Exact Match
*             profile identifier (APPLICABLE RANGES:1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPortGroupEntryGet
(
    IN GT_U8                                  devNum,
    IN GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN GT_U32                                 index,
    OUT GT_BOOL                               *validPtr,
    OUT PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT PRV_TGF_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT PRV_TGF_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                *expandedActionIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC                 dxChEntryData;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           dxChActionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                dxChActionData;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChActionData, 0, sizeof(dxChActionData));
    cpssOsMemSet((GT_VOID*) &dxChEntryData, 0, sizeof(dxChEntryData));

    rc = cpssDxChExactMatchPortGroupEntryGet(devNum,
                                             portGroupsBmp,
                                             index,
                                             validPtr,
                                             &dxChActionType,
                                             &dxChActionData,
                                             &dxChEntryData,
                                             expandedActionIndexPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchPortGroupEntryGet FAILED, rc = [%d]", rc);
    }

    if (*validPtr==GT_TRUE)
    {
        PRV_TGF_D2S_ACTION_TYPE_CONVERT_MAC(dxChActionType, (*actionTypePtr));

        rc = prvTgfConvertDxChExactMatchEntryToGenericExactMatchEntry(&dxChEntryData, entryPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChExactMatchEntryToGenericExactMatchEntry FAILED, rc = [%d]", rc);
        }

        switch(*actionTypePtr)
        {
        case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
            /* convert tti action type2 into device specific format */
            rc = prvTgfConvertDxChAction2ToGenericTtiAction2(&dxChActionData.ttiAction,&(actionPtr->ttiAction));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
        case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
            rc = prvTgfConvertDxChToGenericRuleAction(&dxChActionData.pclAction,&(actionPtr->pclAction));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRuleAction FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        default:
            return GT_BAD_PARAM;
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(expandedActionIndex);
    TGF_PARAM_NOT_USED(entryPtr);
    TGF_PARAM_NOT_USED(actionType);
    TGF_PARAM_NOT_USED(actionPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchPortGroupEntryInvalidate function
* @endinternal
*
* @brief   Invalidate the exact match entry
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the exact match table
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPortGroupEntryInvalidate
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN GT_U32                               index
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,
                                             portGroupsBmp,
                                             index);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchPortGroupEntryInvalidate FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(index);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                           - device number
* @param[in] entryKeyPtr                      - (pointer to) entry key
* @param[in] numberOfElemInCrcMultiHashArrPtr - (pointer to) number of valid
*                                                elements in the exactMatchCrcMultiHashArr
* @param[out] exactMatchCrcMultiHashArr[]     - (array of) 'multi hash' CRC results.
*                                               index in this array is entry inside the bank
*                                               + bank Id'
*                                               size of exactMatchCrcMultiHashArr must be 16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW,
*       and do only SW calculations.
*/
GT_STATUS prvTgfExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_EXACT_MATCH_KEY_STC     *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                          exactMatchCrcMultiHashArr[]
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U32    ii;
    CPSS_DXCH_EXACT_MATCH_KEY_STC       dxChEntryKey;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChEntryKey, 0, sizeof(dxChEntryKey));

    PRV_TGF_S2D_KEY_SIZE_CONVERT_MAC(dxChEntryKey.keySize,entryKeyPtr->keySize);

    for(ii=0;ii<PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS;ii++)
    {
        dxChEntryKey.pattern[ii]= entryKeyPtr->pattern[ii];
    }

    rc = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &dxChEntryKey, numberOfElemInCrcMultiHashArrPtr,&exactMatchCrcMultiHashArr[0]);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfExactMatchHashCrcMultiResultsByParamsCalc FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exactMatchProfileIndex);
    TGF_PARAM_NOT_USED(keyParamsPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchCheck function
* @endinternal
*
* @brief   check if the device supports Exact Match
*
* @retval GT_TRUE         - the device supports Exact Match
* @retval GT_FALSE        - the device not supports Exact Match
*/
GT_BOOL prvTgfExactMatchCheck(
    void
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum = prvTgfDevNum;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum == 0)
    {
        /* the device do not support Exact Match */
        return GT_FALSE;
    }

    return GT_TRUE;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_TRUE;
#endif /* !(defined CHX_FAMILY) */
}
