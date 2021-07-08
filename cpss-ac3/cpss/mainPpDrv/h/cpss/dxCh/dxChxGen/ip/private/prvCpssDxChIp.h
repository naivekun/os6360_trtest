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
* @file prvCpssDxChIp.h
*
* @brief Private CPSS DXCH Ip HW structures
*
* @version   13
********************************************************************************
*/
#ifndef __prvCpssDxChIph
#define __prvCpssDxChIph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>

/* Maximal index of analyzer that can be used in the nexthop entry */
#define PRV_CPSS_DXCH_IP_MIRROR_ANALYZER_MAX_INDEX_CNS  6

/* Maximal index of multicast shared tree that can be used in nexthop entry */
#define PRV_CPSS_DXCH_IP_SHARED_TREE_MAX_INDEX_CNS  63

/**
* @internal prvCpssDxChIpConvertUcEntry2HwFormat function
* @endinternal
*
* @brief   This function converts a given ip uc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] ipUcRouteEntryPtr        - The entry to be converted.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertUcEntry2HwFormat
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *ipUcRouteEntryPtr,
    OUT GT_U32                          *hwDataPtr
 );

/**
* @internal prvCpssDxChIpConvertHwFormat2UcEntry function
* @endinternal
*
* @brief   This function converts a given ip uc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] hwDataPtr                - The entry in the HW format representation.
* @param[in,out] ipUcRouteEntryPtr        - Set the route entry type to determine how hw data will be
*                                      interpreted
* @param[in,out] ipUcRouteEntryPtr        - The entry to be converted.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertHwFormat2UcEntry
(
    IN  GT_U8                               devNum,
    IN  GT_U32 *hwDataPtr,
    INOUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *ipUcRouteEntryPtr
);

/**
* @internal prvCpssDxChIpUcRouteEntriesCheck function
* @endinternal
*
* @brief   Check validity of the route entry parametrers, in all entries
*         of routeEntriesArray.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array.
*
* @retval GT_OK                    - on all valid parameters.
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDxChIpUcRouteEntriesCheck
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                          numOfRouteEntries
);

/**
* @internal prvCpssDxChIpConvertMcEntry2HwFormat function
* @endinternal
*
* @brief   This function converts a given ip mc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] ipMcRouteEntryPtr        - The entry to be converted.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertMcEntry2HwFormat
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *ipMcRouteEntryPtr,
    OUT GT_U32                          *hwDataPtr
);

/**
* @internal prvCpssDxChIpMcRouteEntryCheck function
* @endinternal
*
* @brief   Check validity of the route entry parametrers, in all entries
*         of routeEntriesArray.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntryPtr            - the Mc route entry
*
* @retval GT_OK                    - on all valid parameters.
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
*/
GT_STATUS prvCpssDxChIpMcRouteEntryCheck
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
);


/**
* @internal prvCpssDxChIpConvertHwFormat2McEntry function
* @endinternal
*
* @brief   This function converts a given ip mc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] hwDataPtr                - The mc entry in the HW format to be converted..
*
* @param[out] ipMcRouteEntryPtr        - The mc entry data.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertHwFormat2McEntry
(
    IN   GT_U8                           devNum,
    OUT  CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *ipMcRouteEntryPtr,
    IN   GT_U32                          *hwDataPtr
);

/**
* @internal prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet function
* @endinternal
*
* @brief   Function to get the number of ECMP indirect table 'logical' entries
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2, Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   -  the device number
*
* @retval The number of ECMP indirect table 'logical' entries.
*
* @note none.
*
*/
GT_U32  prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(
    IN  GT_U8                       devNum
);

/**
* @internal prvCpssDxChTableIpvxRouterEcmpNumEntriesGet function
* @endinternal
*
* @brief   Function to get the number of ECMP table 'logical' entries
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2, Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   -  the device number
*
* @retval The number of ECMP table 'logical' entries.
*
* @note none.
*
*/
GT_U32  prvCpssDxChTableIpvxRouterEcmpNumEntriesGet(
    IN  GT_U8                       devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChIph */


