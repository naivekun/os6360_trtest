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
* @file prvCpssDxChTxqQfc.c
*
* @brief CPSS SIP6 TXQ Qfc low level configurations.
*
* @version   1
********************************************************************************
*/

/*Synced to \Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.7}*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/prvCpssFalconTxqQfcRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqQfc.h>



#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>


#define PRV_MAX_QFC_RETRY_MAC 10

/**
* @internal prvCpssFalconTxqQfcPortBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified local port.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] index -                     if perPort then local port number (APPLICABLE RANGES:0..8) else local queue number (APPLICABLE RANGES:0..399).
* @param[in] perPort -                if equal GT_TRUE then it is per port querry,else per queue
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcBufNumberGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 qfcNum,
    IN  GT_U32 index,
    IN  GT_BOOL  perPort,
    OUT  GT_U32    *numPtr
)
{


    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regValue;
    GT_U32 counter = 0;
    GT_BOOL dataValid = GT_FALSE;


    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(perPort == GT_TRUE)
    {
        TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(index);
         regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_1_Indirect_Read_Address ;
    }
    else
    {
        if(index >= CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_0_Indirect_Read_Address ;
    }



     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

     rc = prvCpssHwPpWriteRegister(devNum,regAddr,index);

     if(rc!=GT_OK)
     {
        return rc;
     }

     /*Busy wait until the value is latched*/
     while(counter <300)counter++;

     /*Now read*/

      if(perPort == GT_TRUE)
      {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_1_Indirect_Read_Data ;
      }
      else
      {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_0_Indirect_Read_Data ;
      }

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

     counter = 0;

     while((dataValid == GT_FALSE)&&(counter<PRV_MAX_QFC_RETRY_MAC))
     {
         rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

         if(rc!=GT_OK)
         {
                return rc;
         }
#ifdef ASIC_SIMULATION
    dataValid = GT_TRUE;/* Yet not implemented in simulation */
#else
    dataValid = (GT_BOOL) U32_GET_FIELD_MAC(regValue,TXQ_QFC_INDIRECT_ACCESS_DATA_VALID_FIELD_OFFSET,
                                TXQ_QFC_INDIRECT_ACCESS_DATA_VALID_FIELD_SIZE );
#endif


         if(dataValid == GT_TRUE)
         {
#ifdef GM_USED
            *numPtr =0;/* Yet not implemented in golden model */
#else
            *numPtr =  U32_GET_FIELD_MAC(regValue,TXQ_QFC_INDIRECT_ACCESS_COUNTER_DATA_FIELD_OFFSET,
                               TXQ_QFC_INDIRECT_ACCESS_COUNTER_DATA_FIELD_SIZE );
#endif
         }
         else
         {
                counter++;
         }
      }

     if(counter ==PRV_MAX_QFC_RETRY_MAC)
      {
            rc = GT_FAIL;
      }

    return rc;

}




/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortSet function
* @endinternal
*
* @brief   Map local DP port to global dma
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               -  device number
* @param[in] tileNum                -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort              -  local DP port (APPLICABLE RANGES:0..8).
* @param[in] dmaPort              -  Global DMA number(0..263).
* @param[in] fcMode             - flow control mode
* @param[in] ignoreMapping    -  Ignore mapping configuration ,configure only PFC generation
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  qfcNum,
    IN  GT_U32  localPort,
    IN  GT_U32  dmaPort,
    IN CPSS_DXCH_PORT_FC_MODE_ENT  fcMode,
    IN GT_BOOL ignoreMapping
)
{
        GT_STATUS rc = GT_OK;
        GT_U32 regValue =0;
        GT_U32 localPortFcType;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPort);

        if(dmaPort>=(1<<TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_SIZE))
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "dmaPort port is too big %d",dmaPort);
        }


        switch(fcMode)
        {
            case CPSS_DXCH_PORT_FC_MODE_802_3X_E:
                localPortFcType = 0x2;
                break;
             case CPSS_DXCH_PORT_FC_MODE_PFC_E:
                localPortFcType = 0x1;
                break;
             case  CPSS_DXCH_PORT_FC_MODE_DISABLE_E:
                localPortFcType = 0x0;
                break;
             default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid FC mode ");
                break;
        }



        if(ignoreMapping == GT_TRUE)
        {
            /*configure only port type*/

             rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        localPort,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_OFFSET,
                                       TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_SIZE,
                                        localPortFcType);
        }
        else
        {


                    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_OFFSET,
                            TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_SIZE,
                            localPortFcType);

                    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_OFFSET,
                            TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_SIZE,
                            dmaPort);

                 rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        localPort,
                                        &regValue);
        }

        return rc;

}

/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortGet function
* @endinternal
*
* @brief   Get local DP port to global dma mapping
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               -  device number
* @param[in] tileNum                -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum               -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort             -  local DP port (APPLICABLE RANGES:0..8).
* @param[out] dmaPortPtr             -  (Pointer to)Global DMA number(0..263).
* @param[out] fcModePtr             - (Pointer to) Enable PFC generation for the local port.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  qfcNum,
    IN  GT_U32  localPort,
    OUT  GT_U32  * dmaPortPtr,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT  *fcModePtr
)
{
        GT_STATUS rc;
        GT_U32 regValue,localPortFcType;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPort);



         rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        localPort,
                                        &regValue);

         if(rc!=GT_OK)
         {
                return rc;
         }

       if(fcModePtr!=NULL)
       {


            localPortFcType = U32_GET_FIELD_MAC(regValue,
                                TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_OFFSET,
                                TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE_FIELD_SIZE);

            switch(localPortFcType)
            {
                case 0x2 :
                    *fcModePtr = CPSS_DXCH_PORT_FC_MODE_802_3X_E;
                    break;
                 case 0x1:
                   *fcModePtr = CPSS_DXCH_PORT_FC_MODE_PFC_E;
                    break;
                 case 0x0:
                   *fcModePtr = CPSS_DXCH_PORT_FC_MODE_DISABLE_E;
                    break;
                 default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid FC mode ");
                    break;
            }
        }

       if(dmaPortPtr!=NULL)
       {
        *dmaPortPtr = U32_GET_FIELD_MAC(regValue,
                            TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_OFFSET,
                            TXQ_QFC_LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_SIZE);
       }


        return GT_OK;

}


/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgSet function
* @endinternal
*
* @brief   Set global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] cfgPtr                     -   (pointer to)QFC PFC global configuration

* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC * cfgPtr
 )
{
        GT_STATUS rc;
        GT_U32 regValue=0,regAddr,tileOffset;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        if(cfgPtr->pbAvailableBuffers>=1<<TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "pbAvailableBuffers  is too big %d",cfgPtr->pbAvailableBuffers);
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;



        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_SIZE,
                                cfgPtr->globalPfcEnable);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_SIZE,
                                cfgPtr->pdxCongestionPfcEnable);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_SIZE,
                                cfgPtr->pbCongestionPfcEnable);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_SIZE,
                                cfgPtr->pbAvailableBuffers);

         U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_SIZE,
                                cfgPtr->ingressAgregatorTcPfcBitmap);

       U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_HR_COUNTING_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_HR_COUNTING_ENABLE_FIELD_SIZE,
                                cfgPtr->hrCountingEnable);

       rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);

       return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGet function
* @endinternal
*
* @brief   Get global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] cfgPtr                     -   (pointer to)QFC PFC global configuration

* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  *cfgPtr
 )
{
        GT_STATUS rc;
        GT_U32 regValue,regAddr,tileOffset;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;


        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

         if(rc!=GT_OK)
         {
                return rc;
         }

        cfgPtr->globalPfcEnable = U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_SIZE );


        cfgPtr->pdxCongestionPfcEnable = U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_SIZE);


        cfgPtr->pbCongestionPfcEnable = U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_SIZE);


        cfgPtr->pbAvailableBuffers = U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_SIZE );

        cfgPtr->ingressAgregatorTcPfcBitmap= U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_SIZE );

        cfgPtr->hrCountingEnable= U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_PFC_CONF_HR_COUNTING_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PFC_CONF_HR_COUNTING_ENABLE_FIELD_SIZE );

         return rc;
}


/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitSet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimit                     -   packet buffer limit

* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  GT_U32  globalPbLimit
 )
{
        GT_STATUS rc;
        GT_U32 regValue = 0,regAddr,tileOffset;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        if(globalPbLimit>=1<<TXQ_QFC_GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "globalPbLimit  is too big %d",globalPbLimit);
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].global_pb_limit;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;



        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_SIZE,
                                globalPbLimit);


       rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);

         return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitGet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimitPtr                     -   (pointer to)packet buffer limit

* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     OUT  GT_U32  * globalPbLimitPtr
 )
{
        GT_STATUS rc;
        GT_U32 regValue,regAddr,tileOffset;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);


        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].global_pb_limit;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;


        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

        if(rc == GT_OK)
        {
            *globalPbLimitPtr = U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_OFFSET,
                                TXQ_QFC_GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_SIZE);
        }

         return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdSet function
* @endinternal
*
* @brief   Set global TC threshold .
*    Note - Duplication to all units is done internally in prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   device number
* @param[in] tc                  -          traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable               -    Global TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdSet
(
  IN  GT_U8                           devNum,
  IN  GT_U32                          tc,
  IN  GT_BOOL                         enable,
  IN  CPSS_DXCH_PFC_THRESHOLD_STC     *thresholdCfgPtr,
  IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    if(enable == GT_TRUE)
    {
      if(thresholdCfgPtr->alfa>=1<<TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_SIZE)
      {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "alfa  is too big %d",thresholdCfgPtr->alfa);
      }

      if(thresholdCfgPtr->guaranteedThreshold>=1<<TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_SIZE)
      {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "guaranteedThreshold  is too big %d",thresholdCfgPtr->alfa);
      }

      if(hysteresisCfgPtr->xonOffsetValue>=1<<TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_SIZE)
      {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "xonOffsetValue  is too big %d",hysteresisCfgPtr->xonOffsetValue);
      }
    }


    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_OFFSET,
                        TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_SIZE,
                        thresholdCfgPtr->guaranteedThreshold);

     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_OFFSET,
                        TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_SIZE,
                        thresholdCfgPtr->alfa);

     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE_FIELD_OFFSET,
                        TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE_FIELD_SIZE,
                        enable?1:0);

     rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_E,
                                    tc,
                                    &regValue);

    if(rc!=GT_OK)
    {
        return rc;
    }

    /*No histeresys*/
    if (prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
      return GT_OK;
    }

    regValue =0;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_OFFSET,
                        TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_SIZE,
                        hysteresisCfgPtr->xonOffsetValue);

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_MODE_FIELD_OFFSET,
                        TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_MODE_FIELD_SIZE,
                        (hysteresisCfgPtr->xonMode==CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E)?0:1);

    rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_QFC_TC_PFC_HYSTERESIS_CONF_E,
                                    tc,
                                    &regValue);

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdGet function
* @endinternal
*
* @brief   Get global TC threshold .Note - The threshold is read from QFC0(all other DPs should contain the same value)
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tc                  -   traffic class(APPLICABLE RANGES:0..7).
* @param[out] enablePtr               - (pointer to )Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdGet
(
     IN  GT_U8                           devNum,
     IN  GT_U32                          tc,
     OUT GT_BOOL                        *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
       GT_U32 regValue,xonMode;
       GT_STATUS rc;

       rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_E,
                                   tc,
                                   &regValue);

       if(rc == GT_OK)
       {

        thresholdCfgPtr->guaranteedThreshold = U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_OFFSET,
                           TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_SIZE);

        thresholdCfgPtr->alfa = U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_OFFSET,
                           TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_SIZE );

        *enablePtr =  (GT_BOOL)U32_GET_FIELD_MAC(regValue,TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE_FIELD_OFFSET,
                           TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE_FIELD_SIZE);
       }

        if(hysteresisCfgPtr==NULL||rc!=GT_OK||(prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E))
        {
            return rc;
        }


        rc = prvCpssDxChReadTableEntry(devNum,
                                            CPSS_DXCH_SIP6_TXQ_QFC_TC_PFC_HYSTERESIS_CONF_E,
                                            tc,
                                            &regValue);

        if(rc == GT_OK)
        {

            hysteresisCfgPtr->xonOffsetValue  = U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_OFFSET,
                                TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_SIZE);


            xonMode =  U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_OFFSET,
                                TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_SIZE);

            if(xonMode==0)
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E;
            }
            else
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_FIXED_E;
            }
        }

        return rc;


}

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port/TC threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[in] tc                  -   traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable               - Port/TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdSet
(
     IN  GT_U8                       devNum,
     IN  GT_U32                      tileNum,
     IN  GT_U32                      qfcNum,
     IN  GT_U32                      localPort,
     IN  GT_U32                      tc,
     IN  GT_BOOL                     enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC  *thresholdCfgPtr
)
{
        GT_STATUS rc;
        GT_U32 regValue = 0;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        if(thresholdCfgPtr->alfa>=1<<TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "alfa  is too big %d",thresholdCfgPtr->alfa);
        }

        if(thresholdCfgPtr->guaranteedThreshold>=1<<TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "guaranteedThreshold  is too big %d",thresholdCfgPtr->alfa);
        }


        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_OFFSET,
                            TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_SIZE,
                            thresholdCfgPtr->guaranteedThreshold);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_OFFSET,
                            TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_SIZE,
                            thresholdCfgPtr->alfa);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE_FIELD_OFFSET,
                            TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE_FIELD_SIZE,
                            enable?1:0);

        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_QFC_PORT_TC_PFC_THRESHOLD_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        localPort*8+tc,
                                        &regValue);

        return rc;
}

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port/TC threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[in] tc                  -   traffic class(APPLICABLE RANGES:0..7).
* @param[out] enablePtr               - (pointer to )Port TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdGet
(
     IN  GT_U8                          devNum,
     IN  GT_U32                         tileNum,
     IN  GT_U32                         qfcNum,
     IN  GT_U32                         localPort,
     IN  GT_U32                         tc,
     IN  GT_BOOL                        *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr
)
{
        GT_STATUS rc;
        GT_U32 regValue;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        rc = prvCpssDxChReadTableEntry(devNum,
                                            CPSS_DXCH_SIP6_TXQ_QFC_PORT_TC_PFC_THRESHOLD_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                            localPort*8+tc,
                                            &regValue);

        if(rc == GT_OK)
        {

            thresholdCfgPtr->guaranteedThreshold = U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_OFFSET,
                                TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_SIZE);

            thresholdCfgPtr->alfa = U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_OFFSET,
                                TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_SIZE  );


            *enablePtr = (GT_BOOL)U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE_FIELD_SIZE);
        }

        return rc;
}

/**
* @internal prvCpssFalconTxqQfcPortPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[in] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             localPort,
     IN  GT_BOOL                            enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
        GT_STATUS rc;
        GT_U32 regValue = 0;

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        if(thresholdCfgPtr->alfa>=1<<TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "alfa  is too big %d",thresholdCfgPtr->alfa);
        }

        if(thresholdCfgPtr->guaranteedThreshold>=1<<TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "guaranteedThreshold  is too big %d",thresholdCfgPtr->alfa);
        }

        if(hysteresisCfgPtr->xonOffsetValue>=1<<TXQ_QFC_TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_SIZE)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "xonOffsetValue  is too big %d",hysteresisCfgPtr->xonOffsetValue);
        }


        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_OFFSET,
                            TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_SIZE,
                            thresholdCfgPtr->guaranteedThreshold);
        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_OFFSET,
                            TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_SIZE,
                            thresholdCfgPtr->alfa);
        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ENABLE_FIELD_OFFSET,
                            TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ENABLE_FIELD_SIZE,
                            enable?1:0);

        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_QFC_PORT_PFC_THRESHOLD_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        localPort,
                                        &regValue);
        if(rc!=GT_OK)
        {
            return rc;
        }

        /*No histeresys*/
        if (prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
        {
          return GT_OK;
        }
        regValue =0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_OFFSET,
                            TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_SIZE,
                            hysteresisCfgPtr->xonOffsetValue);
        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_OFFSET,
                            TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_SIZE,
                            (hysteresisCfgPtr->xonMode==CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E)?0:1);

        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        localPort,
                                        &regValue);
        return rc;
}
/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[out] enablePtr               - (pointer to )Port PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdGet
(
     IN  GT_U8                         devNum,
     IN  GT_U32                        tileNum,
     IN  GT_U32                        qfcNum,
     IN  GT_U32                        localPort,
     OUT GT_BOOL                       *enablePtr,
     OUT  CPSS_DXCH_PFC_THRESHOLD_STC  *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
        GT_STATUS rc;
        GT_U32 regValue,xonMode;

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);


        rc = prvCpssDxChReadTableEntry(devNum,
                                            CPSS_DXCH_SIP6_TXQ_QFC_PORT_PFC_THRESHOLD_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                            localPort,
                                            &regValue);

        if(rc == GT_OK)
        {

            thresholdCfgPtr->guaranteedThreshold = U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_OFFSET,
                                TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_SIZE);


            thresholdCfgPtr->alfa = U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_OFFSET,
                                TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_SIZE  );


            *enablePtr = (GT_BOOL)U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ENABLE_FIELD_OFFSET,
                                TXQ_QFC_PORT_PFC_THRESHOLD_PORT_PFC_ENABLE_FIELD_SIZE);
        }

        if(hysteresisCfgPtr==NULL||rc!=GT_OK||(prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E))
        {
            return rc;
        }


        rc = prvCpssDxChReadTableEntry(devNum,
                                            CPSS_DXCH_SIP6_TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                            localPort,
                                            &regValue);

        if(rc == GT_OK)
        {

            hysteresisCfgPtr->xonOffsetValue  = U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_OFFSET,
                                TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE_FIELD_SIZE);


            xonMode =  U32_GET_FIELD_MAC(regValue,TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_OFFSET,
                                TXQ_QFC_PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE_FIELD_SIZE);

            if(xonMode==0)
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E;
            }
            else
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_FIXED_E;
            }
        }
        return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet function
