/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfL2mllGen.c
*
* DESCRIPTION:
*       Generic API implementation for L2 MLL
*
* FILE REVISION NUMBER:
*       $Revision: 17 $
*
*******************************************************************************/
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

#ifdef CHX_FAMILY

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>

/**
* @internal prvTgfConvertGenericToDxChL2MllLttEntry function
* @endinternal
*
* @brief   Convert generic into device specific L2 MLL LTT entry
*
* @param[in] l2MllLttEntryPtr         - (pointer to) L2 MLL LTT entry
*
* @param[out] dxChL2MllLttEntryPtr     - (pointer to) DxCh L2 MLL LTT entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChL2MllLttEntry
(
    IN  PRV_TGF_L2_MLL_LTT_ENTRY_STC    *l2MllLttEntryPtr,
    OUT CPSS_DXCH_L2_MLL_LTT_ENTRY_STC  *dxChL2MllLttEntryPtr
)
{
    cpssOsMemSet(dxChL2MllLttEntryPtr, 0, sizeof(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC));

    switch (l2MllLttEntryPtr->entrySelector)
    {
    case PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E:
        dxChL2MllLttEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
        break;
    case PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E:
        dxChL2MllLttEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2MllLttEntryPtr, l2MllLttEntryPtr, mllPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2MllLttEntryPtr, l2MllLttEntryPtr, mllMaskProfileEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2MllLttEntryPtr, l2MllLttEntryPtr, mllMaskProfile);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericL2MllLttEntry function
* @endinternal
*
* @brief   Convert device specific L2 MLL LTT entry into generic
*
* @param[in] dxChL2MllLttEntryPtr     - (pointer to) DxCh L2 MLL LTT entry parameters
*
* @param[out] l2MllLttEntryPtr         - (pointer to) L2 MLL LTT entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericL2MllLttEntry
(
    IN CPSS_DXCH_L2_MLL_LTT_ENTRY_STC  *dxChL2MllLttEntryPtr,
    OUT PRV_TGF_L2_MLL_LTT_ENTRY_STC    *l2MllLttEntryPtr

)
{
    cpssOsMemSet(l2MllLttEntryPtr, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    /* convert entry info into generic format */

    switch (dxChL2MllLttEntryPtr->entrySelector)
    {
    case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E:
        l2MllLttEntryPtr->entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
        break;
    case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E:
        l2MllLttEntryPtr->entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2MllLttEntryPtr, l2MllLttEntryPtr, mllPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2MllLttEntryPtr, l2MllLttEntryPtr, mllMaskProfileEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2MllLttEntryPtr, l2MllLttEntryPtr, mllMaskProfile);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChL2MllPairEntry function
* @endinternal
*
* @brief   Convert generic into device specific L2 MLL entry
*
* @param[in] l2MllPairEntryPtr        - (pointer to) L2 MLL entry
*
* @param[out] dxChL2MllPairEntryPtr    - (pointer to) DxCh L2 MLL entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChL2MllPairEntry
(
    IN  PRV_TGF_L2_MLL_PAIR_STC     *l2MllPairEntryPtr,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC   *dxChL2MllPairEntryPtr
)
{
    GT_STATUS rc;

    cpssOsMemSet(dxChL2MllPairEntryPtr, 0, sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2MllPairEntryPtr,l2MllPairEntryPtr, nextPointer);

    switch(l2MllPairEntryPtr->entrySelector)
    {
        case PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E:
            dxChL2MllPairEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
            break;
        case PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E:
            dxChL2MllPairEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), last);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), unknownUcFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), unregMcFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), bcFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), meshId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), mcLocalSwitchingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), maxHopCountEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), maxOutgoingHopCount);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), egressInterface);
    rc = prvUtfHwDeviceNumberGet((GT_U8)l2MllPairEntryPtr->firstMllNode.egressInterface.devPort.hwDevNum,
                                 &(dxChL2MllPairEntryPtr->firstMllNode.egressInterface.devPort.hwDevNum));
    if(GT_OK != rc)
    {
        return rc;
    }
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), maskBitmap);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), ttlThreshold);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), bindToMllCounterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), mllCounterIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), onePlusOneFilteringEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), meshId);

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), last);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), unknownUcFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), unregMcFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), bcFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), meshId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), mcLocalSwitchingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), maxHopCountEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), maxOutgoingHopCount);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), egressInterface);
    rc = prvUtfHwDeviceNumberGet((GT_U8)l2MllPairEntryPtr->secondMllNode.egressInterface.devPort.hwDevNum,
                                 &(dxChL2MllPairEntryPtr->secondMllNode.egressInterface.devPort.hwDevNum));
    if(GT_OK != rc)
    {
        return rc;
    }
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), maskBitmap);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), ttlThreshold);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), bindToMllCounterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), mllCounterIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), onePlusOneFilteringEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), meshId);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericL2MllPairEntry function
* @endinternal
*
* @brief   Convert device specific L2 MLL entry into generic
*
* @param[in] dxChL2MllPairEntryPtr    - (pointer to) DxCh L2 MLL entry parameters
*
* @param[out] l2MllPairEntryPtr        - (pointer to) L2 MLL entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericL2MllPairEntry
(
    IN  CPSS_DXCH_L2_MLL_PAIR_STC *dxChL2MllPairEntryPtr,
    OUT PRV_TGF_L2_MLL_PAIR_STC   *l2MllPairEntryPtr
)
{
    GT_STATUS rc;

    cpssOsMemSet(l2MllPairEntryPtr, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    /* convert entry info into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2MllPairEntryPtr,l2MllPairEntryPtr, nextPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2MllPairEntryPtr,l2MllPairEntryPtr, entrySelector);

    switch(dxChL2MllPairEntryPtr->entrySelector)
    {
        case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E:
            l2MllPairEntryPtr->entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
            break;
        case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E:
            l2MllPairEntryPtr->entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert entry info into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), last);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), unknownUcFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), unregMcFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), bcFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), meshId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), mcLocalSwitchingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), maxHopCountEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), maxOutgoingHopCount);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), egressInterface);
    rc = prvUtfSwFromHwDeviceNumberGet(dxChL2MllPairEntryPtr->firstMllNode.egressInterface.devPort.hwDevNum,
                                       &(l2MllPairEntryPtr->firstMllNode.egressInterface.devPort.hwDevNum));
    if(GT_OK != rc)

    if(GT_OK != rc)
    {
        return rc;
    }
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), maskBitmap);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), ttlThreshold);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), bindToMllCounterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), mllCounterIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), onePlusOneFilteringEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->firstMllNode), &(l2MllPairEntryPtr->firstMllNode), meshId);

    /* convert entry info into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), last);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), unknownUcFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), unregMcFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), bcFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), meshId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), mcLocalSwitchingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), maxHopCountEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), maxOutgoingHopCount);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), egressInterface);
    rc = prvUtfSwFromHwDeviceNumberGet(dxChL2MllPairEntryPtr->secondMllNode.egressInterface.devPort.hwDevNum,
                                       &(l2MllPairEntryPtr->secondMllNode.egressInterface.devPort.hwDevNum));
    if(GT_OK != rc)
    {
        return rc;
    }
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), maskBitmap);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), ttlThreshold);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), bindToMllCounterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), mllCounterIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), onePlusOneFilteringEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChL2MllPairEntryPtr->secondMllNode), &(l2MllPairEntryPtr->secondMllNode), meshId);

    return GT_OK;
}

#endif

/**
* @internal prvTgfL2MllLookupForAllEvidxEnableSet function
* @endinternal
*
* @brief   Enable or disable MLL lookup for all multi-target packets.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: MLL lookup is performed for all multi-target packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllLookupForAllEvidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChL2MllLookupForAllEvidxEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllLookupForAllEvidxEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllLookupForAllEvidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for all multi-target packets.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup is performed for all multi-target
*                                      packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllLookupForAllEvidxEnableGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChL2MllLookupForAllEvidxEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllLookupForAllEvidxEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllLookupMaxVidxIndexSet function
* @endinternal
*
* @brief   Set the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
* @param[in] devNum                   - device number
* @param[in] maxVidxIndex             - maximal VIDX value.
*                                      When cpssDxChL2MllLookupForAllEvidxEnableSet == FALSE
*                                      L2 replication is performed to all eVidx > maxVidxIndex
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when prvTgfL2MllLookupForAllEvidxEnableSet == FALSE
*
*/
GT_STATUS prvTgfL2MllLookupMaxVidxIndexSet
(
    IN  GT_U8    devNum,
    IN GT_U32    maxVidxIndex
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChL2MllLookupMaxVidxIndexSet(devNum, maxVidxIndex);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllLookupMaxVidxIndexSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllLookupMaxVidxIndexGet function
* @endinternal
*
* @brief   Get the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
* @param[in] devNum                   - device number
*
* @param[out] maxVidxIndexPtr          - (pointer to) maximal VIDX value.
*                                      When <Enable MLL Lookup for All eVidx> == FALSE
*                                      L2 replication is performed to all eVidx > <Max VIDX Index>
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when prvTgfL2MllLookupForAllEvidxEnableGet== FALSE
*
*/
GT_STATUS prvTgfL2MllLookupMaxVidxIndexGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *maxVidxIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChL2MllLookupMaxVidxIndexGet(devNum, maxVidxIndexPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllLookupMaxVidxIndexGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfL2MllMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of Multicast packets Duplicated by the
*                                      L2 MLL Engine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllMcCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChL2MllMcCntGet(devNum, mllCntSet, mllOutMCPktsPtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllSetCntInterfaceCfg function
* @endinternal
*
* @brief   Sets a L2 mll counter set's bounded inteface.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllSetCntInterfaceCfg
(
    IN GT_U32                                    mllCntSet,
    IN PRV_TGF_L2MLL_COUNTER_SET_INTERFACE_CFG_STC *interfaceCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC  dxChInterfaceCfg;

    /* reset variable */
    cpssOsMemSet(&dxChInterfaceCfg, 0, sizeof(dxChInterfaceCfg));

    /* convert Port/Trunk mode into device specific format */
    switch (interfaceCfgPtr->portTrunkCntMode)
    {
        case PRV_TGF_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E;
            break;

        case PRV_TGF_L2MLL_PORT_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_PORT_CNT_MODE_E;
            dxChInterfaceCfg.portTrunk.port   = interfaceCfgPtr->portTrunk.port;
            break;

        case PRV_TGF_L2MLL_TRUNK_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E;
            dxChInterfaceCfg.portTrunk.trunk  = interfaceCfgPtr->portTrunk.trunk;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert the counter Set vlan mode into device specific format */
    switch (interfaceCfgPtr->vlanMode)
    {
        case PRV_TGF_L2MLL_DISREGARD_VLAN_CNT_MODE_E:
            dxChInterfaceCfg.vlanMode = CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E;
            break;

        case PRV_TGF_L2MLL_USE_VLAN_CNT_MODE_E:
            dxChInterfaceCfg.vlanMode = CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E;
            dxChInterfaceCfg.vlanId   = interfaceCfgPtr->vlanId;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert other fields */
    dxChInterfaceCfg.ipMode = interfaceCfgPtr->ipMode;
    dxChInterfaceCfg.hwDevNum = interfaceCfgPtr->devNum;
    rc = prvUtfHwDeviceNumberGet(interfaceCfgPtr->devNum,&(dxChInterfaceCfg.hwDevNum));
    if (rc != GT_OK)
    {
        return rc;
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChL2MllSetCntInterfaceCfg(devNum, mllCntSet, &dxChInterfaceCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllSetCntInterfaceCfg FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllSilentDropCntGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *silentDropPktsPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChL2MllSilentDropCntGet(devNum, silentDropPktsPtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllTtlExceptionConfigurationSet function
* @endinternal
*
* @brief   Set configuration for L2 MLL TTL Exceptions.
*
* @param[in] devNum                   - device number
* @param[in] trapEnable               - GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL exception
*                                      in the MLL.
* @param[in] cpuCode                  - CPU code of packets that are trapped to CPU due to a
*                                      TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllTtlExceptionConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  trapEnable,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChL2MllTtlExceptionConfigurationSet(devNum, trapEnable, cpuCode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllTtlExceptionConfigurationSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllTtlExceptionConfigurationGet function
* @endinternal
*
* @brief   Get configuration of L2 MLL TTL Exceptions.
*
* @param[in] devNum                   - device number
*
* @param[out] trapEnablePtr            - (pointer to)
*                                      GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL
*                                      exception in the MLL.
* @param[out] cpuCodePtr               - (pointer to) CPU code of packets that are trapped to
*                                      CPU due to a TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllTtlExceptionConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *trapEnablePtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChL2MllTtlExceptionConfigurationGet(devNum, trapEnablePtr, cpuCodePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllTtlExceptionConfigurationGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*
* @param[in] devNum                   - device number
*
* @param[out] countersPtr              - (pointer to) L2 MLL exception counters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllExceptionCountersGet
(
    IN  GT_U8                                  devNum,
    OUT PRV_TGF_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  dxChCounters, *dxChCountersPtr;

    dxChCountersPtr = &dxChCounters;
    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChL2MllExceptionCountersGet(devNum, dxChCountersPtr);
    }
    else
    {
        rc = cpssDxChL2MllPortGroupExceptionCountersGet(devNum, currPortGroupsBmp, dxChCountersPtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllExceptionCountersGet FAILED, rc = [%d]", rc);

        return rc;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChCountersPtr, countersPtr, skip);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChCountersPtr, countersPtr, ttl);

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*
* @param[in] devNum                   - device number
* @param[in] index                    - counter number, range: 0..2
*
* @param[out] counterPtr               - (pointer to) L2 MLL counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  index,
    OUT GT_U32  *counterPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChL2MllCounterGet(devNum, index, counterPtr);
    }
    else
    {
        rc = cpssDxChL2MllPortGroupCounterGet(devNum, currPortGroupsBmp, index, counterPtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllCounterGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllLttEntrySet function
* @endinternal
*
* @brief   Set L2 MLL Lookup Translation Table (LTT) entry.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
* @param[in] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllLttEntrySet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN PRV_TGF_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC      dxChLttEntry;

    /* convert L2 MLL LTT entry into device specific format */
    rc = prvTgfConvertGenericToDxChL2MllLttEntry(lttEntryPtr, &dxChLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChL2MllLttEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChL2MllLttEntrySet(devNum, index, &dxChLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllLttEntrySet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllLttEntryGet function
* @endinternal
*
* @brief   Get L2 MLL Lookup Translation Table (LTT) entry.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
*
* @param[out] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllLttEntryGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         index,
    OUT PRV_TGF_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC      dxChLttEntry;

    rc = cpssDxChL2MllLttEntryGet(devNum, index, &dxChLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllLttEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert L2 MLL LTT entry into generic format */
    rc = prvTgfConvertDxChToGenericL2MllLttEntry(&dxChLttEntry, lttEntryPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericL2MllLttEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllPairWrite function
* @endinternal
*
* @brief   Write L2 MLL pair entry to hw.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - (pointer to) the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllPairWrite
(
    IN GT_U8                                      devNum,
    IN GT_U32                                     mllPairEntryIndex,
    IN PRV_TGF_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN PRV_TGF_L2_MLL_PAIR_STC                    *mllPairEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_L2_MLL_PAIR_STC          dxChMllPairEntry;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT dxchMllPairWriteForm;

    cpssOsMemSet(&dxChMllPairEntry, 0, sizeof(dxChMllPairEntry));

    switch(mllPairWriteForm)
    {
        case PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E:
             dxchMllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
             break;
        case PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
             dxchMllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;
             break;
        case PRV_TGF_PAIR_READ_WRITE_WHOLE_E:
             dxchMllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
             break;
        default:
            return GT_BAD_PARAM;
    }
    /* convert L2 MLL entry into device specific format */
    rc = prvTgfConvertGenericToDxChL2MllPairEntry(mllPairEntryPtr, &dxChMllPairEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChL2MllPairEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChL2MllPairWrite(devNum, mllPairEntryIndex,dxchMllPairWriteForm, &dxChMllPairEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllPairWrite FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllPairRead function
* @endinternal
*
* @brief   Write L2 MLL pair entry to hw.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - (pointer to) the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfL2MllPairRead
(
    IN  GT_U8                                      devNum,
    IN  GT_U32                                     mllPairEntryIndex,
    IN  PRV_TGF_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    OUT PRV_TGF_L2_MLL_PAIR_STC                    *mllPairEntryPtr

)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_L2_MLL_PAIR_STC          dxChMllPairEntry;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT dxchMllPairWriteForm;

    switch(mllPairWriteForm)
    {
        case PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E:
             dxchMllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
             break;
        case PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
             dxchMllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;
             break;
        case PRV_TGF_PAIR_READ_WRITE_WHOLE_E:
             dxchMllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
             break;
        default:
            return GT_BAD_PARAM;
    }

    cpssOsMemSet(&dxChMllPairEntry, 0, sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));

    rc = cpssDxChL2MllPairRead(devNum, mllPairEntryIndex,dxchMllPairWriteForm, &dxChMllPairEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChL2MllPairRead FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert L2 MLL entry into generic format */
    rc = prvTgfConvertDxChToGenericL2MllPairEntry(&dxChMllPairEntry, mllPairEntryPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericL2MllPairEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfL2MllNumberOfMllsGet function
* @endinternal
*
* @brief   The function gets the number of MLL pairs entries.
*/
GT_U32 prvTgfL2MllNumberOfMllsGet
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_U8                               devNum  = prvTgfDevNum;
    GT_U32          numOfEntries;

    numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.mllPairs;

    return numOfEntries;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return 0;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return 0;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfL2MllCheck function
* @endinternal
*
* @brief   check if the device supports L2 MLL.
*
* @param[in] devNum                   - the device number
*
* @retval GT_TRUE                  - the device supports L2 MLL
* @retval GT_FALSE                 - the device not supports L2 MLL
*
* @note Support for L2 MLL is deduced from the eArch enabling state of the
*       device.
*
*/
GT_BOOL prvTgfL2MllCheck
(
    IN  GT_U8   devNum
)
{
#ifdef CHX_FAMILY
    return (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum));
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfL2MllMultiTargetPortEnableSet function
* @endinternal
*
* @brief   Enable multi-target port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - multi-target port mapping status:
*                                      GT_TRUE: enable multi-target port mapping
*                                      GT_FALSE: disable multi-target port mapping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllMultiTargetPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllMultiTargetPortEnableSet(devNum, enable);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllMultiTargetPortEnableGet function
* @endinternal
*
* @brief   Get the multi-target port mapping enabling status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) multi-target port mapping status:
*                                      GT_TRUE: multi-target port mapping is enabled
*                                      GT_FALSE: multi-target port mapping is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllMultiTargetPortEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllMultiTargetPortEnableGet(devNum, enablePtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllMultiTargetPortSet function
* @endinternal
*
* @brief   Set the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - multi-target port value
*                                      (APPLICABLE RANGES: 0..0x1ffff)
* @param[in] mask                     - multi-target port mask
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range value or mask
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllMultiTargetPortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      value,
    IN  GT_U32                      mask
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllMultiTargetPortSet(devNum, value, mask);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(value);
    TGF_PARAM_NOT_USED(mask);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllMultiTargetPortGet function
* @endinternal
*
* @brief   Get the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) multi-target port value
* @param[out] maskPtr                  - (pointer to) multi-target port mask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllMultiTargetPortGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *valuePtr,
    OUT GT_U32                      *maskPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllMultiTargetPortGet(devNum, valuePtr, maskPtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(valuePtr);
    TGF_PARAM_NOT_USED(maskPtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllMultiTargetPortBaseSet function
* @endinternal
*
* @brief   Set the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portBase                 - multi-target port base
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range port base
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllMultiTargetPortBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portBase
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllMultiTargetPortBaseSet(devNum, portBase);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portBase);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllMultiTargetPortBaseGet function
* @endinternal
*
* @brief   Get the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portBasePtr              - (pointer to) multi-target port base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllMultiTargetPortBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *portBasePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllMultiTargetPortBaseGet(devNum, portBasePtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portBasePtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllPortToVidxBaseSet function
* @endinternal
*
* @brief   Set the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidxBase                 - port VIDX base
*                                      (APPLICABLE RANGES: 0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range vidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllPortToVidxBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      vidxBase
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllPortToVidxBaseSet(devNum, vidxBase);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vidxBase);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllPortToVidxBaseGet function
* @endinternal
*
* @brief   Get the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidxBasePtr              - (pointer to) port VIDX base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS prvTgfL2MllPortToVidxBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *vidxBasePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllPortToVidxBaseGet(devNum, vidxBasePtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vidxBasePtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfL2MllVidxEnableSet function
* @endinternal
*
* @brief   Enable/disable MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
* @param[in] enable                   - GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfL2MllVidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_U16  vidx,
    IN GT_BOOL enable

)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllVidxEnableSet( devNum, vidx, enable );
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vidx);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfL2MllVidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfL2MllVidxEnableGet
(
    IN GT_U8     devNum,
    IN GT_U16    vidx,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChL2MllVidxEnableGet( devNum, vidx, enablePtr );
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vidx);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

