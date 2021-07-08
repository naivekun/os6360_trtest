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
* @file cpssDxChPortInterlaken.h
*
* @brief CPSS DxCh Interlaken Port API.
* @version   4
********************************************************************************
*/

#ifndef __cpssDxChPortInterlakenh
#define __cpssDxChPortInterlakenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/common/port/cpssPortStat.h>

/**
* @internal cpssDxChPortInterlakenCounterGet function
* @endinternal
*
* @brief   Function gets counters of Port Interlaken interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] counterId                - counter Id
* @param[in] laneNum                  - lane number - relevant only for
*                                      BLK_TYPE_ERR, DIAG_CRC_ERR, WORD_SYNC_ERR
*                                      (APPLICABLE RANGES: 0..7)
*
* @param[out] counterPtr               - (pointer to) 64-bit counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on illegal state
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortInterlakenCounterGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERLAKEN_COUNTER_ID_ENT     counterId,
    IN  GT_U32                                  laneNum,
    OUT GT_U64                                  *counterPtr
);


GT_STATUS prvCpssDxChPortIlknIFBWSet
(
    IN GT_U8    devNum,
    IN GT_U32   ilknIfBWGBps,
    OUT GT_U32 *maxIlknIFSpeedMbpsPtr
);



/**
* @internal cpssDxChPortIlknChannelSpeedResolutionSet function
* @endinternal
*
* @brief   Configure Interlaken channel speed resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] channelSpeedResMbps      - channel speed resolution in Mbps
*
* @param[out] maxIlknIFSpeedMbpsPtr    - The total bandwidth the Interlaken channels will support with the given resolution (small resolution == small total bandwidth)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - speed out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - default resultion is 40Mbps
*       - if resolution is greater than IF bandwidth, GT_BAD_PARAM is returned
*       - returns GT_BAD_PARAM if requested resolution is not compatible with any currently existing channels.
*       In that case user shall delete all channels that are incompatible with new resolution and
*       repeat the cpssDxChPortInterlakenChannelSpeedResultionSet();
*
*/
GT_STATUS cpssDxChPortIlknChannelSpeedResolutionSet
(
    IN GT_U8    devNum,
    IN GT_U32   channelSpeedResMbps,
    OUT GT_U32 *maxIlknIFSpeedMbpsPtr
);

/**
* @internal cpssDxChPortIlknChannelSpeedResolutionGet function
* @endinternal
*
* @brief   Get Interlaken channel speed resolution and ilkn IF BW that will be supported.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] speedResulutionMBpsPtr   - channel speed resolution in Mbps
*                                      maxIlknIFSpeedMbpsPtr  - The total bandwidth the Interlaken channels will support with the given resolution (small resolution == small total bandwidth)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - speed out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortIlknChannelSpeedResolutionGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *speedResulutionMBpsPtr,
    OUT GT_U32  *maxIlknIFSpeedPtr
);


/**
* @internal cpssDxChPortIlknChannelSpeedSet function
* @endinternal
*
* @brief   Configure Interlaken channel speed.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number of channel.
* @param[in] direction                - TX/RX/Both.
* @param[in] speed                    - channel  in Mbps, resolution depends on ASIC. Use 0 to
*                                      free channel resources.
*
* @param[out] actualSpeedPtr           - because HW constrains dectate granularity, real TX speed
*                                      could differ from requested. Application must use this
*                                      value to know how much of general ILKN interface bandwidth
*                                      remain free for TX.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_INITIALIZED       - ILKN interface not configured properly
* @retval GT_OUT_OF_RANGE          - speed out of range
* @retval GT_NO_RESOURCE           - no bandwidth to supply channel speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - Could be engaged only after cpssDxChPortModeSpeedSet pass.
*       - the speed shall be defined in MBps in quantum of channel speed resolution, provided by
*       cpssDxChPortIlknChannelSpeedResolutionSet()
*       - RX speed reconfiguration can cause fragmentation in Packet Reassembly memory
*       and GT_NO_RESOURCE error when actually there is enough free space, to solve
*       this application will have delete all or part of channels and configure them
*       again in one sequence.
*
*/
GT_STATUS cpssDxChPortIlknChannelSpeedSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  speed,
    OUT GT_U32                  *actualSpeedPtr
);

/**
* @internal cpssDxChPortIlknChannelSpeedGet function
* @endinternal
*
* @brief   Get Interlaken channel speed.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number of channel.
* @param[in] direction                - TX/RX. Attention: Both not supported!
*
* @param[out] speedPtr                 - actual channel speed in Mbps.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - speedPtr == NULL
* @retval GT_NOT_INITIALIZED       - ILKN interface not configured properly
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Bobcat2 ILKN_TXFIFO allows 40Mbps granularity of TX channel speed if
*       bandwidth of ILKN channel is 40Gbps and 20Mbps if total BW is 20Gbps.
*
*/
GT_STATUS cpssDxChPortIlknChannelSpeedGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_U32                  *speedPtr
);

/**
* @internal cpssDxChPortIlknChannelEnableSet function
* @endinternal
*
* @brief   Enable/disable Interlaken channel.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number of channel.
* @param[in] direction                - TX/RX/Both.
* @param[in] enable                   -   GT_TRUE - "direction" of channel enabled;
*                                      GT_FALSE- "direction" channel disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NO_RESOURCE           - no bandwidth to supply channel speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortIlknChannelEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChPortIlknChannelEnableGet function
* @endinternal
*
* @brief   Get Enable/disable state of Interlaken channel.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number of channel.
* @param[in] direction                - TX/RX. Attention: Both not supported.
*
* @param[out] enablePtr                -   GT_TRUE - "direction" of channel enabled;
*                                      GT_FALSE- "direction" channel disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NO_RESOURCE           - no bandwidth to supply channel speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortIlknChannelEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL                 *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortInterlakenh */

