/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChLpmUtils.c
*
* DESCRIPTION:
*       private LPM utility functions
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/
#ifndef __prvCpssDxChLpmUtilsh
#define __prvCpssDxChLpmUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>

/**
* @internal prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap function
* @endinternal
*
* @brief   Convert IP protocol stack value to bitmap of protocols
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] protocolStack            - the IP protocol stack
*
* @param[out] protocolBitmapPtr        - the protocol bitmap
*                                       None
*
* @note The function assumes that a validity check was done on protocolStack
*       before calling to this function.
*
*/
GT_VOID prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT        protocolStack,
    OUT PRV_CPSS_DXCH_LPM_PROTOCOL_BMP    *protocolBitmapPtr
);

/**
* @internal prvCpssDxChLpmConvertProtocolBitmapToIpProtocolStack function
* @endinternal
*
* @brief   Convert bitmap of protocols to IP protocol stack value
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] protocolBitmap           - the protocol bitmap
*
* @param[out] protocolStackPtr         - the IP protocol stack
*                                       None
*
* @note The function assumes that protocolBitmap holds at least one IP protocol
*
*/
GT_VOID prvCpssDxChLpmConvertProtocolBitmapToIpProtocolStack
(
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_BMP    protocolBitmap,
    OUT CPSS_IP_PROTOCOL_STACK_ENT        *protocolStackPtr
);

/**
* @internal prvCpssDxChLpmConvertIpShadowTypeToLpmShadowType function
* @endinternal
*
* @brief   Convert enum of IP shadow type to LPM shadow type
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] ipShadowType             - the IP shadow type
*
* @param[out] lpmShadowTypePtr         - the LPM shadow type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChLpmConvertIpShadowTypeToLpmShadowType
(
    IN  CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT    ipShadowType,
    OUT PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT   *lpmShadowTypePtr
);

/**
* @internal prvCpssDxChLpmConvertLpmShadowTypeToIpShadowType function
* @endinternal
*
* @brief   Convert enum of LPM shadow type to IP shadow type
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] lpmShadowType            - the LPM shadow type
*
* @param[out] ipShadowTypePtr          - the IP shadow type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChLpmConvertLpmShadowTypeToIpShadowType
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT    lpmShadowType,
    OUT CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT     *ipShadowTypePtr
);

/**
* @internal prvCpssDxChLpmConvertIpLttEntryToLpmRouteEntry function
* @endinternal
*
* @brief   Convert IP LTT entry structure to private LPM route entry structure
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] lpmHwType                - whether the device uses RAM for LPM
* @param[in] ipLttEntryPtr            - the IP LTT entry
*
* @param[out] routeEntryPtr            - the LPM route entry
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertIpLttEntryToLpmRouteEntry
(
    IN  PRV_CPSS_DXCH_LPM_HW_ENT                        lpmHwType,
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC                      *ipLttEntryPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *routeEntryPtr
);

/**
* @internal prvCpssDxChLpmConvertLpmRouteEntryToIpLttEntry function
* @endinternal
*
* @brief   Convert private LPM route entry structure to IP LTT entry structure
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] lpmHwType                - whether the device uses RAM for LPM
* @param[in] routeEntryPtr            - the LPM route entry
*
* @param[out] ipLttEntryPtr            - the IP LTT entry
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertLpmRouteEntryToIpLttEntry
(
    IN  PRV_CPSS_DXCH_LPM_HW_ENT                        lpmHwType,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *routeEntryPtr,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC                      *ipLttEntryPtr
);


/*******************************************************************************
* prvCpssDxChLpmConvertIpVrConfigToTcamVrConfig
*
* @brief   Convert IP VR config structure to private TCAM VR config structure
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] shadowType               - the shadow type
* @param[in] ipVrConfigPtr            - the IP VR config structure
*
* @param[out] tcamVrConfigPtr          - the TCAM VR config structure
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertIpVrConfigToTcamVrConfig
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT        shadowType,
    IN  CPSS_DXCH_IP_LPM_VR_CONFIG_STC           *ipVrConfigPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_VR_CONFIG_STC     *tcamVrConfigPtr
);

/**
* @internal prvCpssDxChLpmConvertTcamVrConfigToIpVrConfig function
* @endinternal
*
* @brief   Convert private TCAM VR config structure to IP VR config structure
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] shadowType               - the shadow type
* @param[in] tcamVrConfigPtr          - the TCAM VR config structure
*
* @param[out] ipVrConfigPtr            - the IP VR config structure
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertTcamVrConfigToIpVrConfig
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT        shadowType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_VR_CONFIG_STC     *tcamVrConfigPtr,
    OUT CPSS_DXCH_IP_LPM_VR_CONFIG_STC           *ipVrConfigPtr
);

/**
* @internal prvCpssDxChLpmConvertIpVrConfigToRamVrConfig function
* @endinternal
*
* @brief   Convert IP VR config structure to private RAM VR config structure
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] ipVrConfigPtr            - the IP VR config structure
*
* @param[out] ramVrConfigPtr           - the RAM VR config structure
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertIpVrConfigToRamVrConfig
(
    IN  CPSS_DXCH_IP_LPM_VR_CONFIG_STC           *ipVrConfigPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC      *ramVrConfigPtr
);

/**
* @internal prvCpssDxChLpmConvertRamVrConfigToIpVrConfig function
* @endinternal
*
* @brief   Convert private RAM VR config structure to IP VR config structure
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] ramVrConfigPtr           - the RAM VR config structure
*
* @param[out] ipVrConfigPtr            - the IP VR config structure
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertRamVrConfigToIpVrConfig
(
    IN  PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC      *ramVrConfigPtr,
    OUT CPSS_DXCH_IP_LPM_VR_CONFIG_STC           *ipVrConfigPtr
);

/**
* @internal prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry function
* @endinternal
*
* @brief   Convert IP route entry union to private LPM route entry union
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] shadowType               - shadow type
* @param[in] ipRouteEntryInfoPtr      - IP route entry union
*
* @param[out] lpmRouteEntryInfoPtr     - private LPM route entry union
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT        shadowType,
    IN  CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *ipRouteEntryInfoPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT   *lpmRouteEntryInfoPtr
);

/**
* @internal prvCpssDxChLpmConvertLpmRouteEntryToIpRouteEntry function
* @endinternal
*
* @brief   Convert private LPM route entry union to IP route entry union
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] shadowType               - shadow type
* @param[in] lpmRouteEntryInfoPtr     - private LPM route entry union
*
* @param[out] ipRouteEntryInfoPtr      - IP route entry union
*                                       None
*/
GT_VOID prvCpssDxChLpmConvertLpmRouteEntryToIpRouteEntry
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT        shadowType,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT   *lpmRouteEntryInfoPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *ipRouteEntryInfoPtr
);

/**
* @internal prvCpssDxChLpmGetHwType function
* @endinternal
*
* @brief   Convert LPM shadow type to HW type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] lpmShadowType            - shadow type
*                                       the HW type used for LPM (TCAM or RAM)
*/
PRV_CPSS_DXCH_LPM_HW_ENT prvCpssDxChLpmGetHwType
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT lpmShadowType
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmUtilsh */

