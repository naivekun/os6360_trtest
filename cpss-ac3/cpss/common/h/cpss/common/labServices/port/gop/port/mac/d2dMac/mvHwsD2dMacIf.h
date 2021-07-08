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
* @file mvHwsD2dMacIf.h
*
* @brief D2D MAC interface API
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsD2dMacIf_H
#define __mvHwsD2dMacIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

#define HWS_D2D_MAC_CHANNEL_STEP                    4
#define HWS_D2D_MAC_CHANNEL_ALL                     0xFFFFFFFF

#define HWS_D2D_MAC_BASE_ADDR                       0x33000000
#define HWS_D2D_OFFSET                              0x01000000
#define PRV_HWS_D2D_MAC_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (regOffset + HWS_D2D_MAC_BASE_ADDR + d2dIndex * HWS_D2D_OFFSET)

#define HWS_D2D_MAC_DBG_BASE_ADDR                   0x33000000
#define PRV_HWS_D2D_DBG_MAC_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (regOffset + HWS_D2D_MAC_DBG_BASE_ADDR + d2dIndex * HWS_D2D_OFFSET)

#define HWS_D2D_MAC_DBG_FRM_TBL_HEADER_LEN          7

/**
* @struct MV_HWS_D2D_FRAME_GEN_CONFIG_STC
 *
 * @brief
*/
typedef struct
{
    /** @brief specifies number of times the entire frame is sent
     */
    GT_U32 txFrameRepeatCount;

    /** @brief number of frame to sent sent starting from index <frameTableStartPointer>
     */
    GT_U32 txFrameTableCount;

    /** @brief pointer to frame table
     */
    GT_U32 txFrameTableStartPointer;

    /** @brief payload type. should be the same like RX payload type
     */
    GT_U32 txPayloadType;

}MV_HWS_D2D_FRAME_GEN_CONFIG_STC;

/**
* @struct MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC
 *
 * @brief
*/
typedef struct
{
    /** @brief RX payload type
     *         0: Continued 16b Byte Seq (frame payload bytes following the first
     *            32 byte header contains 2 byte incrementing sequence that are
     *            continued from frame to frame).
     *         1: Restarted 16b Byte Seq (frame payload bytes following the first
     *            32 byte header contains 2 byte incrementing sequence that are
     *            restarted every frame).
     *         2: Fixed payload (frame payload bytes following the first 32 byte
     *            header contains 4 fixed bytes)
     */
    GT_U32 rxPayloadType;

    /** @brief mask
     *         Bit0: Select whether Payload byte sequence err is counted as good or bad frames
     *         Bit1: Select whether Frame sequence err is counted as good or bad frames
     *         Bit2: Select whether err marked frames is counted as good or bad frames
     *         Bit3: Select whether frame length mismatch is counted as good or bad frames
     */
    GT_U32 rxBadCntMask;

}MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC;


/**
* @struct MV_HWS_D2D_FRAME_TABLE_CONFIG_STC
 *
 * @brief
*/
typedef struct
{
    /** @brief number of times frame to be sent
     */
    GT_U32 frameRepeatCnt;

    /** @brief Number of Idle Wrd following the frame header
     */
    GT_U32 idleWord;

    /** @brief frame command
     *         0: Normal SOP / EOP marked frame
     *         1: Missing SOP. Frame is transmitted without SOP.
     *         2: Missing EOP. Frame is transmitted without EOP.
     *         3: EOP_ERR. Frame is abort marked.
     */
    GT_U32 frameCommand;

    /** @brief Maximum frame size indicator such that frames are
     *         generated in the range [FrmLenMin; FrmLenMin + 2**FrmLenMax - 1].
     *         Set to zero to ensure fixed length frames
     */
    GT_U32 frameLengthMax;

    /** @brief Minimum frame size
     */
    GT_U32 frameLengthMin;

    /** @brief Frame payload
     */
    GT_U32 framePayload;

    /** @brief frame header array
     */
    GT_U32 frameHeader[HWS_D2D_MAC_DBG_FRM_TBL_HEADER_LEN];

}MV_HWS_D2D_FRAME_TABLE_CONFIG_STC;


GT_STATUS hwsD2dMacChannelEnable
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL enable

);

/**
* @internal hwsD2dMacFrameTableConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
GT_STATUS hwsD2dMacFrameTableConfigSet
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              port    ,   /* port number 1024... */
    GT_U32                              frameIndex, /* 0..31 */
    MV_HWS_D2D_FRAME_TABLE_CONFIG_STC   *configPtr
);

/**
* @internal hwsD2dMacFrameGenConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
GT_STATUS hwsD2dMacFrameGenConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          port,   /*port number 1024... */
    GT_BOOL                         enable,
    GT_U32                          channelNum,
    MV_HWS_D2D_FRAME_GEN_CONFIG_STC *configPtr
);

/**
* @internal hwsD2dMacFrameCheckerConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
GT_STATUS hwsD2dMacFrameCheckerConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          port,   /* port number, D2D is 1024... */
    GT_BOOL                         enable,
    GT_U32                          channelNum,
    MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC *configPtr
);

/**
* @internal hwsD2dMacFrameGenStatusGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
GT_STATUS hwsD2dMacFrameGenStatusGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  port,       /* port number, D2D is 1024... */
    GT_U32  channelNum, /* 0..16 */
    GT_U32  *frameGoodCnt,
    GT_U32  *frameBadCnt
);

/**
* @internal hwsD2dMacIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS hwsD2dMacIfInit
(
    GT_U8 devNum,
    MV_HWS_MAC_FUNC_PTRS *funcPtrArray
);

/**
* @internal hwsD2dMacIfClose function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
void hwsD2dMacIfClose(GT_U8 devNum);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsD2dMacIf_H */

