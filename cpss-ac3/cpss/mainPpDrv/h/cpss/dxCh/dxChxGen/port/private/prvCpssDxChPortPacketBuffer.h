/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPortPacketBuffer.h
*
* DESCRIPTION:
*       private CPSS DxCh Packet Buffer functions.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChPortPacketBuffer_h
#define __prvCpssDxChPortPacketBuffer_h

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT
*
* @brief This enum defines Packet buffer tile mode.
*/
typedef enum
{
    /** 1 tile system */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_1_TILE_E,

    /** 2 tiles system */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_2_TILE_E,

    /**  4 tiles system*/
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_4_TILE_E

} PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT;

/* amount of front ports in each GPC including CPU and TXQ Descripters FIFO */
#define PRV_CPSS_DXCH_PORT_PACKET_BUFFER_FRONT_PORTS_PER_GPC_CNS 10

/* Feature external functions */

/**
* @internal prvCpssDxChPortPacketBufferTileModeGet function
* @endinternal
*
* @brief Get tile mode and id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
*
* @param[out] tilemodePtr   - (pointer to)enum value for 1,2 or 4-tile systems
* @param[out] tileIdPtr     - (pointer to)tile Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on wrong register content
* @retval GT_BAD_PTR               - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferTileModeGet
(
    IN   GT_U8                                          devNum,
    IN   GT_U32                                         tileIndex,
    OUT  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT *tilemodePtr,
    OUT  GT_U32                                         *tileIdPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInit function
* @endinternal
*
* @brief Initialize all subunits of tile not depended of port speeds.
*    Packet Buffer unit yet not ready for traffic.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] tilemode       - enum value for 1,2 or 4-tile systems
* @param[in] tileId         - tile Id
*                             (APPLICABLE RANGES: 0..3)
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note
*    Typically tiles configured according to given mode.
*    Configured tiles by pairs of tileIndex and tileId below:
*    1 tile:  <0, 0>
*    2 tiles: <0, 0>, <1, 1>
*    4 tiles: <0, 0>, <1, 1>, <2, 2>, <3, 3>
*    This function should allow to configure only part of tiles
*    for debugging purposes(using only part of ports).
*/
GT_STATUS prvCpssDxChPortPacketBufferInit
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         tileIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT tilemode,
    IN  GT_U32                                         tileId
);

/**
* @internal prvCpssDxChPortPacketBufferAllTilesInit function
* @endinternal
*
* @brief Initialize all subunits of all tiles not depended of port speeds.
*    Packet Buffer unit yet not ready for traffic.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferAllTilesInit
(
    IN  GT_U8     devNum
);

/**
* @internal prvCpssDxChPortPacketBufferGpcChannelSpeedSet function
* @endinternal
*
* @brief Set speed of GPC read channel. All relevant subunits configured.
*    Assumed that the port that their speed being changed stopped,
*    but other ports connected to the same GPC
*    continue to receive/send packets and should not be affected.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] portNum        - port number
* @param[in] speed          - port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcChannelSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssDxChPortPacketBufferGpcChannelSpeedGet function
* @endinternal
*
* @brief Set speed of GPC read channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum         - device number
* @param[in]  portNum        - port number
*
* @param[out] speedPtr       - (pointer to)port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NULL_PTR              - on NULL pointer parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcChannelSpeedGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_SPEED_ENT             *speedPtr
);

/**
* @internal prvCpssDxChPortPacketBufferGpcPacketReadRegsDump function
* @endinternal
*
* @brief Dump registers of GPC read subunit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcPacketReadRegsDump
(
    IN  GT_U8      devNum,
    IN  GT_U32     tileIndex,
    IN  GT_U32     gpcIndex
);

/**
* @internal prvCpssDxChPortPacketBufferGpcSpeedsPrint function
* @endinternal
*
* @brief Print speed configurations of GPC read subunit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcSpeedsPrint
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
);

/**
* @internal prvCpssDxChPortPacketBufferGlobalDump function
* @endinternal
*
* @brief Dump global configurations of PB unit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGlobalDump
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex
);

/**
* @internal prvCpssDxChPortPacketBufferRegsPerGpcDump function
* @endinternal
*
* @brief Dump per GPC configurations of all per GPC subunits.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferRegsPerGpcDump
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileIndex,
    IN  GT_U32      gpcIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPortPacketBuffer_h */