* @endinternal
*
* @brief   Set PBB PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit, PFC OFF message will be sent to all port.tc
*
*
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  pbCongestionEnable
 )
{
        GT_STATUS rc;
        GT_U32 regAddr,tileOffset;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);


        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;


       rc = prvCpssHwPpSetRegField(devNum,regAddr,TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_OFFSET,
        TXQ_QFC_GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_SIZE,pbCongestionEnable?1:0);

         return rc;
}


/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet function
* @endinternal
*
* @brief   Set global PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit, PFC OFF message will be sent to all port.tc
*
*
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  enable
 )
{
        GT_STATUS rc;
        GT_U32 regAddr,tileOffset;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);


        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;


       rc = prvCpssHwPpSetRegField(devNum,regAddr,TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_OFFSET,
        TXQ_QFC_GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_SIZE,enable?1:0);

         return rc;
}

/**
* @internal prvCpssFalconTxqDumpQfcLocalPfcCounter function
* @endinternal
*
* @brief   Print  the current number of buffers  per source port/tc and Global TC
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] rxDma                         rx dma -
* @param[in] tc -                               traffic class
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqDumpQfcLocalPfcCounter
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   qfcNum,
    IN  GT_U32   rxDma,
    IN  GT_U32   tc
)
{
    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regValue,pfcCount = 0;
    GT_U32 counter = 0;
    GT_BOOL dataValid = GT_FALSE;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if((rxDma >= PRV_CPSS_MAX_DMA_NUM_CNS) )
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Fail in prvCpssFalconTxqDumpQfcLocalPfcCounter");
    }

     regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_2_Indirect_Read_Address ;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

     rc = prvCpssHwPpWriteRegister(devNum,regAddr,rxDma*8+tc);

     if(rc!=GT_OK)
     {
        return rc;
     }

     /*Busy wait until the value is latched*/
     while(counter <300)counter++;

     /*Now read*/

     regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_2_Indirect_Read_Data ;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

     counter = 0;

     while((dataValid == GT_FALSE)&&(counter<PRV_MAX_QFC_RETRY_MAC))
     {
         rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

         if(rc!=GT_OK)
         {
                return rc;
         }

#ifndef ASIC_SIMULATION  /* Yet not implemented in simulation */
         dataValid = (GT_BOOL) U32_GET_FIELD_MAC(regValue,TXQ_QFC_INDIRECT_ACCESS_DATA_VALID_FIELD_OFFSET,
                                TXQ_QFC_INDIRECT_ACCESS_DATA_VALID_FIELD_SIZE );
#else
         dataValid = GT_TRUE;
#endif

         if(dataValid == GT_TRUE)
         {
                pfcCount =  U32_GET_FIELD_MAC(regValue,TXQ_QFC_INDIRECT_ACCESS_COUNTER_DATA_FIELD_OFFSET,
                                TXQ_QFC_INDIRECT_ACCESS_COUNTER_DATA_FIELD_SIZE );
         }
         else
         {
                counter++;
         }
      }

     if(counter ==PRV_MAX_QFC_RETRY_MAC)
      {
            rc = GT_FAIL;
      }

     if(rc == GT_OK)
     {
        cpssOsPrintf("\n qfcNum          = %d",qfcNum);
        cpssOsPrintf("\n dma             = %d",rxDma);
        cpssOsPrintf("\n tc              = %d",tc);
        cpssOsPrintf("\n pfc counter     = %d\n",pfcCount);
     }

     /*Now get global TC*/

       rc = prvCpssDxChReadTableEntryField(devNum,
                                         CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_COUNTERS_E+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                         tc,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                         TXQ_QFC_GLOBAL_COUNTERS_TC_COUNTER_VAL_FIELD_OFFSET,
                                         TXQ_QFC_GLOBAL_COUNTERS_TC_COUNTER_VAL_FIELD_SIZE,
                                         &pfcCount);


     if(rc == GT_OK)
     {
        cpssOsPrintf("\n Global TC count  = %d\n",pfcCount);
     }

    return rc;

}

/**
* @internal prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
* @endinternal
*
* @brief   Set amount of   buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[in] availableBuffers - amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8   tc,
     IN  GT_U32  availeblePoolBuffers
 )
{
      GT_U32  regValue=0,fieldSize,fieldOffset;
      CPSS_DXCH_TABLE_ENT tableType;

      TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

      TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

      if(confMode==CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E)
      {
        fieldOffset = TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_OFFSET;
        fieldSize = TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_SIZE;
        tableType = CPSS_DXCH_SIP6_TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_E;
      }
      else
      {
        fieldOffset =TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_OFFSET;
        fieldSize = TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_SIZE;
        tableType = CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_E;
      }

      if(availeblePoolBuffers>=(GT_U32)(1<<fieldSize))
      {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "availeblePoolBuffers  is too big %d",availeblePoolBuffers);
      }

      U32_SET_FIELD_MASKED_MAC(regValue,fieldOffset,fieldSize,availeblePoolBuffers);

      return  prvCpssDxChWriteTableEntry(devNum,
                                        tableType+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        tc,
                                        &regValue);

}


/**
* @internal internal_cpssDxChPortPfcDbaTcAvailableBuffersGet
* @endinternal
*
* @brief   Get amount of   buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[out] availableBuffersPtr - (pointer to)amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/

GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8   tc,
     IN  GT_U32  *availeblePoolBuffersPtr
 )
{
      GT_U32  fieldSize,fieldOffset;
      CPSS_DXCH_TABLE_ENT tableType;

      TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

      TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

      if(confMode==CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E)
      {
        fieldOffset = TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_OFFSET;
        fieldSize = TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_SIZE;
        tableType = CPSS_DXCH_SIP6_TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_E;
      }
      else
      {
        fieldOffset =TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_OFFSET;
        fieldSize = TXQ_QFC_PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_SIZE;
        tableType = CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_TC_AVAILABLE_BUFFERS_CONF_E;
      }

      return  prvCpssDxChReadTableEntryField(devNum,
                                        tableType+qfcNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC,
                                        tc,PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        fieldOffset,fieldSize,
                                        availeblePoolBuffersPtr);

}



