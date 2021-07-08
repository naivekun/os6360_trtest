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
* @file prvCpssDxChPortDynamicPizzaArbiter.h
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter
*
* @version   9
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_DYNAMIC_PIZZA_ARBITER_H
#define __PRV_CPSS_DXCH_PORT_DYNAMIC_PIZZA_ARBITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>


extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterSpeedConv
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speedEnm,
    OUT GT_U32              *speedPtr
);


extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfInit
(
    IN  GT_U8  devNum
);

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfTMBWSet function
* @endinternal
*
* @brief   define BW of TM units
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*                                      portNum  - physical port number
*                                      portSpeed - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfTMBWSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  bwMbps
);


extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfCheckSupport
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
);

extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
);

extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfCCFCClientConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  macNum,
    IN  GT_U32                  txqNum,
    IN  GT_U32                  portSpeedInMBit
);


extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelCheckSupport
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
);

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      portSpeed - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfIlknChannelConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
);


extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfDevStateGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    OUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
);

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedGet function
* @endinternal
*
* @brief   Get Pizza Arbiter Unit Channel Speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in]  devNum                   - device number
* @param[in]  unitId                   - Id of unit containing pizza arbiter
* @param[in]  removeOtherChannels      - GT_TRUE - all new slice map specified in this function
*                                                  parameters channel arrays
*                                        GT_FALSE - update only speed of specified channels
*                                                speed of other channels left unchanged
* @param[in]  channelArrSize           - size of arrays for channels
* @param[out] channelNumArrPtr         - (pointer to)array of channel numbers
* @param[out] channelSpeedInMBitArrPtr - (pointer to)array of channel speeds in megabit/sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId,
    IN  GT_U32                  channelArrSize,
    OUT GT_U32                  *channelArrActualSizePtr,
    OUT GT_U32                  *channelNumArrPtr,
    OUT GT_U32                  *channelSpeedInMBitArrPtr
);

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedOverride function
* @endinternal
*
* @brief   Override Pizza Arbiter Unit Channel Speed (ingnoring previous state)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] unitId                   - Id of unit containing pizza arbiter
* @param[in] channelArrSize           - size of arrays of specified channels
* @param[in] channelNumArrPtr         - (pointer to)array of channel numbers
* @param[in] channelSpeedInMBitArrPtr - (pointer to)array of channel speeds in megabit/sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedOverride
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId,
    IN  GT_U32                  channelArrSize,
    IN  GT_U32                  *channelNumArrPtr,
    IN  GT_U32                  *channelSpeedInMBitArrPtr
);

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedUpdate function
* @endinternal
*
* @brief   Update Pizza Arbiter Unit Channel Speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] unitId                   - Id of unit containing pizza arbiter
* @param[in] channelArrSize           - size of arrays of specified channels
* @param[in] channelNumArrPtr         - (pointer to)array of channel numbers
* @param[in] channelSpeedInMBitArrPtr - (pointer to)array of channel speeds in megabit/sec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedUpdate
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId,
    IN  GT_U32                  channelArrSize,
    IN  GT_U32                  *channelNumArrPtr,
    IN  GT_U32                  *channelSpeedInMBitArrPtr
);

/**
* @internal prvCpssDxChPortPizzaArbiterTxFifoChannelSpeedFactorSet function
* @endinternal
*
* @brief   Set Bandwidth speed factor of TX_FIFO Unit Pizza Arbiter Channel
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port mapped to TX_FIFO Pizza Arbitter Channel
*                                       being configured
* @param[in] speedFactor              - TX_FIFO Pizza Arbiter Channel speed factor in percent
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter or not enough Unit BW for ports summary BW
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterTxFifoChannelSpeedFactorSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              speedFactor
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

