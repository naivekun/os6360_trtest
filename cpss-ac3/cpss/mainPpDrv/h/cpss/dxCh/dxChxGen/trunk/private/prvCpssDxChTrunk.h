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
* @file prvCpssDxChTrunk.h
*
* @brief API definitions for 802.3ad Link Aggregation (Trunk) facility
* private - CPSS - DxCh
*
* @version   14
********************************************************************************
*/

#ifndef __prvCpssDxChTrunkh
#define __prvCpssDxChTrunkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>

/**
* @internal prvCpssDxChTrunkFamilyInit function
* @endinternal
*
* @brief   This function sets CPSS private DxCh family info about trunk behavior
*         that relate to the family and not to specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devFamily                - device family.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - the family was not initialized properly
*/
GT_STATUS prvCpssDxChTrunkFamilyInit(
    IN CPSS_PP_FAMILY_TYPE_ENT  devFamily
);


/**
* @internal prvCpssDxChTrunkTrafficToCpuWaUpInit function
* @endinternal
*
* @brief   initialize the UP (user priority) to port settings,needed WA for :
*         "wrong trunk id source port information of packet to CPU"
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that we set member on.
* @param[in] additionalInfoBmp        - bitmap for additional WA info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaUpInit
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  additionalInfoBmp
);

/**
* @internal prvCpssDxChTrunkTrafficToCpuWaUpToMemberConvert function
* @endinternal
*
* @brief   convert the UP (user priority) to trunk member (dev,port)
*         Needed for WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that connected to CPU port/SDMA.
* @param[in,out] dsaTagInfoPtr            - (pointer to) the DSA tag info parsed from the DSA tag.
* @param[in,out] dsaTagInfoPtr            - (pointer to) the DSA tag info with fixed info regarding
*                                      "wrong trunk id source port information of packet to CPU"
*
* @retval GT_OK                    - on success , the DSA tag was 'fixed' (updated)
* @retval GT_NO_CHANGE             - there is no need for 'convert'
* @retval GT_DSA_PARSING_ERROR     - No valid mapping between the UP and trunk member.
*/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaUpToMemberConvert
(
    IN  GT_U8   devNum,
    INOUT CPSS_DXCH_NET_DSA_PARAMS_STC  *dsaTagInfoPtr
);

/**
* @internal prvCpssDxChTrunkTrafficToCpuWaCpuPortEnableSet function
* @endinternal
*
* @brief   enable/disable the needed for the WA for the CPU/cascade port that may
*         represent the pass to the CPU
*         WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that we set member on.
* @param[in] portNum                  - the CPU/cascade port that may represent the pass to the CPU
* @param[in] enable                   - enable/disable the settings
*
* @retval GT_OK                    - on success , or device not need the WA
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaCpuPortEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal prvCpssDxChTrunkHwDevNumSet function
* @endinternal
*
* @brief   Update trunk logic prior change of HW device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] hwDevNum                 - HW device number
*                                      NOTE: at this stage the PRV_CPSS_HW_DEV_NUM_MAC(devNum) hold
*                                      the 'old hwDevNum' !!!
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChTrunkHwDevNumSet
(
    IN GT_U8            devNum,
    IN GT_HW_DEV_NUM    hwDevNum
);

/**
* @internal prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet function
* @endinternal
*
* @brief   the function sets the designated device table with the portsArr[].trunkPort
*         in indexes that match hash (%8 or %64(modulo)) on the ports in portsArr[].srcPort
*         this to allow 'Src port' trunk hash for traffic sent to the specified
*         trunk.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] numOfPorts               - number of pairs in array portsArr[].
* @param[in] portsArr[]               - (array of) pairs of 'source ports' ,'trunk ports'
*                                      for the source port hash.
* @param[in] mode                     - hash  (%8 or %64 (modulo))
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or trunkId or port in portsArr[].srcPort or
*                                       port in portsArr[].trunkPort or mode
*                                       or map two Source ports that falls into same Source hash index
*                                       into different trunk member ports
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*/
GT_STATUS prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet
(
    IN GT_U8             devNum,
    IN GT_U32            numOfPorts,
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC  portsArr[],
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT  mode
);

/**
* @internal prvCpssDxChTrunkHashBitsSelectionGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
*
* @param[out] startBitPtr              - (pointer to) start bit
* @param[out] numOfBitsPtr             - (pointer to) number of bits
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTrunkHashBitsSelectionGet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    OUT GT_U32           *startBitPtr,
    OUT GT_U32           *numOfBitsPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssDxChTrunkh */


