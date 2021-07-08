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
* @file prvCpssDxChTxqPdx.c
*
* @brief CPSS SIP6 TXQ PDX low level configurations.
*
* @version   1
********************************************************************************
*/
/*#define CPSS_LOG_IN_MODULE_ENABLE - do not log currently*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/prvCpssFalconTxqPdxRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPdx.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>


static GT_U32 prvCpssDxChTxqPdxInterfaceMapGetOffset(GT_U32 pdxNum);

/*macro that get offset of specific PDX2PDX_INTERFACE*/
#define PDX2PDX_INTERFACE_OFFSET_MAC(_X)    prvCpssDxChTxqPdxInterfaceMapGetOffset(_X)
/*macro that get size of specific PDX2PDX_INTERFACE*/
#define PDX2PDX_INTERFACE_FIELD_SIZE_MAC(_X)    prvCpssDxChTxqPdxInterfaceMapGetFieldSize(_X)

/*macro that get offset of specific register field at TXQ/PDX*/
#define GET_PDX_REGISTER_FIELD_OFFSET_MAC(REG,FIELD) GET_REGISTER_FIELD_OFFSET_MAC(TXQ,PDX,REG,FIELD)
/*macro that get size of specific register field at TXQ/PDX*/
#define GET_PDX_REGISTER_FIELD_SIZE_MAC(REG,FIELD) GET_REGISTER_FIELD_SIZE_MAC(TXQ,PDX,REG,FIELD)



#define TXQ_PDX_CPU_SLICE_INDEX (TXQ_PDX_MAX_SLICE_NUMBER_MAC-1)






/**
* @internal prvCpssDxChTxqPdxInterfaceMapGetOffset function
* @endinternal
*
* @brief   Get offset of PDX2PDX_INTERFACE_MAPPING for specific interface
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @retval offset                   - the of PDX2PDX_INTERFACE_MAPPING for specific interface
*/


static GT_U32 prvCpssDxChTxqPdxInterfaceMapGetOffset(GT_U32 interfaceId)
{
    GT_U32 offset;

    switch(interfaceId)
    {
        case 0:
          offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_0);
          break;
        case 1:
          offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_1);
          break;
       case 2:
          offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_2);
          break;
        case 3:
        default:/*avoid warnings*/
          offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_3);
          break;

    }

    return offset;
}

/**
*
* @internal prvCpssDxChTxqPdxInterfaceMapGetFieldSize function
* @endinternal
*
* @brief   Get size of PDX2PDX_INTERFACE_MAPPING for specific interface
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @retval size                     - of PDX2PDX_INTERFACE_MAPPING field
*
* @note Currently only one tile is supported
*
*/

static GT_U32 prvCpssDxChTxqPdxInterfaceMapGetFieldSize(GT_U32 interfaceId)
{
    GT_U32 size;

    switch(interfaceId)
    {
        case 0:
          size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_0);
          break;
        case 1:
          size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_1);
          break;
       case 2:
          size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_2);
          break;
        case 3:
        default:/*avoid warning*/
          size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING,PDX_INTERFACE_ID_3);
          break;

    }

    return size;
}


/**
* @internal prvCpssDxChTxqFalconPdxInterfaceMapSet function
* @endinternal
*
* @brief   Set current PDX to another PDX/s interface mapping
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] pdxNum                   -current PDX id(APPLICABLE RANGES:0..3).
* @param[in] connectedPdxId           -  the mapping of connected PDXs
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

 GT_STATUS prvCpssDxChTxqFalconPdxInterfaceMapSet
 (
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 * connectedPdxId
 )
 {   GT_U32 regAddr ;
     GT_U32 regValue = 0;
     GT_U32 i;

    GT_STATUS rc;
    GT_U32 tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);


     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.pdx2pdx_interface_mapping;
     tileOffset = prvCpssSip6TileOffsetGet(devNum,pdxNum/*tileId*/);
     regAddr+=tileOffset;

     for(i=0;i<4;i++)
     {
        U32_SET_FIELD_MASKED_MAC(regValue,PDX2PDX_INTERFACE_OFFSET_MAC(i),
                                 PDX2PDX_INTERFACE_FIELD_SIZE_MAC(i),
                                 connectedPdxId[i]);
     }

     rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);

     return rc;
 }






/*******************************************************************************
* prvCpssDxChTxqFalconPdxQueueGroupMapSet
*
* DESCRIPTION:
*
*   Set entry at PDX DX_QGRPMAP table
*
*
* APPLICABLE DEVICES:
*        Falcon
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*
*       devNum     -PP's device number.
*       pdxNum      - tile number.(APPLICABLE RANGES:0..3)
*       queueGroupIndex - queue group number , entry index
*       txPortMap   -   entry to be written to HW
*
*
*
*
* OUTPUTS:
*       NONE
*
* RETURNS:
*        GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong pdx number.
*       GT_HW_ERROR              - on writing to HW error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/

GT_STATUS prvCpssDxChTxqFalconPdxQueueGroupMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroupIndex,
    IN PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC txPortMap
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;


    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(txPortMap.dpCoreLocalTrgPort);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(txPortMap.queuePdsIndex);
    TXQ_SIP_6_CHECK_PDX_NUM_MAC(txPortMap.queuePdxIndex);

    TXQ_SIP_6_CHECK_QUEUE_GROUP_NUM_MAC(queueGroupIndex);

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_BASE_FIELD_OFFSET,
                            TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_BASE_FIELD_SIZE,
                            txPortMap.queueBase);
    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_OFFSET,
                            TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_SIZE,
                            txPortMap.queuePdxIndex);
    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_OFFSET,
                            TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_SIZE,
                            txPortMap.queuePdsIndex);
    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDX_ITX_QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET,
                            TXQ_PDX_ITX_QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE,
                            txPortMap.dpCoreLocalTrgPort);


    rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,
                                    queueGroupIndex,
                                    &regValue);

    return rc;

}

/**
* @internal prvCpssDxChTxqFalconPdxQueueGroupMapGet function
* @endinternal
*
* @brief   Read entry from PDX DX_QGRPMAP table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] queueGroupIndex          - queue group number , entry index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxqFalconPdxQueueGroupMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroupIndex,
    OUT PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC * txPortMapPtr
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;



    TXQ_SIP_6_CHECK_QUEUE_GROUP_NUM_MAC(queueGroupIndex);

    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,
                                        queueGroupIndex,
                                        &regValue);


     if(rc!=GT_OK)
     {
        return rc;
     }

     txPortMapPtr->queueBase= U32_GET_FIELD_MAC(regValue,
        TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_BASE_FIELD_OFFSET,
        TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_BASE_FIELD_SIZE);

     txPortMapPtr->queuePdxIndex= U32_GET_FIELD_MAC(regValue,
        TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_OFFSET,
        TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_SIZE);

     txPortMapPtr->queuePdsIndex= U32_GET_FIELD_MAC(regValue,
        TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_OFFSET,
        TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_SIZE);

     txPortMapPtr->dpCoreLocalTrgPort= U32_GET_FIELD_MAC(regValue,
        TXQ_PDX_ITX_QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET,
        TXQ_PDX_ITX_QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE);

     return GT_OK;

}


/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet function
* @endinternal
*
* @brief   Read arbitration PIZZA on PDS BurstFifo at PDX
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] pdxNum                   -  current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum                   -  PDS burst fifo number (APPLICABLE RANGES:0..7).
*
* @param[out] activeSliceNumPtr -  the   number of active slices
* @param[out] activeSliceMapArr  - PIZZA configuration
* @param[out] sliceValidArr  - array that mark used slices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    OUT GT_U32* activeSliceNumPtr,
    OUT GT_U32* activeSliceMapArr,
    OUT GT_BOOL * sliceValidArr
)
{

     GT_STATUS rc = GT_OK;
     CPSS_DXCH_TABLE_ENT pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PIZZA_ARBITER_CONTROL_REGISTER_PDS_E;
     GT_U32 regValue = 0,i;



     TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
     TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

     /*read activeSliceNum*/

       if(pdsNum>3)
       {
            pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PIZZA_ARBITER_CONTROL_REGISTER_PDS_E;
            pdsNum%=4;
       }

      pdsTable+= pdxNum;

      rc = prvCpssDxChReadTableEntryField(devNum,
                                        pdsTable,pdsNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_OFFSET,
                                        TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_SIZE,
                                        activeSliceNumPtr);
     if(rc!=GT_OK)
     {
        return rc;
     }



     if(pdsTable <CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PIZZA_ARBITER_CONTROL_REGISTER_PDS_E )
     {
         pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_E;
     }
     else
     {
         pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_E;;
     }


    (*activeSliceNumPtr)++;

     pdsTable+=(pdsNum*4);
     pdsTable+= pdxNum;

     for(i = 0;i<*activeSliceNumPtr;i++)
     {
         rc = prvCpssDxChReadTableEntry(devNum,
                                        pdsTable,
                                        i,
                                        &regValue);
         if(rc !=GT_OK)
         {
            return rc;
         }

         activeSliceMapArr[i] = U32_GET_FIELD_MAC(regValue,
            TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_OFFSET,
            TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_SIZE);

         sliceValidArr[i] = U32_GET_FIELD_MAC(regValue,
            TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_OFFSET,
            TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_SIZE);
     }



     return GT_OK;


}

/**
* @internal prvCpssDxChTxqFalconPdxInterfaceMapSet function
* @endinternal
*
* @brief   Triger loading of a new pizza into arbiter
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] pdxNum                   -  current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum                   -  PDS burst fifo number (APPLICABLE RANGES:0..7).
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum
)
{

        GT_STATUS rc = GT_OK;
        CPSS_DXCH_TABLE_ENT pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PIZZA_ARBITER_CONTROL_REGISTER_PDS_E;


        TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
        TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


       if(pdsNum>3)
       {
            pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PIZZA_ARBITER_CONTROL_REGISTER_PDS_E;
            pdsNum%=4;
       }

      pdsTable+= pdxNum;


          rc = prvCpssDxChWriteTableEntryField(devNum,pdsTable,pdsNum,PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_OFFSET,
                                TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_SIZE,
                              1);

         if(rc!=GT_OK)
         {
            return rc;
         }

          rc = prvCpssDxChWriteTableEntryField(devNum,pdsTable,pdsNum,PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_OFFSET,
                                TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_SIZE,
                                0);


      return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice function
* @endinternal
*
* @brief   Set maximal numer of slices per DP at PDX arbiter
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] pdxNum                   -  current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum -  DP number(APPLICABLE RANGES:0..7).
* @param[in] maxSlice - the number of slices used(APPLICABLE RANGES:8..33).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*
*/

GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    IN GT_U32  maxSlice
)
{

        GT_STATUS rc = GT_OK;
        CPSS_DXCH_TABLE_ENT pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PIZZA_ARBITER_CONTROL_REGISTER_PDS_E;


        TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
        TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

        if(maxSlice>TXQ_PDX_MAX_SLICE_NUMBER_MAC)
         {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected maxSlice - %d ",maxSlice);
         }


       if(pdsNum>3)
       {
            pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PIZZA_ARBITER_CONTROL_REGISTER_PDS_E;
            pdsNum%=4;
       }

      pdsTable+= pdxNum;



          rc = prvCpssDxChWriteTableEntryField(devNum,pdsTable,pdsNum,PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_OFFSET,
                                TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_SIZE,
                                maxSlice-1);



      return rc;
}

/**
* @internal prvCpssDxChTxQFalconPizzaConfigurationSet function
* @endinternal
 *
* @brief   Update pizza arbiter
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -PP's device number.
*@param[in] pdxNum                   -tile number
*@param[in] pdsNum                   -pdsNum number
*@param[in] localChannelIndex -the slice owner[0-7]
*@param[in] portPizzaSlicesBitMap -the slice owner[0-7]
*@param[in] enable -equal GT_TRUE in case this is owned ,GT_FALSE otherwise
*@param[in] isCpu -equal GT_TRUE in case this is slice for CPU ,GT_FALSE otherwise
*/

