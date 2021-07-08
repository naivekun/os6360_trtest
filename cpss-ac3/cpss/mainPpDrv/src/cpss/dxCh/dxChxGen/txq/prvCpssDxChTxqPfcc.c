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
* @file prvCpssDxChTxqPfcc.c
*
* @brief CPSS SIP6 TXQ Pfcc low level configurations.
*
* @version   1
********************************************************************************
*/

/*Synced to \Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.11}*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/prvCpssFalconTxqPfccRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPfcc.h>



#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>


#define PRV_MAX_PFCC_RETRY_MAC 10




/**
* @internal prvCpssFalconTxqPfccCfgTableEntrySet function
* @endinternal
*
* @brief   Write  entry to PFCC CFG table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[in] entryPtr         - PFCC CFG table entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccCfgTableEntrySet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 index,
    IN  PRV_CPSS_PFCC_CFG_ENTRY_STC * entryPtr
)
{

    GT_STATUS rc;
    GT_U32 regValue = 0;
    GT_U32 maxTableSize = PRV_CPSS_DXCH_SIP_6_PFCC_CFG_TABLE_SIZE;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if (prvCpssDxchFalconCiderVersionGet()>PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
        maxTableSize = PRV_CPSS_DXCH_SIP_6_PFCC_CFG__LATEST_CIDER_TABLE_SIZE;
    }


    if(index>=maxTableSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Index %d is bigger(equal)   then %d",index,maxTableSize);
    }

    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC ||entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR)&&
        (entryPtr->globalTc >= 1<<TXQ_PFCC_PFCC_CFG_TC_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Tc %d is bigger(equal)   then %d",entryPtr->globalTc,1<<TXQ_PFCC_PFCC_CFG_TC_FIELD_SIZE);
    }


    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)&&
        (entryPtr->portDma >= 1<<TXQ_PFCC_PFCC_CFG_PORT_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Port %d is bigger(equal)   then %d",entryPtr->portDma,1<<TXQ_PFCC_PFCC_CFG_PORT_FIELD_SIZE);
    }


    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)&&
        (entryPtr->tcBitVecEn>= 1<<TXQ_PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tcBitVecEn %d is bigger(equal)  then %d",entryPtr->tcBitVecEn,1<<TXQ_PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE);
    }


    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)&&
        (entryPtr->numberOfBubbles>= 1<<TXQ_PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "numberOfBubbles %d is bigger(equal)  then %d",entryPtr->numberOfBubbles,1<<TXQ_PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE);
    }




     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_OFFSET,
                            TXQ_PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_SIZE,
                            entryPtr->entryType);

     if(entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)
     {

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_OFFSET,
                            TXQ_PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE,
                            entryPtr->numberOfBubbles);
     }
     else if (entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC ||
        entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR)
     {

            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_PFCC_CFG_TC_FIELD_OFFSET,
                            TXQ_PFCC_PFCC_CFG_TC_FIELD_SIZE,
                            entryPtr->globalTc);
     }
     else
     {

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_PFCC_CFG_PORT_FIELD_OFFSET,
                                TXQ_PFCC_PFCC_CFG_PORT_FIELD_SIZE,
                                entryPtr->portDma);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_OFFSET,
                                TXQ_PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_SIZE,
                                entryPtr->pfcMessageTrigger);

       U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_OFFSET,
                                TXQ_PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE,
                                entryPtr->tcBitVecEn);
       }


    rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E+tileNum,
                                    index,
                                    &regValue);



    return rc;

}

/**
* @internal prvCpssFalconTxqPfccCfgTableEntryGet function
* @endinternal
*
* @brief   Read from  entry to PFCC CFG table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[out] entryPtr         - PFCC CFG table entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqPfccCfgTableEntryGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 index,
    OUT  PRV_CPSS_PFCC_CFG_ENTRY_STC * entryPtr
)
{

    GT_STATUS rc;
    GT_U32 regValue = 0;
    GT_U32 maxTableSize = PRV_CPSS_DXCH_SIP_6_PFCC_CFG_TABLE_SIZE;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if (prvCpssDxchFalconCiderVersionGet()>PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
        maxTableSize = PRV_CPSS_DXCH_SIP_6_PFCC_CFG__LATEST_CIDER_TABLE_SIZE;
    }

    if(index>=maxTableSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Index %d is bigger(equal)   then %d",index,maxTableSize);
    }


    rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E+tileNum,
                                    index,
                                    &regValue);

     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntryGet  failed \n");
     }

    entryPtr->entryType = (PRV_CPSS_PFCC_CFG_ENTRY_TYPE_ENT) U32_GET_FIELD_MAC(regValue,TXQ_PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_OFFSET,
                            TXQ_PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_SIZE);

     if(entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)
    {
        entryPtr->numberOfBubbles = U32_GET_FIELD_MAC(regValue,TXQ_PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_OFFSET,
                            TXQ_PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE);
     }
     else if ((entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC)||
        (entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR))
     {

        entryPtr->globalTc = U32_GET_FIELD_MAC(regValue,TXQ_PFCC_PFCC_CFG_TC_FIELD_OFFSET,
                            TXQ_PFCC_PFCC_CFG_TC_FIELD_SIZE);
      }
     else
     {

        entryPtr->portDma = U32_GET_FIELD_MAC(regValue,TXQ_PFCC_PFCC_CFG_PORT_FIELD_OFFSET,
                                TXQ_PFCC_PFCC_CFG_PORT_FIELD_SIZE);

        entryPtr->pfcMessageTrigger = (GT_BOOL)U32_GET_FIELD_MAC(regValue,TXQ_PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_OFFSET,
                                TXQ_PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_SIZE);

        entryPtr->tcBitVecEn = U32_GET_FIELD_MAC(regValue,TXQ_PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_OFFSET,
                                TXQ_PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE );
      }



    return rc;

}

/**
* @internal prvCpssFalconTxqPfccUnitCfgGet function
* @endinternal
*
* @brief   Write to  PFCC unit global configuration register
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[in] initDbPtr         - PFCC unit configuration data base
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqPfccUnitCfgSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  PRV_CPSS_PFCC_TILE_INIT_STC * initDbPtr
)
{
        GT_STATUS rc;
        GT_U32 regAddr,tileOffset,regValue = 0;
        GT_U32 maxTableSize = PRV_CPSS_DXCH_SIP_6_PFCC_CFG_TABLE_SIZE;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        if (prvCpssDxchFalconCiderVersionGet()>PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
        {
            maxTableSize = PRV_CPSS_DXCH_SIP_6_PFCC_CFG__LATEST_CIDER_TABLE_SIZE;
        }

        if(initDbPtr->pfccLastEntryIndex>=maxTableSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "pfccLastEntryIndex %d is bigger(equal)   then %d",initDbPtr->pfccLastEntryIndex,maxTableSize);
        }

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_OFFSET,
                            TXQ_PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_SIZE,
                            initDbPtr->isMaster);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_OFFSET,
                           TXQ_PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_SIZE,
                            initDbPtr->pfccLastEntryIndex);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_OFFSET,
                            TXQ_PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_SIZE,
                            initDbPtr->pfccEnable);


        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.global_pfcc_cfg;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;

        rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);

         return rc;


}

/**
* @internal prvCpssFalconTxqPfccUnitCfgGet function
* @endinternal
*
* @brief   Read PFCC unit global configuration register
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[out] initDbPtr         - PFCC unit configuration data base
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccUnitCfgGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    OUT  PRV_CPSS_PFCC_TILE_INIT_STC  *initDbPtr
)
{
        GT_STATUS rc;
        GT_U32 regAddr,tileOffset,regValue = 0;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);


        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.global_pfcc_cfg;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;

         rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccUnitCfgGet  failed \n");
         }

        initDbPtr->isMaster =(GT_BOOL) U32_GET_FIELD_MAC(regValue,
                TXQ_PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_OFFSET,
                TXQ_PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_SIZE);


        initDbPtr->pfccEnable=(GT_BOOL)  U32_GET_FIELD_MAC(regValue,
                    TXQ_PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_OFFSET,
                    TXQ_PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_SIZE);



        initDbPtr->pfccLastEntryIndex = U32_GET_FIELD_MAC(regValue,
                     TXQ_PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_OFFSET,
                     TXQ_PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_SIZE);


         return rc;


}

/**
* @internal prvCpssFalconTxqPfccPortTcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per  port and traffic class.
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] masterTileNum          -master tile number
* @param[in] tc           - traffic class [0..7](packet attribute) or 0xFF for source port
* @param[in] dmaNumber              - Source port dma number
* @param[out] pfcCounterValuePtr         -counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccPortTcCounterGet
(
  IN  GT_U8   devNum,
  IN  GT_U32  masterTileNum,
  IN  GT_U8   tc,
  IN  GT_U32  dmaNumber,
  OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regValue = 0;
    GT_U32 counter = 0;
    GT_BOOL dataValid = GT_FALSE;
    GT_U32 startIndex,localPort,dpNum,tileNum;
    PRV_CPSS_PFCC_CFG_ENTRY_STC  entry;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(masterTileNum);

    /*First check if  PFCC table contain this entry*/

    rc = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(devNum,dmaNumber,&tileNum,&dpNum,&localPort);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert  failed \n");
    }

    startIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*localPort+dpNum+tileNum*(MAX_DP_IN_TILE(devNum));
    rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,masterTileNum,startIndex,&entry);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntryGet  failed \n");
    }


    if((entry.entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)&&(entry.tcBitVecEn&1<<tc))
    {
        /*Write source port to register*/

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Source_Port_Requested_For_Read;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,masterTileNum);
        regAddr+=tileOffset;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_REQUESTED_TO_READ_FIELD_OFFSET,
                                TXQ_PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_REQUESTED_TO_READ_FIELD_SIZE,
                                dmaNumber);

         rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);

         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssHwPpWriteRegister  failed \n");
         }

          /*Busy wait until the value is latched*/
          while(counter <300)counter++;

         /*Now read*/
          counter = 0;

          if(tc  == PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC)
          {
            regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Source_Port_Counter;
            tileOffset = prvCpssSip6TileOffsetGet(devNum,masterTileNum);
            regAddr+=tileOffset;
            rc = prvCpssHwPpGetRegField(devNum,regAddr,TXQ_PFCC_SOURCE_PORT_COUNTER_SOURCE_PORT_COUNTER_VALUE_FOR_DEBUG_FIELD_OFFSET,
             TXQ_PFCC_SOURCE_PORT_COUNTER_SOURCE_PORT_COUNTER_VALUE_FOR_DEBUG_FIELD_SIZE,pfcCounterValuePtr);

          }
          else
          {
            while(counter<PRV_MAX_PFCC_RETRY_MAC)
            {
                 rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

                 if(rc!=GT_OK)
                 {
                        return rc;
                 }

#ifdef ASIC_SIMULATION  /* Yet not implemented in simulation */
                 dataValid = GT_TRUE;
#else
                 dataValid = (GT_BOOL)U32_GET_FIELD_MAC(regValue,TXQ_PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_COUNTER_FOR_DEBUG_VALID_FIELD_OFFSET+tc,
                                        TXQ_PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_COUNTER_FOR_DEBUG_VALID_FIELD_SIZE );
#endif

                 if(dataValid == GT_TRUE)
                 {
                     break;
                 }
                 else
                 {
                     counter++;
                 }
              }

         if(counter ==PRV_MAX_PFCC_RETRY_MAC)
          {
                rc = GT_FAIL;
          }
          else
          {

             rc = prvCpssDxChReadTableEntryField(devNum,
                    CPSS_DXCH_SIP6_TXQ_PFCC_SOURCE_PORT_COUNTER_VALUE_E+masterTileNum,
                    tc,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                    TXQ_PFCC_SOURCE_PORT_COUNTER_VALUE_SOURCE_PORT_TC_COUNTER_VALUE_FOR_DEBUG_FIELD_OFFSET,
                    TXQ_PFCC_SOURCE_PORT_COUNTER_VALUE_SOURCE_PORT_TC_COUNTER_VALUE_FOR_DEBUG_FIELD_SIZE,
                    pfcCounterValuePtr);
           }
          }

          return rc;
    }
    else
    {
         /*No threshold was set for this port/TC . No information is availeble*/
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

}

/**
* @internal prvCpssFalconTxqPfccPortTcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per  global  traffic class.
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -   PP's device number.
* @param[in] tileNum                         tile number
* @param[in] tc         -                        traffic class [0..7](packet attribute)
* @param[out] pfcCounterValuePtr         -counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccGlobalTcCounterGet
(
  IN  GT_U8   devNum,
  IN  GT_U32  tileNum,
  IN  GT_U8   tc,
  OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PFCC_GLOBAL_STATUS_COUNTERS_E+tileNum,
                                    tc,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                    TXQ_PFCC_GLOBAL_STATUS_COUNTERS_TC_COUNTER_VAL_FIELD_OFFSET,
                                    TXQ_PFCC_GLOBAL_STATUS_COUNTERS_TC_COUNTER_VAL_FIELD_SIZE,
                                    pfcCounterValuePtr);
   return rc;
}


/**
 * @internal  prvCpssFalconTxqPfccMapTcToPoolSet function
 * @endinternal
 *
 * @brief  Get tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in]  tileNum                                         Tile number
 * @param[in] trafClass                                        Traffic class [0..7]
 * @param[in]  poolIdPtr                                       Pool id[0..1]
 * @param[in]  update_EGF_QAG_TC_DP_MAPPER                   indication to update the CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E
 *                                                           (during init must be 'GT_FALSE' to reduce the number of HW accessing !)
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccMapTcToPoolSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               poolId,
    IN  GT_BOOL                              update_EGF_QAG_TC_DP_MAPPER
)
{
    GT_U32 regAddr,tileOffset;
    GT_STATUS rc;
    GT_U32 offset;
    GT_U32 oldPoolId;
    GT_U32  i,j,chunkValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(trafClass);

    if( poolId >= SIP6_SHARED_POOLS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*Get old mapping*/
     rc = prvCpssFalconTxqPfccMapTcToPoolGet(devNum,tileNum,trafClass,&oldPoolId);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
     }
     if(oldPoolId == poolId)
     {  /*Same value - do nothing*/
        return GT_OK;
     }

     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG;
     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

     offset = trafClass;

     if(poolId==1)
     {
        offset+=TXQ_PFCC_POOL_CFG_POOL_1_TC_EN_VEC_FIELD_OFFSET;
     }
     else if(poolId==2)
     {
        offset+=TXQ_PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET;
     }
      else if(poolId==3)
     {
        offset+=TXQ_PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET;
     }

    rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,1,1);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolSet  failed \n");
    }

    /*delete from old pool*/
    if(oldPoolId!=SIP6_SHARED_POOLS_NUM_CNS)
    {

        offset = trafClass;


        if(oldPoolId==1)
        {
           offset+=TXQ_PFCC_POOL_CFG_POOL_1_TC_EN_VEC_FIELD_OFFSET;
        }
        else if(oldPoolId==2)
        {
           offset+=TXQ_PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET;
        }
         else if(oldPoolId==3)
        {
           offset+=TXQ_PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET;
        }


        rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,1,0);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
        }
     }

    /*update TC_to_pool_CFG if required*/

    if (prvCpssDxchFalconCiderVersionGet()>PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
        rc =prvCpssFalconTxqPfccTcResourceModeSet(devNum,tileNum,trafClass,GT_FALSE,poolId,GT_FALSE,GT_FALSE);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcResourceModeSet  failed \n");
        }
     }


    if(update_EGF_QAG_TC_DP_MAPPER == GT_FALSE)
    {
        /* must not update the entries during initialization , because it modify
           same entries over and over again ! */
        /* saves 220608 times of calling
           prvCpssDxChWriteTableEntryField(..CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E..)

           this reduce cpssInitSystem time for GM from 37 seconds to 8 seconds (in VC10).
        */

        return GT_OK;
    }
    /*Now sync all the EGF entries that are mapped to this tc4Pfc.
            Table size is 4K ,each cell in the shadow array represent 32 entries,hence the array size is 128 (4K/32).

            Note:

        Tail-Drop/PFC pool usage is scaled to work well with up to two pools (0,1)
        When using pools #2,3 we have some limitations

        4 pools limitations:
        -        Tail Drop cannot be applied on pools 1,2,3
        -        CP will map all PFC flows to pool1 for tail drop
        -        Pool1 TailDrop must be disabled
        -        Only global taildrop is enabled


        If TC is mapped to pool2 or pool3 , in EGF  pool1 will be configured in this entry.

        Application responsibility is  to disable Pool1 TailDrop since the counting will be incorrect.

        See below table

        |Pool that TC is mapped at CPSS                       |                                      |                                                         |
        |(cpssDxChPortTxTcMapToSharedPoolSet)   | Pool mapped at txQ  |          Pool mapped at EGF table   |
        ======================================================================
       | 0                                                                            |      0                             |                     0                                   |
       | 1                                                                            |      1                             |                     1                                   |
       | 2                                                                            |      2                             |                     1                                   |
       | 3                                                                            |      3                             |                     1                                    |
        */

    /*update only if relevant*/
    if((oldPoolId==0 &&poolId>0)||(oldPoolId>0 &&poolId==0))
    {
        for(i=0;i<128;i++)
        {
           chunkValue = PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex[trafClass][i];
           for(j=0;chunkValue&&(j<32);j++,chunkValue>>=1)
           {
               if(chunkValue&0x1)
               {
                   rc = prvCpssDxChWriteTableEntryField(devNum,
                       CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E,
                       (i<<5)+j,
                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,9,1,
                       (poolId==0)?0:1);

                    if(rc!=GT_OK)
                   {
                       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating EGF_QAG_TC_DP_MAPPER table failed \n");
                   }
               }
           }
        }
     }

    return GT_OK;

}

/**
 * @internal  prvCpssFalconTxqPfccMapTcToPoolGet function
 * @endinternal
 *
 * @brief  Get tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in]  tileNum                                         Tile number
 * @param[in] trafClass                                        Traffic class [0..7]
 *@param[out]poolIdPtr                                       (Pointer to)Pool id[0..1]
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccMapTcToPoolGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    OUT  GT_U32                               *poolIdPtr
)
{
    GT_U32 rc;
    GT_U32 regValue;
    GT_U32 regAddr,tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(trafClass);

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
    }

    if((regValue>>TXQ_PFCC_POOL_CFG_POOL_TC_EN_VEC_FIELD_OFFSET)&(1<<trafClass))
    {
        *poolIdPtr=0;
    }
    else if((regValue>>TXQ_PFCC_POOL_CFG_POOL_1_TC_EN_VEC_FIELD_OFFSET)&(1<<trafClass))
    {
        *poolIdPtr=1;
    }
    else if((regValue>>TXQ_PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET)&(1<<trafClass))
    {
        *poolIdPtr=2;
    }
    else if((regValue>>TXQ_PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET)&(1<<trafClass))
    {
        *poolIdPtr=3;
    }
    else
    {
       *poolIdPtr=SIP6_SHARED_POOLS_NUM_CNS;/*unmapped*/
    }

    return GT_OK;
}

