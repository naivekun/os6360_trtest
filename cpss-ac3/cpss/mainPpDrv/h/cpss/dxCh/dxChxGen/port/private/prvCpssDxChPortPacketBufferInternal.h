/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPortPacketBufferInternal.h
*
* DESCRIPTION:
*       private CPSS DxCh Packet Buffer functions.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChPortPacketBufferInternal_h
#define __prvCpssDxChPortPacketBufferInternal_h

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortPacketBuffer.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT
*
* @brief This enum defines Packet buffer Channel speed values.
*/
typedef enum
{
    /** channel disabled  */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E,

    /** 25  Gigabit/sec */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E,

    /** 50  Gigabit/sec */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E,

    /** 100 Gigabit/sec */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E,

    /** 200 Gigabit/sec */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E,

    /** 400 Gigabit/sec */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E

} PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT;

/**
* @enum PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_ENT
*
* @brief This enum defines Packet buffer HW values types.
*/
typedef enum
{
    /** number of GPC read channels */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_NUMOF_E,

    /** GPC read channel for CPU */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_CPU_E,

    /** GPC read channel for TXQ */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_TXQ_E,

    /** Bitmap of virtual SERDES used by channel */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_VSD_BMP_E,

    /**"not used" value in FIFO Start registers - SW stamp in unused HW  */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_START_NOT_USED_E,

    /** number of GPC read FIFO Segments */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_ALL_NUMOF_E,

    /** number of Arbiter Point Round Robins */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RRS_NUMOF_E,

    /** number of Arbiter Point in given Round Robin */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_NUMOF_E,

    /** number of Round Robin for CPU channel */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_CPU_E,

    /** number of Arbiter Point in Round Robin for CPU channel */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_CPU_E,
    /** number of Round Robin for TXQ channel */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_TXQ_E,

    /** number of Arbiter Point in Round Robin for TXQ channel */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_TXQ_E,

    /** bitmap of Arbiter Points in given Round Robin for given channel and speed */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_BMP_E,

    /** HW value for disconnected arbiter point */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_AP_DISCONNECTED_E,

    /** HW value for arbiter calendar slot with higest priority for given RR */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_RR_VALUE_E,

    /** RR index with max priority for given virtual serdes of given channel */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_RR_INDEX_E,

    /** maximal amount of used slots of Arbiter Calendar */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_MAX_SLOTS_E,

    /** virtual SERDES index for given slot of Arbiter Caledar */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_VSD_BY_SLOT_E,

    /** HW value for channel shaper rerister (calculated by speed) */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_SHAPER_E

} PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_ENT;

/* error retutn code */
#define PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS 0xFFFFFFFF
/* maximal amount of channels used for array sizezs */
#define PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_MAX_CHANNELS_CNS 10
/* speed for each FIFO segment  */
#define PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_FIFO_SEG_SPEED_CNS 25

/* Feature internal functions */

/**
* @struct PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC
*
* @brief Structure for configuration of GPC Read Channel
*/
typedef struct
{
    /** channel index */
    GT_U32                                             channel;

    /** Virtual SERDES bitmap */
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT channelSpeed;

    /** channel speed enum element */
    GT_U32                                             vsdBitmap;

} PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC;


/**
* @internal prvCpssDxChPortPacketBufferInternalChannelSpeedTo25GUnits function
* @endinternal
*
* @brief   Converts channel speed values to speed in 25G units.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] channelSpeed   - channel speed enum value
*
* @param[out] speedUnitsPtr - (pointer to)channel speed in 25G units
*
* @retval GT_OK             - on success
* @retval GT_BAD_PARAM      - on wrong parameters
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalChannelSpeedTo25GUnits
(
    IN   PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    channelSpeed,
    OUT  GT_U32                                                *speedUnitsPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInternalChannel25GUnitsToSpeed function
* @endinternal
*
* @brief   Converts speed in 25G units to channel speed enum values.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speedUnits      - speed in 25G units
*
* @param[out]channelSpeedPtr - (pointer to)of channel speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalChannel25GUnitsToSpeed
(
    IN   GT_U32                                              speedUnits,
    OUT  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT  *channelSpeedPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInternalPortToChannelSpeedConvert function
* @endinternal
*
* @brief   Converts port speed values to channel speed values.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] portSpeed        - port speed in enum values
*
* @param[out] channelSpeedPtr - (pointer to)of channel speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NULL_PTR              - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Device number parameter currently not used.
*    It definded for resolving problems of port speeds supported not by all devices..
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalPortToChannelSpeedConvert
(
    IN  GT_U8                                                 devNum,
    IN  CPSS_PORT_SPEED_ENT                                   portSpeed,
    OUT PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    *channelSpeedPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInternalChannelToPortSpeedConvert function
* @endinternal
*
* @brief   Converts port speed values to channel speed values.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum          - device number
* @param[in]  channel Speed   - channel speed in enum values
*
* @param[out] portSpeedPtr    - (pointer to)of port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NULL_PTR              - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Device number parameter currently not used.
*    It definded for resolving problems of port speeds supported not by all devices..
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalChannelToPortSpeedConvert
(
    IN  GT_U8                                                 devNum,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    channelSpeed,
    OUT CPSS_PORT_SPEED_ENT                                   *portSpeedPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInternalPortToChannelConvert function
* @endinternal
*
* @brief   Converts port number to tripple <tileIndex, gpcIndex, channel>.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] portNum          - port number
*
* @param[out] tileIndexPtr    - (pointer to) tile index
* @param[out] gpcIndexPtr     - (pointer to) GPC index
* @param[out] channelPtr      - (pointer to) Read GPC Channel index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalPortToChannelConvert
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *tileIndexPtr,
    OUT GT_U32                          *gpcIndexPtr,
    OUT GT_U32                          *channelPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInternalHwValueGet function
* @endinternal
*
* @brief Get different HW values and HW array indexes.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum          - device number
* @param[in]  hwValType       - type of required value
* @param[in]  param0          - additional parameter depending of hwValType
* @param[in]  param1          - additional parameter depending of hwValType
* @param[in]  param2          - additional parameter depending of hwValType
*
* @retval required value or 0xFFFFFFFF on wrong parameters
*
*/
GT_U32 prvCpssDxChPortPacketBufferInternalHwValueGet
(
    IN  GT_U8                                         devNum,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_ENT hwValType,
    IN  GT_U32                                        param0,
    IN  GT_U32                                        param1,
    IN  GT_U32                                        param2
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapSet function
* @endinternal
*
* @brief Configure GPC packet read channel Fifo Segments.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
* @param[in] fifoSegBmp     - bitmap of Fifo segments.
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel,
    IN  GT_U32                                                fifoSegBmp
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapGet function
* @endinternal
*
* @brief Get GPC packet read channel Fifo Segment bitmap.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum         - device number
* @param[in]  tileIndex      - index of tile
*                              (APPLICABLE RANGES: 0..3)
* @param[in]  gpcIndex       - index of GPC (Group of Port Channels)
*                              (APPLICABLE RANGES: 0..7)
* @param[in]  channel        - index of channel
*
* @param[out] fifoSegBmpPtr  - (pointer to)bitmap of Fifo segments.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapGet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel,
    OUT GT_U32                                                *fifoSegBmpPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentsDisable function
* @endinternal
*
* @brief Disable GPC packet read channel Fifo Segments.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentsDisable
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsSet function
* @endinternal
*
* @brief Configure GPC packet read channels arbiter points for given channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
* @param[in] speed          - channel speed enum value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    speed
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsDisable function
* @endinternal
*
* @brief Disable GPC packet read channels arbiter points for given channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsDisable
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadCfgGet function
* @endinternal
*
* @brief Get configurations of all channels from HW.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum           - device number
* @param[in]  tileIndex        - index of tile
*                                (APPLICABLE RANGES: 0..3)
* @param[in]  gpcIndex         - index of GPC (Group of Port Channels)
*                                (APPLICABLE RANGES: 0..7)
* @param[out] channelsNumOfPtr - (pointer to) number of configured channels
* @param[out] channelsCfgArr   - (pointer to) array of channels configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadCfgGet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    OUT GT_U32                                                *channelsNumOfPtr,
    OUT PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC channelsCfgArr[]
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterCalendarSet function
* @endinternal
*
* @brief Set all Arbiter Calendar by given configuration of all ports.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]   devNum         - device number
* @param[in]   tileIndex      - index of tile
*                               (APPLICABLE RANGES: 0..3)
* @param[in]   gpcIndex       - index of GPC (Group of Port Channels)
*                               (APPLICABLE RANGES: 0..7)
* @param[in]   channelsNum    - number of configured channels
* @param[in]   channelsCfgArr - (pointer to) array of channels configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterCalendarSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channelsNum,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC channelsCfgArr[]
);

/**
* @internal prvCpssDxChPortPacketBufferInternalTxCpuTxqReset function
* @endinternal
*
* @brief Reset TX_DMA channels for CPU and TXQ.
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
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalTxDmaCpuTxqReset
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadInit function
* @endinternal
*
* @brief Init speed of GPC packet read channels.
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
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadInit
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet function
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
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channelIndex   - channel index.
*                             channels 0..7 related to network ports - configured only at runtime
*                             channel 8 related to CPU port - initialized to 25G speed,
*                             can be changed at runtime,
*                             channel 9 related to TXQ - initialized to 50G speed,
*                             cannot be changed at runtime,
*                             (APPLICABLE RANGES: 0..9)
* @param[in] speed          - speed of the channel enum value
*                             The speeds of channels 0..7 speeds should be also valid for connected MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channelIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    speed
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcChannelSpeedGet function
* @endinternal
*
* @brief Set speed of GPC read channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channelIndex   - channel index.
*                             channels 0..7 related to network ports
*                             channel 8 related to CPU port
*                             channel 9 related to TXQ
*                             (APPLICABLE RANGES: 0..9)
*
* @param[out] speedPtr       - (pointer to)speed of the channel enum value
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
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcChannelSpeedGet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channelIndex,
    OUT PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    *speedPtr
);

/**
* @internal prvCpssDxChPortPacketBufferInternalGenericInit function
* @endinternal
*
* @brief Generic init of all tile subunits.
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
GT_STATUS prvCpssDxChPortPacketBufferInternalGenericInit
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex
);

/**
* @internal prvCpssDxChPortPacketBufferInternalTileModeSet function
* @endinternal
*
* @brief Set tile mode and id. All relevant subunits configured.
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
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Typically tiles configured according to given mode.
*      Configured tiles by pairs of tileIndex and tileId below:
*       1 tile:  <0, 0>
*       2 tiles: <0, 0>, <1, 1>
*       4 tiles: <0, 0>, <1, 1>, <2, 2>, <3, 3>
*       This function should allow to configure only part of tiles
*       for debugging purposes(using only part of ports).
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalTileModeSet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         tileIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT tilemode,
    IN  GT_U32                                         tileId
);

/**
* @internal prvCpssDxChPortPacketBufferInternalRxDmaPbFillLevelThresholdSet function
* @endinternal
*
* @brief Set PB Fill Level Thresholds.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] tilemode       - enum value for 1,2 or 4-tile systems
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of range parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalRxDmaPbFillLevelThresholdSet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              tileIndex,
    IN  GT_U32                                              gpcIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT      tilemode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPortPacketBufferInternal_h */