GT_STATUS prvCpssDxChTxQFalconPizzaConfigurationSet
(
    IN  GT_U8                   devNum,
    IN GT_U32                   pdxNum,
    IN GT_U32                   pdsNum,
    IN  GT_U32                  localChannelIndex,
    IN  GT_U32                  portPizzaSlicesBitMap,
    IN  GT_BOOL                 enable,
    IN  GT_BOOL                 isCpu
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_TABLE_ENT pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_E;
    GT_U32 i =0;
    GT_U32 regValue = 0;
    GT_U32 originalPdsNum = pdsNum;


    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if((enable == GT_TRUE) && (isCpu==GT_FALSE))
     {
        TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localChannelIndex);
      }


    if(pdsNum>3)
    {
        pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_E;
        pdsNum%=4;
    }

    /*assuption that table enum definition is continues for PDS 0 till 7*/
    pdsTable+=(pdsNum*4);
    pdsTable+= pdxNum;

    if(isCpu == GT_TRUE)
    {
            i = TXQ_PDX_CPU_SLICE_INDEX;
    }



            while(portPizzaSlicesBitMap||(isCpu == GT_TRUE))
             {
                    if((isCpu==GT_TRUE)||(portPizzaSlicesBitMap&1))
                    {
                             U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_OFFSET,
                                         TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_SIZE,
                                         (enable==GT_TRUE)?1:0);

                               U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_OFFSET,
                                         TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_SIZE,
                                         localChannelIndex);

                                    rc = prvCpssDxChWriteTableEntry(devNum,
                                            pdsTable,
                                            i,
                                            &regValue);
                    }



                    i++;
                    portPizzaSlicesBitMap>>=1;
                    isCpu = GT_FALSE;
             }


    /*Load*/

     rc = prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza(devNum, pdxNum,originalPdsNum);

    return rc;
}
/**
* @internal prvCpssDxChTxQFalconPizzaArbiterInitPerDp function
* @endinternal
*
* @brief   Initialize the number of silices to each PDS pizza arbiter,also initialize all the slices to unused
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] pdxNum                   -tile number
* @param[in] pdsNum                   -pdsNum number
* @param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerDp
(
    IN  GT_U8                       devNum,
    IN GT_U32                      pdxNum,
    IN GT_U32                      pdsNum,
    IN GT_U32                      pdxPizzaNumberOfSlices
)
{
     GT_U32 i;
     GT_STATUS rc = GT_OK;
     CPSS_DXCH_TABLE_ENT pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_E;
     GT_U32         originalPdsNum = pdsNum;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
     TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

        if(pdsNum>3)
        {
            pdsTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_E;
            pdsNum%=4;
        }

    /*assuption that table enum definition is continues for PDS 0 till 7*/
     pdsTable+=(pdsNum*4);
     pdsTable+= pdxNum;


        /*invalidate all the slots*/
            for(i=0;i<TXQ_PDX_MAX_SLICE_NUMBER_MAC;i++)
            {

                 rc = prvCpssDxChWriteTableEntryField(devNum,pdsTable,i,PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                        TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_OFFSET,
                        TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_SIZE,
                        0);

                if(rc!=GT_OK)
                 {
                        return rc;
                 }

            }


   /*set slice numbers*/
    rc = prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice(devNum, pdxNum,originalPdsNum,pdxPizzaNumberOfSlices);

     if(rc!=GT_OK)
     {
            return rc;
     }

   /*load*/

   rc = prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza(devNum, pdxNum,originalPdsNum);


   return rc;


}

/**
* @internal prvCpssDxChTxqFalconPdxPacPortDescCounterGet function
* @endinternal
*
* @brief   Get descriptor counter per port at PDX PAC unit
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] pdxNum                   -tile number
* @param[in] portNum                   -global port num (0..71)
* @param[out ] descNumPtr          -number of descriptors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS prvCpssDxChTxqFalconPdxPacPortDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 portNum,
    OUT GT_U32* descNumPtr
)
{

     GT_STATUS rc = GT_OK;
     CPSS_DXCH_TABLE_ENT pdxTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PORT_DESC_COUNTER_E;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
     TXQ_SIP_6_CHECK_GLOBAL_PORT_NUM_MAC(portNum);

       if(portNum>=36)
       {
            pdxTable = CPSS_DXCH_SIP6_TXQ_PDX_PAC_1_PORT_DESC_COUNTER_E;
            portNum%=36;
       }

      pdxTable+= pdxNum;

      rc = prvCpssDxChReadTableEntryField(devNum,
                                        pdxTable,portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_PDX_PAC_PORT_DESC_COUNTER_PORT_DESC_COUNTER_FIELD_OFFSET,
                                        TXQ_PDX_PAC_PORT_DESC_COUNTER_PORT_DESC_COUNTER_FIELD_SIZE,
                                        descNumPtr);

     return rc;


}