/**
 * @internal  prvCpssFalconTxqPfccTcMapVectorGet function
 * @endinternal
 *
 * @brief  Get tail drop mapping vector between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in]  tileNum                                         Tile number
 *@param[out]valuePtr                                       (Pointer to)Mapping vector
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccTcMapVectorGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    OUT GT_U32                               *valuePtr
)
{
    GT_U32 regAddr,tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    return prvCpssHwPpReadRegister(devNum,regAddr,valuePtr);
}

/**
* @internal prvCpssFalconTxqPfccTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] trafClass       Traffic class [0..7]
* @param[in] forceSet        If GT_TRUE set resource mode regardless of currently congiured mode,
                                                        else if resourse mode is PB do not set currenly requested mode
* @param[in] poolId -         Pool ID to configure
* @param[in] configureHeadroom        If GT_TRUE configure headroom subtruction field ,
                                                        else do not configure headroom subtruction field
* @param[in] headroomEnable -  headroom subtraction mode,only relevant if pool id is not equal PB
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqPfccTcResourceModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_BOOL                              forceSet,
    IN  GT_U32                               poolId,
    IN  GT_BOOL                              configureHeadroom,
    IN  GT_BOOL                              headroomEnable
)
{
    GT_U32    regAddr,tileOffset,regValue,globalTcHrIndex;
    GT_STATUS rc;
    GT_BOOL oldHeadRoom;
    PRV_CPSS_PFCC_CFG_ENTRY_STC  entry;

     /*Read current */
    rc = prvCpssFalconTxqPfccTcResourceModeGet(devNum,tileNum,trafClass,&regValue,&oldHeadRoom);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcResourceModeGet  failed \n");
    }

    if(regValue!=PRV_TC_MAP_PB_MAC)
    {
        forceSet = GT_TRUE;
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.TC_to_pool_CFG;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    if((forceSet == GT_TRUE)&&(poolId!=regValue))
    {
        regValue = 0;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,(TXQ_PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE*trafClass),
        TXQ_PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE,poolId);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssHwPpSetRegField  failed \n");
        }
    }

    if((GT_TRUE==configureHeadroom)&&(headroomEnable!=oldHeadRoom))
    {
        rc = prvCpssHwPpSetRegField(devNum,regAddr,(TXQ_PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_OFFSET+trafClass),
        TXQ_PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_SIZE,BOOL2BIT_MAC(headroomEnable));

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssHwPpSetRegField  failed \n");
        }

        /*Configure calendar*/

        if(GT_TRUE == headroomEnable)
        {
            entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR;
            entry.globalTc = trafClass;
        }
        else
        {
            entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE;
            entry.numberOfBubbles = 1;
        }

        /*After global TC*/
        globalTcHrIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC+8;
        rc =prvCpssFalconTxqPfccCfgTableEntrySet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,
            globalTcHrIndex+trafClass,&entry);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntrySet  failed \n");
        }

    }

    return GT_OK;
}
/**
* @internal prvCpssFalconTxqPfccTcResourceModeGet
* @endinternal
*
* @brief  Get configured  mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] trafClass       Traffic class [0..7]
* @param[in] poolIdPtr -         (pointer to)Pool ID to configured
* @param[in] headRoomPtr -  (pointer to)headroom subtraction mode,only relevant if pool id is not equal PB
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqPfccTcResourceModeGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               *poolIdPtr,
    IN  GT_BOOL                              *headRoomPtr
)
{
    GT_U32    regAddr,tileOffset,regData;
    GT_STATUS rc;

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.TC_to_pool_CFG;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regData);
    if(rc!=GT_OK)
    {
        return rc;
    }

    *poolIdPtr = U32_GET_FIELD_MAC(regData,(TXQ_PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE*trafClass),
        TXQ_PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE);

    *headRoomPtr= (GT_BOOL)U32_GET_FIELD_MAC(regData,(TXQ_PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_OFFSET+trafClass),
        TXQ_PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_SIZE);

    return rc;
}

