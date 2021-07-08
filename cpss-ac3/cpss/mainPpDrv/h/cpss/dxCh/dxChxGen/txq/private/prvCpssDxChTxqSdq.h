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
* @file prvCpssDxChTxqSdq.h
*
* @brief CPSS SIP6 TXQ SDQ low level configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqSdq
#define __prvCpssDxChTxqSdq


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES
 *
 * @brief SDQ port thresholds.Controls when the credits are requested for port from PDQ
*/
typedef struct
{
        /** @brief  Port Back pressure low threshold
     */
    GT_U32 portBackPressureLowThreshold;

           /** @brief  Port Back pressure high threshold
        */

    GT_U32 portBackPressureHighThreshold;
}
PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES;


/*## type cpssFalconTxqSdqQueueAttributesStruct */
typedef struct
{
    GT_BOOL enable;      /*## attribute enable */
    /* Aging threshold in clock cycles */
    GT_U32 agingThreshold;      /*## attribute agingThreshold */
    GT_U32 highCreditThreshold;     /*## attribute highCreditThreshold */
    GT_U32 lowCreditTheshold;       /*## attribute lowCreditTheshold */
    /* Minimum credit budget for selection. */
    GT_U32 negativeCreditThreshold;     /*## attribute negativeCreditThreshold */
    GT_U32 tc;      /*## attribute tc */
    GT_U32 semiEligThreshold;
}
PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES;



typedef struct
{
    /* Port high range queue */
    GT_U32 hiQueueNumber;       /*## attribute hiQueueNumber */
    /* Port low range queue */
    GT_U32 lowQueueNumber;      /*## attribute lowQueueNumber */
} PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE;



typedef struct
{
    GT_U32 prio0HighLimit;
    GT_U32 prio0LowLimit;
    GT_U32 prio1HighLimit;
    GT_U32 prio1LowLimit;
} PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE;


/**
* @internal prvCpssFalconTxqSdqQueueAttributesSet function
* @endinternal
*
* @brief   Queue attributes set
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
* @param[in] speed       - speed of port that contain the queue
* @param[in] enable       - queue enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS prvCpssFalconTxqSdqQueueAttributesSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN CPSS_PORT_SPEED_ENT speed
);

/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeSet function
* @endinternal
*
* @brief   Map port to queue
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] range                    -the  of the queues(APPLICABLE RANGES:0..399).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE range
);
/**
* @internal prvCpssFalconTxqSdqQueueAttributesGet function
* @endinternal
*
* @brief   Read Queue attributes
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the global number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
*
* @param[out] queueAttributesPtr       - pointer to queue attributes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqQueueAttributesGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
);

/**
* @internal prvCpssFalconTxqSdqQueueEnableSet function
* @endinternal
*
* @brief   Enable/disable queue
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] enable                - if GT_TRUE credit aging is enabled, otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableSet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNumber,
 IN GT_U32 tc,
 IN GT_BOOL enable
);
/**
* @internal prvCpssFalconTxqSdqQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/disable queue status
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the global number of SDQ (APPLICABLE RANGES:0..7).
*
* @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableGet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNumber,
 IN GT_U32 queueNumber,
 IN GT_BOOL * enablePtr
);

/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeGet function
* @endinternal
*
* @brief   Read port to queue from specific SDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE * rangePtr
);


/**
* @internal prvCpssFalconTxqSdqLocalPortEnableSet function
* @endinternal
*
* @brief   Set port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                - If equal GT_TRUE port is enabled at SDQ, else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortEnableSet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNum,
 IN GT_BOOL enable
);
/**
* @internal prvCpssFalconTxqSdqLocalPortEnableGet function
* @endinternal
*
* @brief   Get port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   -the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] enablePtr                - If equal GT_TRUE port is enabled at SDQ,else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortEnableGet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNum,
 OUT GT_BOOL * enablePtr
);
/**
* @internal prvCpssFalconTxqSdqPortAttributesSet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   -the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[in] speed                - oort speed
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqPortAttributesSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    IN CPSS_PORT_SPEED_ENT speed
);

/**
* @internal prvCpssFalconTxqSdqPortAttributesGet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   -the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] portAttributesPtr                - (pointer to) port thresholds
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqPortAttributesGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
);


GT_STATUS prvCpssFalconTxqSdqQueueStrictPrioritySet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL sp
);


GT_STATUS prvCpssFalconTxqSdqSelectListSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE * rangePtr
);

/**
* @internal prvCpssFalconTxqSdqSelectListGet function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] range                    -the  range of the queues
*
* @retval GT_OK                - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/
GT_STATUS prvCpssFalconTxqSdqSelectListGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE * rangePtr
);



GT_STATUS prvCpssFalconTxqSdqGlobalConfigEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN  GT_BOOL agingEnable,
    IN  GT_BOOL seligEnable
);


GT_STATUS prvCpssFalconTxqSdqQCreditBalanceGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNum,
    OUT  GT_U32  * balance
);

GT_STATUS prvCpssFalconTxqSdqEligStateGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNum,
    OUT  GT_U32  * eligStatePtr
);
/**
* @internal prvCpssFalconTxqSdqPfcControlEnableSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] tileNum                   - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset               queue offset[0..15]
* @param[in] enable                        port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableSet
(
   IN GT_U8   devNum,
   IN GT_U32  tileNum,
   IN GT_U32  sdqNum,
   IN GT_U32  localPortNumber,
   IN GT_U32  queueNumber,
   IN GT_BOOL enable
);
/**
* @internal prvCpssFalconTxqSdqPfcControlEnableGet function
* @endinternal
*
* @brief   Get enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -      physical device number
* @param[in] tileNum                   -       the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -       the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             -    local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset                    queue offset[0..15]
* @param[in] enablePtr                        (pointer to)port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/

GT_STATUS prvCpssFalconTxqSdqPfcControlEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueNumber,
    IN GT_BOOL *enablePtr
);
/**
* @internal prvCpssFalconTxqSdqQueueTcSet function
* @endinternal
*
* @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -      physical device number
* @param[in] tileNum                   -       the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -       the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             -    local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset                    queue offset[0..15]
* @param[in] tc                                       tc that is mapped to.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/

GT_STATUS prvCpssFalconTxqSdqQueueTcSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_U32 tc
);

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapGet  function
* @endinternal
*
* @brief   Get Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    -physical device number
* @param[in] tileNum                   -  the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset               queue offset[0..15]
* @param[in] enableBtmpPtr           (pointer to)port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  *enableBtmpPtr
);
/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] tileNum                   - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset               queue offset[0..15]
* @param[in] enableBtmp                        port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  enableBtmp
);

/**
* @internal prvCpssFalconTxqUtilsPortFlushSet function
* @endinternal
*
* @brief  Set port to "credit ignore" mode.This make port to transmit whenever there is data to send ignoring credits.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] tileNum                   -  the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             local port (APPLICABLE RANGES:0..8).
* @param[in] enable                                                             Enable/disable "credit ignore" mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortFlushSet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNum,
 IN GT_BOOL enable
);
/**
* @internal prvCpssFalconTxqSdqQueueStatusDump function
* @endinternal
*
* @brief  Dump queue eligeble status in parsed format
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] tileNum                   - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] startQ                         first queue to dump
* @param[in] size             -              number of queues to dump.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqQueueStatusDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 sdqNum,
    IN  GT_U32 startQ,
    IN  GT_U32 size
);

/**
* @internal prvCpssFalconTxqSdqQueueStrictPriorityGet function
* @endinternal
*
* @brief   Get queue strict priority bit
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   -the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] queueNumber             - local queue number  (APPLICABLE RANGES:0..399).
*
* @param[out] spPtr                - (pointer to)strict priority bit
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueStrictPriorityGet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  *spPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqSdq */

