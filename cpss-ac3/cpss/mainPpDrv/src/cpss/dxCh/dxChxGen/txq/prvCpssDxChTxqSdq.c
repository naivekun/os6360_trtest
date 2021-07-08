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
* @file prvCpssDxChTxqSdq.c
*
* @brief CPSS SIP6 TXQ Sdq low level configurations.
*
* @version   1
********************************************************************************
*/

/*Synced to Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_SCN_IP \TXQ_SCN_IP {7.0.5} \TXQ_SDQ */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/prvCpssFalconTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqSdq.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>


#define PRV_TH_GRANULARITY_SHIFT 8 /* 256*/
#define PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT 13 /* 8K*/



/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   -  port speed
* @param[out] queueAttributesPtr                  -(pointer to) queue thresholds
*
* @retval GT_OK                - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/

static GT_STATUS prvCpssFalconTxqSdqInitQueueThresholdAttributes
(
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
);

static GT_STATUS prvCpssFalconTxqSdqInitPortAttributes
(
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES * portAttributesPtr
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
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);


    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if((range.hiQueueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)||
        (range.lowQueueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)||
        (range.lowQueueNumber>=range.hiQueueNumber))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET,
                            TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE,
                            range.lowQueueNumber);




     rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_RANGE_LOW_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        &regValue);
     if(rc!=GT_OK)
     {
        return rc;
     }

     regValue = 0;


     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET,
                                TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE,
                                range.hiQueueNumber);





     rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_RANGE_HIGH_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        &regValue);

    return GT_OK;
}
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
)
{

    GT_STATUS rc;
    GT_U32 regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);


    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_RANGE_LOW_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        &regValue);

     rangePtr->lowQueueNumber= U32_GET_FIELD_MAC(regValue,
        TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET,
        TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE);


    if(rc!=GT_OK)
    {
        return rc;
    }

     rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_RANGE_HIGH_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        &regValue);

     rangePtr->hiQueueNumber = U32_GET_FIELD_MAC(regValue,
        TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET,
        TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE);

     return rc;

}
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
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32 size;
    GT_U32 value;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES  queueAttributes = {0};

     GT_U32 semiEligThreshold;



    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);



    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssFalconTxqSdqInitQueueThresholdAttributes(speed,&queueAttributes);

     if(rc !=GT_OK)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
     }

    if(queueAttributes.agingThreshold>= (1<<TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*According to Cider : "This threshold is 256B granule and is signed number defined by the msb"*/

    if((queueAttributes.highCreditThreshold>>PRV_TH_GRANULARITY_SHIFT)>= (1<<TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((queueAttributes.lowCreditTheshold>>PRV_TH_GRANULARITY_SHIFT)>= (1<<TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((queueAttributes.negativeCreditThreshold>>PRV_TH_GRANULARITY_SHIFT)>= (1<<TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((queueAttributes.semiEligThreshold>>PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT)>= (1<<TXQ_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(regValue,0,sizeof(GT_U32)*2);
    queueAttributes.enable = GT_TRUE;/*don't care - this bit does not work*/
    regValue[0] = 0;
    U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_SDQ_QUEUE_CFG_QUEUE_EN_FIELD_OFFSET,
                                TXQ_SDQ_QUEUE_CFG_QUEUE_EN_FIELD_SIZE,
                                queueAttributes.enable);


     U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET,
                                TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE,
                                queueAttributes.agingThreshold>>PRV_TH_GRANULARITY_SHIFT);



     U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET,
                                TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE,
                                queueAttributes.negativeCreditThreshold>PRV_TH_GRANULARITY_SHIFT);



     value = U32_GET_FIELD_MAC(queueAttributes.lowCreditTheshold>>PRV_TH_GRANULARITY_SHIFT,0,32 - TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET);
     size = 32 - TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;

     U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET,
                                size,
                                value);

     value = U32_GET_FIELD_MAC(queueAttributes.lowCreditTheshold>>PRV_TH_GRANULARITY_SHIFT,
                                32 - TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET,
                                TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE - (32 - TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET));

     size = TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE - (32 - TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET);

     U32_SET_FIELD_MASKED_MAC(regValue[1],0,
                                 size,
                                value);



     U32_SET_FIELD_MASKED_MAC(regValue[1],TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET-32,
                                TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE,
                                queueAttributes.highCreditThreshold>>PRV_TH_GRANULARITY_SHIFT);



     rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);

      if(rc!=GT_OK)
      {
         return rc;
      }

      /*Now set semi-elig threshold*/



      semiEligThreshold = queueAttributes.semiEligThreshold>>PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT;

      /*round up*/
      if(queueAttributes.semiEligThreshold%(1<<PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT)>((1<<(PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT-1))))
      {
            semiEligThreshold++;
      }

       rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CONFIG_REG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET,
                                       TXQ_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE,
                                        semiEligThreshold);


    return rc;


}

/**
* @internal prvCpssFalconTxqSdqCreditAgingEnableSet function
* @endinternal
*
* @brief   Enable/disable credit aging
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] enable                   - if GT_TRUE credit aging is enabled, otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqGlobalConfigEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN  GT_BOOL agingEnable,
    IN  GT_BOOL seligEnable
)
{
     GT_U32 regAddr,tileOffset ;
     GT_U32 regValue = 0;
     GT_STATUS rc;


     TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);


     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum/*tileId*/);
      regAddr+=tileOffset;



     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_GLOBAL_CONFIG_AGING_EN_FIELD_OFFSET,
                                 TXQ_SDQ_GLOBAL_CONFIG_AGING_EN_FIELD_SIZE,
                                 agingEnable?1:0);

      U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_GLOBAL_CONFIG_SELIG_EN_FIELD_OFFSET,
                                 TXQ_SDQ_GLOBAL_CONFIG_SELIG_EN_FIELD_SIZE,
                                 seligEnable?1:0);



     rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);

     return rc;
 }


/*******************************************************************************
* prvCpssFalconTxqSdqQueueEnableSet
*
* DESCRIPTION:
*      Enable/disable queue
*
* APPLICABLE DEVICES:
*        Falcon
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       devNum        -  device number
*       tileNum          - the tile number  (APPLICABLE RANGES:0..3).
*       sdqNum            -the local number of SDQ (APPLICABLE RANGES:0..7).
*       enable             - if GT_TRUE queue  is enabled , otherwise disabled
*
* OUTPUTS:
*       NONE
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong sdq number.
*       GT_HW_ERROR              - on writing to HW error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/

GT_STATUS prvCpssFalconTxqSdqQueueEnableSet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNumber,
 IN GT_U32 tc,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(tc>=CPSS_DXCH_SIP_6_MAX_TC_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

   TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNumber);



     rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PAUSE_TC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNumber,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PAUSE_TC_PAUSE_TC_0_FIELD_OFFSET+tc,
                                       1,
                                       /*reverse logic*/
                                        enable?0:1);

     return rc;



}

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
)
{
    GT_STATUS rc;
    GT_U32          regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }



     rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PAUSE_TC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNumber,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PAUSE_TC_PAUSE_TC_0_FIELD_OFFSET+queueNumber,
                                       1,
                                        &regValue);

     if(rc==GT_OK)
     {
          /*reverse logic*/
        *enablePtr = (regValue == 1)?GT_FALSE:GT_TRUE;
     }

     return rc;



}


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
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32 size,value;

   TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);


    TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }



    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CONFIG_REG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);


     if(rc!=GT_OK)
     {
        return rc;
     }

     queueAttributesPtr->tc = U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET,
        TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE);

     rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);
     if(rc!=GT_OK)
     {
        return rc;
     }

     queueAttributesPtr->enable = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_QUEUE_CFG_QUEUE_EN_FIELD_OFFSET,
        TXQ_SDQ_QUEUE_CFG_QUEUE_EN_FIELD_SIZE);

     queueAttributesPtr->agingThreshold = U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET,
        TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE);

     queueAttributesPtr->negativeCreditThreshold = U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET,
        TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE);

     queueAttributesPtr->negativeCreditThreshold <<=PRV_TH_GRANULARITY_SHIFT;

     queueAttributesPtr->highCreditThreshold = U32_GET_FIELD_MAC(regValue[1],
        TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET-32,
        TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE);

     queueAttributesPtr->highCreditThreshold <<=PRV_TH_GRANULARITY_SHIFT;

     size = 32 - TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;

     queueAttributesPtr->lowCreditTheshold = 0;


     queueAttributesPtr->lowCreditTheshold= U32_GET_FIELD_MAC(regValue[0],
             TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET,
             size);

     value =  U32_GET_FIELD_MAC(regValue[1],
             0,
             TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE - size);

     queueAttributesPtr->lowCreditTheshold|= ((value)<<size);

     queueAttributesPtr->lowCreditTheshold <<=PRV_TH_GRANULARITY_SHIFT;


     return rc;


}


/**
* @internal prvCpssFalconTxqSdqLocalPortEnableSet function
* @endinternal
*
* @brief   Set port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   -the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                   - If equal GT_TRUE port is enabled at SDQ,else otherwise
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
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


       rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CONFIG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PORT_CONFIG_PORT_EN_0_FIELD_OFFSET,
                                       TXQ_SDQ_PORT_CONFIG_PORT_EN_0_FIELD_SIZE,
                                        enable?1:0);
    return  rc;

}

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
)
{

    GT_U32 value;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);
    TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);

     rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CONFIG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PORT_CONFIG_PORT_EN_0_FIELD_OFFSET,
                                       TXQ_SDQ_PORT_CONFIG_PORT_EN_0_FIELD_SIZE,
                                        &value);

     if(rc==GT_OK)
     {
        *enablePtr = (value==1)?GT_TRUE:GT_FALSE;
     }

    return rc;



}




/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   -  port speed
* @param[out] queueAttributesPtr                  -(pointer to) queue thresholds
*
* @retval GT_OK                - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/

static GT_STATUS prvCpssFalconTxqSdqInitQueueThresholdAttributes
(
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
)
{
           GT_STATUS rc = GT_OK;

            queueAttributesPtr->agingThreshold = 0x0;
            queueAttributesPtr->negativeCreditThreshold = 0;
            queueAttributesPtr->lowCreditTheshold = 149000 ;
            queueAttributesPtr->highCreditThreshold = 150000;

            switch(speed)
            {
                    case CPSS_PORT_SPEED_10000_E:
                        queueAttributesPtr->semiEligThreshold =  7500;
                        break;
                    case CPSS_PORT_SPEED_50000_E:
                        queueAttributesPtr->semiEligThreshold = 37500;
                        break;
                    case CPSS_PORT_SPEED_100G_E:
                        queueAttributesPtr->semiEligThreshold = 75000 ;
                        break;
                    case CPSS_PORT_SPEED_200G_E:
                        queueAttributesPtr->semiEligThreshold =  150000;
                        break;
                   case CPSS_PORT_SPEED_400G_E:
                        queueAttributesPtr->semiEligThreshold =  300000;
                        break;
                        /*Default initialization on power up. The speed is undefined yet*/
                    case CPSS_PORT_SPEED_NA_E:
                        queueAttributesPtr->semiEligThreshold =  0;
                        queueAttributesPtr->negativeCreditThreshold = 0;
                        queueAttributesPtr->lowCreditTheshold = (0x1FF) <<PRV_TH_GRANULARITY_SHIFT;
                        queueAttributesPtr->highCreditThreshold = (0x3FF)<<PRV_TH_GRANULARITY_SHIFT;
                        break;


                   default :
                       rc = GT_FAIL;
                      break;
            }

            return rc;
}

/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize port thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   -  port speed
* @param[out] portAttributesPtr                  -(pointer to) port thresholds
*
* @retval GT_OK                - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/
static GT_STATUS prvCpssFalconTxqSdqInitPortAttributes
(
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES * portAttributesPtr
)
{
    GT_STATUS rc = GT_OK;

    switch(speed)
    {
            case CPSS_PORT_SPEED_10000_E:
                portAttributesPtr->portBackPressureLowThreshold =  14200;
                break;
            case CPSS_PORT_SPEED_50000_E:
                portAttributesPtr->portBackPressureLowThreshold = 15000;
                break;
            case CPSS_PORT_SPEED_100G_E:
                portAttributesPtr->portBackPressureLowThreshold = 16000 ;
                break;
            case CPSS_PORT_SPEED_200G_E:
                portAttributesPtr->portBackPressureLowThreshold = 18000;
                break;
           case CPSS_PORT_SPEED_400G_E:
                portAttributesPtr->portBackPressureLowThreshold =  22000;
                break;
           default :
               rc = GT_FAIL;
              break;
    }

    portAttributesPtr->portBackPressureHighThreshold = portAttributesPtr->portBackPressureLowThreshold;

    return rc;
}

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
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  portAttributes;

    GT_STATUS rc ;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);


    rc = prvCpssFalconTxqSdqInitPortAttributes(speed,&portAttributes);

     if(rc !=GT_OK)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
     }

      rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_OFFSET,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_SIZE,
                                        portAttributes.portBackPressureLowThreshold);

     if(rc !=GT_OK)
     {
        return rc;
     }


      rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_OFFSET,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_SIZE,
                                        portAttributes.portBackPressureHighThreshold);


      return rc;




}

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
)
{
       GT_STATUS rc ;

      TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

       TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);

       TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);

       rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_OFFSET,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_SIZE,
                                        &(portAttributesPtr->portBackPressureLowThreshold));

     if(rc !=GT_OK)
     {
        return rc;
     }


      rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_OFFSET,
                                       TXQ_SDQ_PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_SIZE,
                                        &(portAttributesPtr->portBackPressureHighThreshold));


      return rc;
}


GT_STATUS prvCpssFalconTxqSdqQueueStrictPrioritySet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL sp
)
{
      GT_STATUS rc;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
     }





      rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CONFIG_REG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET,
                                       TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE,
                                      sp?1:0);

      return rc;


}

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
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL *spPtr
)
{
     GT_STATUS rc;
     GT_U32 tmp;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
     }

      rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CONFIG_REG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET,
                                       TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE,
                                      &tmp);

    *spPtr= tmp?GT_TRUE:GT_FALSE;

     return rc;


}


GT_STATUS prvCpssFalconTxqSdqSelectListSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE * rangePtr
)
{
        GT_STATUS rc;

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

         TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

         TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);

    if((rangePtr->prio0LowLimit >=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)||
        (rangePtr->prio1LowLimit>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC))
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if((rangePtr->prio0HighLimit >=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)||
        (rangePtr->prio1HighLimit>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC))
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


            if((rangePtr->prio0LowLimit >rangePtr->prio0HighLimit)||
        (rangePtr->prio1LowLimit >rangePtr->prio1HighLimit))
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }




         rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_LOW_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE,
                                        rangePtr->prio0LowLimit);

         if(rc!=GT_OK)
         {
            return rc;
         }


         rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_HIGH_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE,
                                        rangePtr->prio0HighLimit);

         if(rc!=GT_OK)
         {
            return rc;
         }



         rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_LOW_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum+ CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE,
                                        rangePtr->prio1LowLimit);

         if(rc!=GT_OK)
         {
            return rc;
         }



         rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_HIGH_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum+ CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE,
                                        rangePtr->prio1HighLimit);






         return rc;
}


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
)
{
        GT_STATUS rc;

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

         TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

         TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);



         rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_LOW_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE,
                                        &(rangePtr->prio0LowLimit));

         if(rc!=GT_OK)
         {
            return rc;
         }


         rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_HIGH_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE,
                                        &(rangePtr->prio0HighLimit));

         if(rc!=GT_OK)
         {
            return rc;
         }



         rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_LOW_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum+ CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE,
                                        &(rangePtr->prio1LowLimit));

         if(rc!=GT_OK)
         {
            return rc;
         }



         rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE_HIGH_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum+ CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET,
                                       TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE,
                                        &(rangePtr->prio1HighLimit));




         return rc;
}


GT_STATUS prvCpssFalconTxqSdqQCreditBalanceGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNum,
    OUT  GT_U32  * balancePtr
)
{

    GT_STATUS rc;

     rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_OFFSET,
                                        TXQ_SDQ_QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_SIZE,
                                        balancePtr);

     return rc;


}

GT_STATUS prvCpssFalconTxqSdqEligStateGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNum,
    OUT  GT_U32  * eligStatePtr
)
{

    GT_STATUS rc;

     rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNum,
                                        eligStatePtr);

     return rc;


}
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
    IN GT_U8  devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueOffset,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueOffset>=1<<TXQ_SDQ_PFC_CONTROL_TC_EN0_FIELD_SIZE)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNumber);

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PFC_CONTROL_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNumber,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_PFC_CONTROL_TC_EN0_FIELD_OFFSET+queueOffset,
                                        1,
                                        enable?1:0);

    return rc;
}

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
)
{
    GT_STATUS rc;
    GT_U32          regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=1<<TXQ_SDQ_PFC_CONTROL_TC_EN0_FIELD_SIZE)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PFC_CONTROL_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNumber,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_PFC_CONTROL_TC_EN0_FIELD_OFFSET+queueNumber,
                                        1,
                                        &regValue);

     if(rc==GT_OK)
     {
        *enablePtr = (regValue == 1)?GT_TRUE:GT_FALSE;
     }

     return rc;



}

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
)
{

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(tc>= (1<<TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CONFIG_REG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                         queueNumber,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                         TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET,
                                         TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE,
                                         tc);
}

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
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PFC_CONTROL_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNumber,
                                        &enableBtmp);

    return rc;
}

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
    IN GT_U32 *enableBtmpPtr
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PFC_CONTROL_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNumber,
                                        enableBtmpPtr);

    return rc;
}

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
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


       rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CONFIG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        localPortNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_SDQ_PORT_CONFIG_CRDT_IGN_0_FIELD_OFFSET,
                                       TXQ_SDQ_PORT_CONFIG_CRDT_IGN_0_FIELD_SIZE,
                                        enable?1:0);
    return  rc;

}


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
)
{
    GT_U32           i;
    GT_STATUS        rc;
    GT_U32 eligState,credit,qcn,dry,inPortCnt,InSel,enq,SemiElig;

    cpssOsPrintf("\nQueue Status SDQ %d\n",sdqNum);

    cpssOsPrintf("\n+-------------+------+---+--------+---------+-------+----+--------+");
    cpssOsPrintf("\n| queueNumber |credit|QCN|  DRY   |inPortCnt|InSel  |ENQ |SemiElig|");
    cpssOsPrintf("\n+-------------+------+---+--------+---------+-------+----+--------+");

    for(i =startQ;i<(startQ+size);i++)
    {

       rc = prvCpssFalconTxqSdqEligStateGet(devNum,tileNumber,sdqNum,i,&eligState);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n",rc );
            return rc;
        }

        credit = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_QUEUE_ELIG_STATE_CRDT_FIELD_OFFSET,
            TXQ_SDQ_QUEUE_ELIG_STATE_CRDT_FIELD_SIZE);

        qcn = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_QUEUE_ELIG_STATE_QCN_FIELD_OFFSET,
            TXQ_SDQ_QUEUE_ELIG_STATE_QCN_FIELD_SIZE);

        dry = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_QUEUE_ELIG_STATE_DRY_FIELD_OFFSET,
            TXQ_SDQ_QUEUE_ELIG_STATE_DRY_FIELD_SIZE);

        inPortCnt = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_QUEUE_ELIG_STATE_INPORTCNT_FIELD_OFFSET,
            TXQ_SDQ_QUEUE_ELIG_STATE_INPORTCNT_FIELD_SIZE);

        InSel =  U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_QUEUE_ELIG_STATE_INSEL_FIELD_OFFSET,
            TXQ_SDQ_QUEUE_ELIG_STATE_INSEL_FIELD_SIZE);

        enq = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_QUEUE_ELIG_STATE_ENQ_FIELD_OFFSET,
            TXQ_SDQ_QUEUE_ELIG_STATE_ENQ_FIELD_SIZE);

        SemiElig = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_QUEUE_ELIG_STATE_SELIG_FIELD_OFFSET,
            TXQ_SDQ_QUEUE_ELIG_STATE_SELIG_FIELD_SIZE);


        cpssOsPrintf("\n|%13d|%6d|%2d|%8d|%10d|%7d|%4d|%8d|",i,credit,qcn,dry,inPortCnt,InSel,enq,SemiElig);
        cpssOsPrintf("\n+-------------+------+---+--------+---------+-------+----+--------+");
    }

    cpssOsPrintf("\n");

    return GT_OK;
}



