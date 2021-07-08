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
* @file tgfLpmGen.c
*
* @brief Generic API implementation for LPM
*
* @version   2
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

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfLpmGen.h>

#ifdef CHX_FAMILY

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>

#endif

/**
* @internal prvTgfLpmLastLookupStagesBypassEnableSet function
* @endinternal
*
* @brief   Set the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
* @param[in] devNum                   - the device number
* @param[in] bypassEnabled            - the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS prvTgfLpmLastLookupStagesBypassEnableSet
(
    IN GT_U8                                    devNum,
    IN GT_BOOL                                  bypassEnabled
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChLpmLastLookupStagesBypassEnableSet(devNum, bypassEnabled);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLpmLastLookupStagesBypassEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfLpmLastLookupStagesBypassEnableGet function
* @endinternal
*
* @brief   Get the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
* @param[in] devNum                   - the device number
*
* @param[out] bypassEnabledPtr         - (pointer to) the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS prvTgfLpmLastLookupStagesBypassEnableGet
(
    IN  GT_U8                                   devNum,
    OUT GT_BOOL                                 *bypassEnabledPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChLpmLastLookupStagesBypassEnableGet(devNum, bypassEnabledPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLpmLastLookupStagesBypassEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfLpmPortSipLookupEnableSet function
* @endinternal
*
* @brief   Enable/Disable SIP lookup for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] enable                   - GT_TRUE:    SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value set
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
GT_STATUS prvTgfLpmPortSipLookupEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChLpmPortSipLookupEnableSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLpmPortSipLookupEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfLpmPortSipLookupEnableGet function
* @endinternal
*
* @brief   Return the SIP Lookup status for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:   enable SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
GT_STATUS prvTgfLpmPortSipLookupEnableGet
(
   IN  GT_U8                        devNum,
   IN  GT_PORT_NUM                  portNum,
   OUT GT_BOOL                      *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChLpmPortSipLookupEnableGet(devNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLpmPortSipLookupEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfLpmExceptionStatusGet function
* @endinternal
*
* @brief   Return the Lpm Exception Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmEngine                - LPM engine index (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon )(APPLICABLE RANGES: 0..3)
*
* @param[out] hitExceptionPtr          - (pointer to) hit Exception happened in LPM Stages
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_STATE             - wrong value returned from hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/

GT_STATUS prvTgfLpmExceptionStatusGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                             lpmEngine,
    OUT PRV_TGF_LPM_EXCEPTION_STATUS_ENT  *hitExceptionPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT dxChHitException;

    /* call device specific API */
    rc = cpssDxChLpmExceptionStatusGet(devNum,lpmEngine, &dxChHitException);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLpmExceptionStatusGet FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(dxChHitException)
    {
    case CPSS_DXCH_LPM_EXCEPTION_HIT_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_HIT_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_LPM_ECC_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_LPM_ECC_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_ECMP_ECC_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_ECMP_ECC_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_PBR_BUCKET_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_PBR_BUCKET_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_CONTINUE_TO_LOOKUP_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_CONTINUE_TO_LOOKUP_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_0_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_1_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV4_PACKETS_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_DST_G_IPV4_PACKETS_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV4_PACKETS_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_SRC_G_IPV4_PACKETS_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_IPV6_PACKETS_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_0_IPV6_PACKETS_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_IPV6_PACKETS_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_1_IPV6_PACKETS_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV6_PACKETS_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_DST_G_IPV6_PACKETS_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV6_PACKETS_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_SRC_G_IPV6_PACKETS_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_FCOE_D_ID_LOOKUP_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_FCOE_D_ID_LOOKUP_E;
        break;
    case CPSS_DXCH_LPM_EXCEPTION_FCOE_S_ID_LOOKUP_E:
        *hitExceptionPtr = PRV_TGF_LPM_EXCEPTION_FCOE_S_ID_LOOKUP_E;
        break;
    default:
        return GT_BAD_STATE;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfLpmEngineGet function
* @endinternal
*
* @brief   Get LPM engine
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number
* @param[in] isSipLookup              - GT_TRUE if this is SIP lookup,GT_FALSE otherwise
*
* @param[out] lpmEnginePtr             - lpm engine number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_VALUE             - failure to calculate lpm engine
*/
GT_STATUS prvTgfLpmEngineGet
(
   IN  GT_U8                        devNum,
   IN  GT_PORT_NUM                  portNum,
   IN  GT_BOOL                      isSipLookup,
   OUT GT_U32                        *lpmEnginePtr
)
{
   GT_STATUS rc = GT_OK;
   GT_U32 globalMacPortNum;
   GT_U32 pipeId;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, globalMacPortNum);
    rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum, globalMacPortNum,& pipeId , NULL);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert FAILED, rc = [%d]", rc);

        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *lpmEnginePtr = 0;/* need to be PRV_CPSS_DXCH_LPM_FALCON_ENGINE_NUM_CNS-1 */
        return GT_OK;
    }


    switch(pipeId)
    {
        case 0:
            if(isSipLookup == GT_TRUE)
            {
                *lpmEnginePtr = 1;
            }
            else
            {
                *lpmEnginePtr = 0;
            }
         break;
        case 1:
            if(isSipLookup == GT_TRUE)
            {
                *lpmEnginePtr = 3;
            }
            else
            {
                *lpmEnginePtr = 2;
            }
         break;
        default:
         PRV_UTF_LOG1_MAC("[TGF]: prvTgfLpmEngineGet : illegal pipe [%d]", pipeId);
         rc= GT_BAD_VALUE;
        break;
    }

    return rc;
}




