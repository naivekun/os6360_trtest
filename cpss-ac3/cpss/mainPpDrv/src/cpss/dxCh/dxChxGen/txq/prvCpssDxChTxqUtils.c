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
* @file prvCpssDxChTxqUtils.c
*
* @brief CPSS SIP6 TXQ high level configurations.
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>


#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPdx.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPsi.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPds.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPfcc.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqFcGopUtils.h>

/*
     Mapping mode that reserve queue gaps at DPs 4-7 .
     Alligned to port mode excel ,with each DP has defined range of queues
*/
#define PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC 0

/*Mapping mode that do not reserve queue gaps. All queues are consequitive , no limitation.*/
#define PRV_CPSS_QUEUE_MAPPING_MODE_CONSEQUTIVE_MAC 1

#define PRV_CPSS_QUEUE_MAPPING_MODE_DEFAULT_MAC PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC


#define DEFAULT_ELIG_FUNC CPSS_PDQ_SCHED_ELIG_N_PRIO1_E /*CPSS_PDQ_SCHED_ELIG_N_DEQ_DIS_E*/


#define PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_LENGTH_ADJUST_BYTES_MAC 20




#define QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC 8

#define QUEUES_PER_REMOTE_PORT_MAX_Q_NUM_VAL_MAC 16


#define PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC 48


#define PRV_CPSS_DXCH_FALCON_TXQ_MAX_QUANTA_MAC 0xFFFF

#define PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_THRESHOLD_MAC 0xF000



#define DEFAULT_SCHED_PROFILE 0

#define PRV_TXQ_PDS_10G_PROFILE_MAC 1
#define PRV_TXQ_PDS_25G_PROFILE_MAC 2
#define PRV_TXQ_PDS_50G_PROFILE_MAC 0
#define PRV_TXQ_PDS_100G_PROFILE_MAC 3
#define PRV_TXQ_PDS_200G_PROFILE_MAC 4
#define PRV_TXQ_PDS_400G_PROFILE_MAC 5
#define PRV_TXQ_PDS_DEF_PROFILE_MAC PRV_TXQ_PDS_10G_PROFILE_MAC


#define PRV_CPSS_DXCH_TXQ_SIP_6_CREDIT _4K


/*Tail Drop*/

#define PRV_CPSS_TD_GLOBAL_LIMIT_FACTOR_MAC (0.97)

#define PRV_CPSS_TD_PB_SIZE_PER_TILE_MAC (98304)/*Single tile = 12MB/128 = 96K cells*/

/*End of Tail Drop*/



/*Flow control*/

#define PRV_CPSS_FC_PB_SIZE_PER_TILE_MAC PRV_CPSS_TD_PB_SIZE_PER_TILE_MAC


#define PRV_CPSS_TD_DEFAULT_RESOURCE_MODE_MAC CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E

/*End of Flow control*/

static const  GT_U32 falconPdxInterfaceMap[4][4] =
{
        {0,1,2,3},
        {1,0,3,2},
        {2,3,0,1},
        {3,2,1,0}
};

static GT_STATUS prvCpssFalconTxqUtilsGetCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 remotePortNum,
    IN GT_BOOL remote,
    OUT GT_U32 *casCadePortNumPtr,
    OUT GT_U32 * pNodeIndPtr
);


/**
* @internal prvCpssFalconTxqUtilsSetDmaToPnodeMapping function
* @endinternal
 *
* @brief    Write entry to DMA to Pnode data base
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -  device number
* @param[in] dmaNum                  - Global DMA number(0..263).
* @param[in] tileNum                  - traffic class queue on this device (0..7).
* @param[in] pNodeNum                  - Index of P node

*/


static GT_STATUS prvCpssFalconTxqUtilsSetDmaToPnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  dmaNum,
     IN  GT_U32   tileNum,
     IN  GT_U32   pNodeNum
) ;

/**
* @internal prvCpssFalconTxqUtilsGetDmaToPnodeMapping function
* @endinternal
 *
* @brief    Read entry from DMA to Pnode data base
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -  device number
* @param[in] dmaNum                  - Global DMA number(0..263).
* @param[in] tileNum                  - traffic class queue on this device (0..7).
* @param[out] pNodeNum                  - Index of P node
* @param[in] isErrorForLog            - indication the CPSS ERROR should be in the LOG or not

*/

static GT_STATUS prvCpssFalconTxqUtilsGetDmaToPnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  dmaNum,
     OUT  GT_U32   *tileNumPtr,
     OUT GT_U32   * pNodeNumPtr,
    IN GT_BOOL isErrorForLog
) ;
/*******************************************************************************
* prvCpssFalconTxqUtilsSetDmaToPnodeMapping
*
* DESCRIPTION:
*      Write entry to physical port to Anode index data base
*
* APPLICABLE DEVICES:
*        Falcon
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       devNum        -  device number
*       dmaNum       - Global DMA number(0..263).
*       tileNum         - traffic class queue on this device (0..7).
*       aNodeNum   - Index of A node
*
* OUTPUTS:
*       None
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
static GT_STATUS prvCpssFalconTxqUtilsSetPhysicalNodeToAnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  physicalPortNumber,
     IN  GT_U32   tileNum,
     IN  GT_U32   aNodeNum
);
/*******************************************************************************
* prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping
*
* DESCRIPTION:
*      Read entry from physical port to Anode index data base
*
* APPLICABLE DEVICES:
*        Falcon
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       devNum        -  device number
*       dmaNum       - Global DMA number(0..263).
*       tileNum         - traffic class queue on this device (0..7).
*
* OUTPUTS:
*      aNodeNumPtr   - Index of A node
*      tileNumPtr   - Index of tile
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
GT_STATUS prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  physicalPortNumber,
     OUT   GT_U32   * tileNumPtr,
     OUT   GT_U32   * aNodeNumPtr
) ;

/**
* @internal prvCpssFalconTxqUtilsNumOfQueuesInAnodeGet function
* @endinternal
*
* @brief  Initialize available PB size used for DBA. Initialize to maximal value.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] aNodePtr                   -         `                          (pointer to )aNode
*
* @retval  Number of queues mapped to A node
*
*
*/
static GT_U32 prvCpssFalconTxqUtilsNumOfQueuesInAnodeGet
(
    IN  PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr
)
{
    return aNodePtr->queuesData.pdqQueueLast-aNodePtr->queuesData.pdqQueueFirst+1;
}

/**
* @internal prvCpssFalconTxqUtilsLowLatencyQueuePresentInProfileGet function
* @endinternal
*
* @brief  Check if profile contain TC configured to SP
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                                            physical device number
* @param[in] profileSet                                                          scheduling profile
* @param[out] queueOffsetPtr                                            (pointer to)queue offset that is SP
*
* @retval GT_TRUE                    -           profile contain SP queue
* @retval GT_FALSE                    -           profile does not contain SP queue
*
*
*/
static GT_BOOL prvCpssFalconTxqUtilsLowLatencyQueuePresentInProfileGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_U32                                  *queueOffsetPtr
)
{
     GT_U32 bmp;
     *queueOffsetPtr=0;

     bmp= PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp ^(0xFFFF);
     if((bmp)!= 0)
     {
        while((bmp&0x1)==0)
        {
            bmp>>=1;
            (*queueOffsetPtr)++;
        }

        return GT_TRUE;
     }

     return GT_FALSE;
}

/**
* @internal prvCpssFalconTxqUtilsLowLatencyQueuePresentInProfileGet function
* @endinternal
*
* @brief  Check if pNode contain TC configured to SP
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                                            physical device number
* @param[in] pNode                                                          p node
* @param[out] presentPtr                                            (pointer to)GT_TRUE pNode is contain SP queue,GT_FALSE otherwise
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS prvCpssFalconTxqUtilsLowLatencyQueuePresentInPnodeGet
(
    IN  GT_U8                           devNum,
    IN  PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNode,
    OUT GT_BOOL                        *presentPtr
)
{
   GT_STATUS rc;
   GT_U32    i,queueOffset;
   CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profile;

   *presentPtr=GT_FALSE;

   for(i=0;i<pNode->aNodeListSize;i++)
   {
        rc =prvCpssFalconTxqUtilsPortSchedulerProfileGet(devNum,pNode->aNodelist[i].physicalPort,&profile);

        if(rc==GT_NOT_FOUND)
        {   /*may be a case when port is not mapped to any profile yet*/
            continue;
        }

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPortSchedulerProfileGet failed ");
        }
        if(prvCpssFalconTxqUtilsLowLatencyQueuePresentInProfileGet(devNum,profile,&queueOffset))
        {
            /*check that this queue is present */
            if(queueOffset<prvCpssFalconTxqUtilsNumOfQueuesInAnodeGet(&(pNode->aNodelist[i])))
            {
                *presentPtr=GT_TRUE;
                break;
            }
        }
   }

   return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber function
* @endinternal
*
* @brief  Find P node connected to physical port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                                            physical device number
* @param[in] portNum                                                          physical port number
* @param[out] outPtr                                            (pointer to)p node
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE ** outPtr
)
{
    GT_U32 pNodeIndex,aNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_STATUS rc;
    GT_U32 tileNum;

    rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,&tileNum,&aNodeIndex);

    if(rc!=GT_OK)
    {
        return rc;
    }

    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);

    pNodeIndex = tileConfigsPtr->mapping.searchTable.aNodeIndexToPnodeIndex[aNodeIndex];

    if(pNodeIndex==CPSS_PDQ_SCHED_INVAL_DATA_CNS)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else
    {
      *outPtr= &(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeIndex]);
    }

    return GT_OK;

}


GT_U32 prvCpssFalconTxqUtilsGetPnodeIndex
(
    GT_U32 devNum,
    GT_U32 dpNum,
    GT_U32 localdpPortNum
)
{

    GT_U32 pNodeNum;

    pNodeNum = dpNum + (MAX_DP_IN_TILE(devNum))*localdpPortNum;

    return pNodeNum;

}
/**
* @internal prvCpssFalconTxqUtilsFlowControlInitAvailebleBufferSize function
* @endinternal
*
* @brief  Initialize available PB size used for DBA. Initialize to maximal value.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS prvCpssFalconTxqUtilsFlowControlInitAvailebleBufferSize
(
    IN GT_U8                                devNum
)
{
   GT_U32 numberOfTiles;
   GT_STATUS rc;
   GT_U8 trafficClass;


   switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
   {
       case 0:
       case 1:
           numberOfTiles = 1;
           break;
      case 2:
      case 4:
           numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
           break;
       default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
           break;

   }

   if(prvCpssDxchFalconCiderVersionGet()>= PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_07_09_E)
   {
      for(trafficClass = 0; trafficClass < TC_NUM_CNS; trafficClass++)
      {
            rc = cpssDxChPortPfcDbaTcAvailableBuffersSet(devNum,CPSS_DATA_PATH_UNAWARE_MODE_CNS,
                CPSS_PORT_TX_PFC_AVAILEBLE_BUFF_CONF_MODE_GLOBAL_AND_PORT_TC_E,
                trafficClass, numberOfTiles*PRV_CPSS_FC_PB_SIZE_PER_TILE_MAC);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxMcastAvailableBuffersSet failed ");
            }
       }
    }

   rc = cpssDxChPortPfcDbaAvailableBuffSet(devNum,CPSS_DATA_PATH_UNAWARE_MODE_CNS,
        numberOfTiles*PRV_CPSS_FC_PB_SIZE_PER_TILE_MAC);

   if(rc!=GT_OK)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortPfcDbaAvailableBuffSet failed ");
   }

   return GT_OK;

}



/**
* @internal prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
*         to global DMA number in the device.
*         the global DMA number must be registered in 'port mapping' at this stage
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tileId                   - The tile Id
* @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
* @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileId,
    IN  GT_U32  localDpIndexInTile,
    IN  GT_U32  localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS rc;
    GT_U32 globalDmaNum;
    GT_PHYSICAL_PORT_NUM physicalPortNum;

    /* get the global DMA number of the port */
    rc = prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileId,
            localDpIndexInTile,localDmaNumInDp,&globalDmaNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    do
    {
        rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum,
            PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,globalDmaNum,&physicalPortNum);
        if(rc == GT_OK)
        {
            *globalDmaNumPtr = globalDmaNum;

            return GT_OK;
        }
        /****************************************************************/
        /* maybe the {dpNum,localPortNum} hold other DMA port number(s) */
        /****************************************************************/
        rc = prvCpssFalconDmaGlobalDmaMuxed_getNext(devNum,&globalDmaNum);
        if(rc != GT_OK)
        {
            break;
        }
    }
    while(1);

    return /* not error for the LOG */ GT_NOT_FOUND;
}

/**
* @internal prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource function
* @endinternal
*
* @brief   Find index of port in CPSS_DXCH_PORT_MAP_STC by dp index/local port
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] sourcePtr                -pointer to structure that is going to be searched(array of mappings)
* @param[in] size                     - of structure (Number of ports to map, array size)
* @param[in] tileNum                  -  Number of the tile (APPLICABLE RANGES:0..4).
* @param[in] dpNum                    - Number of the dp (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - Number of the local port (APPLICABLE RANGES:0..8).
*
* @param[out] indexPtr                 - index of found port in sourcePtr
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong tile number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_PORT_MAP_STC * sourcePtr,
    IN GT_U32 size,
    IN GT_U32 tileNum,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 * indexesArr,
    OUT GT_U32 * indexesSizePtr
)
{
    GT_U32 i,j,physicalPorts[PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC];
    GT_U32 physicalPortsNum=0;
    GT_STATUS rc;
    GT_U32  globalDmaNum;

    /* get the global DMA number of the port */
    rc = prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileNum,
            dpNum,localPortNum,&globalDmaNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet(devNum,globalDmaNum,&physicalPortsNum,physicalPorts);
    if(rc!=GT_OK)
    {
        return rc;
    }

    *indexesSizePtr = 0;

    for(j=0;j<size;j++)
    {
        for(i=0;i<physicalPortsNum;i++)
        {
            if(sourcePtr[j].physicalPortNumber == physicalPorts[i])
            {
                indexesArr[*indexesSizePtr] = j;
                (*indexesSizePtr)++;
            }
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget function
* @endinternal
*
* @brief   Find index of port in pNodeMappingConfiguration by dp index/local port
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNumber               - Number of the tile (APPLICABLE RANGES:0..3).
* @param[in] dpNum                    - Number of the dp (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - Number of the local port (APPLICABLE RANGES:0..8).
* @param[out] indexPtr                - index of found port in pNodeMappingConfiguration
* @param[in] isErrorForLog            - indication the CPSS ERROR should be in the LOG or not
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong tile number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 * indexPtr,
    IN GT_BOOL isErrorForLog
)
{
    GT_STATUS   rc;
    GT_U32  globalDmaNum;



    /*1. Find port RxDma by sdq/local port*/
    /*2. Find Pnode by rxDma*/

    /* get the global DMA number of the port */
    rc = prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileNumber,
            dpNum,localPortNum,&globalDmaNum);

    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum,globalDmaNum,NULL,indexPtr,isErrorForLog);
}


/**
* @internal prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber function
* @endinternal
*
* @brief   Find dp index/local port of physical port
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] physPort                 - physical port number
*
* @param[out] tileNumPtr               - Number of the tile (APPLICABLE RANGES:0..3).
* @param[out] dpNumPtr                 - Number of the dp (APPLICABLE RANGES:0..7).
* @param[out] localPortNumPtr - Number of the local port (APPLICABLE RANGES:0..8).
* @param[out] mappingTypePtr           - mapping type
*
* @retval GT_OK                    -           on success.
* @retval GT_BAD_PARAM             - wrong tile number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32  globalDmaNum;

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physPort, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    globalDmaNum = portMapShadowPtr->portMap.rxDmaNum;

    rc = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(devNum,globalDmaNum,tileNumPtr,dpNumPtr,localdpPortNumPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(mappingTypePtr)
     {
        *mappingTypePtr = portMapShadowPtr->portMap.mappingType;
     }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber function
* @endinternal
*
* @brief   Find A node by physical port number
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong tile number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE ** outPtr
)
{
    GT_U32 pNodeIndex,aNodeIndex,i;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_STATUS rc;
    GT_U32 tileNum;

    rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,&tileNum,&aNodeIndex);

     if(rc!=GT_OK)
      {
          return rc;
      }

    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);


    pNodeIndex = tileConfigsPtr->mapping.searchTable.aNodeIndexToPnodeIndex[aNodeIndex];

    for(i=0;i<tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeIndex].aNodeListSize;i++)
    {
        if(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeIndex].aNodelist[i].aNodeIndex == aNodeIndex)
        {
            *outPtr = &(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeIndex].aNodelist[i]);
            return GT_OK;
        }
    }

   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

}

/**
* @internal prvCpssFalconTxqUtilsLastQueueIndexForDpGet function
* @endinternal
 *
* @brief   Set first  queue index for DP
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                   -tile number
* @param[in] ,                   -           DP number.
* @param[out]queueIndex      first valid queue index
*/
static GT_STATUS  prvCpssFalconTxqUtilsStartQueueIndexForDpSet
(
    GT_U8  devNum,
    GT_U32  tileNum,
    GT_U32  dp,
    GT_U32  queueIndex
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(dp);

    if((queueIndex>=CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Trying to configure start queue %d\n",queueIndex);
    }

    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);

    tileConfigsPtr->mapping.firstQInDp[dp] = queueIndex;

    return GT_OK;
}
/**
* @internal prvCpssFalconTxqUtilsDpActiveStateGet function
* @endinternal
 *
* @brief   Check if this DP has mapped queues
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                   -tile number
* @param[in]dp                   -           DP number.
* @param[out]dpActivePtr      (pointer to)GT_TRUE if DP has queue mapped ,GT_FALSE otherwise
*/
static GT_STATUS  prvCpssFalconTxqUtilsDpActiveStateGet
(
    GT_U8    devNum,
    GT_U32   tileNum,
    GT_U32   dp,
    GT_BOOL  *dpActivePtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(dp);

    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);

    if(tileConfigsPtr->mapping.firstQInDp[dp] ==CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM)
    {
        *dpActivePtr = GT_FALSE;
    }
    else
    {
        *dpActivePtr = GT_TRUE;
    }

    return GT_OK;
}



/**
* @internal prvCpssFalconTxqUtilsStartQueueIndexForDpGet function
* @endinternal
*
* @brief   In order to be alligned with port mode excel start queue of each DP is fixed.
*         This function return first queue for each DP.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*                                       Start queue number per DP
*/

GT_U32  prvCpssFalconTxqUtilsStartQueueIndexForDpGet
(
    GT_U8  devNum,
    GT_U32  tileNum,
    GT_U32  dp,
    GT_U32  *queueIndexPtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    GT_U32 startIndex;
    GT_BOOL mirroredTile = (tileNum%2)?GT_TRUE:GT_FALSE;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
     tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);

    if(tileConfigsPtr->mapping.mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
    {
        if(mirroredTile==GT_FALSE)
        {
            /*Fixed queue start per DP*/
            switch(dp)
            {
                case 0:
                    startIndex = 0;
                    break;
               case 1:
                    startIndex = 72;
                    break;
                case 2:
                    startIndex = 144;
                    break;
               case 3:
                    startIndex = 216;
                    break;
               case 4:
                    startIndex = 288;
                    break;
               case 5:
                    startIndex = 680;
                    break;
                case 6:
                    startIndex = 1072;
                    break;
               case 7:
                    startIndex = 1464;
                    break;
                default:/*For last DP*/
                    startIndex = 1856;
                    break;
            }
       }
           else
           {
                /*Fixed queue start per DP*/
                switch(dp)
                {
                    case 0:
                        startIndex = 1784;
                        break;
                   case 1:
                        startIndex = 1712;
                        break;
                    case 2:
                        startIndex = 1640;
                        break;
                   case 3:
                        startIndex = 1568;
                        break;
                   case 4:
                        startIndex = 1176;
                        break;
                   case 5:
                        startIndex = 784;
                        break;
                    case 6:
                        startIndex = 392;
                        break;
                   case 7:
                        startIndex = 0;
                        break;
                    default:/*For last DP*/
                        startIndex = 1856;
                        break;
                }
           }
        }
        else
        {
            TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(dp);
            startIndex = tileConfigsPtr->mapping.firstQInDp[dp];
            if(startIndex>=CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Bad start queue %d for DP %d\n",startIndex,dp);
            }
        }

       *queueIndexPtr = startIndex;

        return GT_OK;
}
/**
* @internal prvCpssFalconTxqUtilsLastQueueIndexForDpGet function
* @endinternal
 *
* @brief   Get last valid queue index for DP
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                   -tile number
* @param[in] ,                   -           DP number.
* @param[out]queueIndexPtr      (pointer to)last valid queue index
*/
GT_STATUS  prvCpssFalconTxqUtilsLastQueueIndexForDpGet
(
    GT_U8  devNum,
    GT_U32  tileNum,
    GT_U32  dp,
    GT_U32  *queueIndexPtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_STATUS rc;

    GT_U32 lastIndex;
    GT_BOOL mirroredTile = (tileNum%2)?GT_TRUE:GT_FALSE;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
     tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);

    if(tileConfigsPtr->mapping.mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
    {
        if(mirroredTile==GT_FALSE)
        {
            /*Fixed queue start per DP*/
            switch(dp)
            {
                case 0:
                    lastIndex = 71;
                    break;
               case 1:
                    lastIndex = 143;
                    break;
                case 2:
                    lastIndex = 215;
                    break;
               case 3:
                    lastIndex = 287;
                    break;
               case 4:
                    lastIndex = 679;
                    break;
               case 5:
                    lastIndex = 1071;
                    break;
                case 6:
                    lastIndex = 1463;
                    break;
               case 7:
                    lastIndex = 1855;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Requested last queue %d for DP %d\n",dp);
                    break;
            }
       }
           else
           {
                /*Fixed queue start per DP*/
                switch(dp)
                {
                    case 0:
                        lastIndex = 1855;
                        break;
                   case 1:
                        lastIndex = 1783;
                        break;
                    case 2:
                        lastIndex = 1711;
                        break;
                   case 3:
                        lastIndex = 1639;
                        break;
                   case 4:
                        lastIndex = 1567;
                        break;
                   case 5:
                        lastIndex = 1175;
                        break;
                    case 6:
                        lastIndex = 783;
                        break;
                   case 7:
                        lastIndex = 391;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Requested last queue %d for DP %d\n",dp);
                        break;
                }
           }
        }
        else
        {   rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum,tileNum,dp,&lastIndex);
            if(rc!=GT_OK)
            {
                return rc;
            }
            lastIndex += (CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC-1);
        }

       *queueIndexPtr = lastIndex;

        return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsInitDmaToPnodeDb function
* @endinternal
 *
* @brief   Initialize DmaToPnode database
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*/

GT_VOID  prvCpssFalconTxqUtilsInitDmaToPnodeDb
(
    IN GT_U8 devNum
)
{

     GT_U32 i;

    for(i=0;i<PRV_CPSS_MAX_DMA_NUM_CNS ;i++)
    {
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[i].pNodeInd = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[i].tileInd = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
    }

}


/**
* @internal prvCpssFalconTxqUtilsInitSchedProfilesDb function
* @endinternal
 *
* @brief   Initialize schedProfiles database . All the queues are in WRR gropu 0. TC 0-3 weight 1 , TC 0-3 weight 5
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*/
GT_VOID  prvCpssFalconTxqUtilsInitSchedProfilesDb
(
    IN GT_U8 devNum
)
{

      GT_U32 i,j;

        /*init scheduling profiles*/
       for(i=0;i<CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM;i++)
        {
            /*all ports are binded to profile 0 */
            for(j=0;j<32;j++)
             {
               PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[j]= 0x0;
             }


            /*all queues in group 0*/
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].wrrEnableBmp = 0xFFFF;
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].wrrGroupBmp = 0;
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].lowLatencyQueue = 0;

             for(j=0;j<QUEUES_PER_REMOTE_PORT_MAX_Q_NUM_VAL_MAC;j++)
             {
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].weights[j]= (j<4)?CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS:(5*CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS);
             }

        }

}


/**
* @internal prvCpssFalconTxqUtilsInitDb function
* @endinternal
*       NONE
*
* @brief   Initialize txq database
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] tileConfigsPtr           - pointer to SW  shadow
* @param[in] tileNumber               - the number of the tile(APPLICABLE RANGES:0..3)
*
*/
GT_VOID  prvCpssFalconTxqUtilsInitDb
(
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr,
    IN GT_U32                              tileNumber,
    IN GT_U32                              numberOfTiles
)
{
    GT_U32 i;

      for(i=0;i<CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC;i++)
    {

       tileConfigsPtr->mapping.searchTable.aNodeIndexToPnodeIndex[i]= CPSS_PDQ_SCHED_INVAL_DATA_CNS;
    }

    for(i=0;i<CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM;i++)
    {

       tileConfigsPtr->mapping.searchTable.qNodeIndexToAnodeIndex[i]= CPSS_PDQ_SCHED_INVAL_DATA_CNS;
    }

    for(i=0;i<CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM;i++)
    {
       tileConfigsPtr->mapping.pNodeMappingConfiguration[i].aNodeListSize = 0;
    }


    for(i=0;i<4;i++)
    {  /*No need to configure this for single tile device*/
       tileConfigsPtr->general.pdxMapping[i] = (numberOfTiles==1)?0:falconPdxInterfaceMap[tileNumber][i];
    }


    for(i=0;i<CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC;i++)
    {
       tileConfigsPtr->mapping.firstQInDp[i] = CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM;/*Treat max is uninitialized*/
    }

    tileConfigsPtr->mapping.mappingMode = PRV_CPSS_QUEUE_MAPPING_MODE_DEFAULT_MAC;


}

/**
* @internal prvCpssFalconTxqUtilsConvertToDb function
* @endinternal
*
* @brief   Convert to txq database format from CPSS_DXCH_PORT_MAP_STC .
*         Create mapping between P nodes /A nodes and physical port numbers
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] sourcePtr                - Pointer to CPSS_DXCH_PORT_MAP_STC to be converted into scheduler tree
* @param[in] size                     - of source array.Number of ports to map, array size
* @param[in] tileNum                  -  The number of tile
*
* @param[out] targetPtr                - Pointer to schedualler mappping configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsConvertToDb
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_PORT_MAP_STC * sourcePtr,
    IN GT_U32 size,
    IN GT_U32 tileNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  *  targetPtr
)
{
    GT_U32 aNodeListSize;
    GT_U32 j,k;
    GT_U32 currentIndex=0;
    GT_U32 currentAnodeIterator=0;
    GT_U32 currentQnumIterator=0;
    GT_STATUS rc;
    GT_U32 foundIndex[PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC] ,foundIndexInTarget =0 ;
    GT_U32 numOfPorts,lastAllowedQueueInDp;
    GT_U32 remotePortsIterator;
    GT_U32 startDp;
    GT_32 i;
    GT_BOOL firstQueueInDpSet;
    GT_U32  numberOfRequestedQueues;


    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    /*First arrange correct numbering for schedualing .Arrange by global mac

            1.loop over PDS and local port
            2.Find physical port by (PDS,local port)
            3.If found port ,then scheduled

            */

    for(i=0;i<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;i++)
    {

        for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
        {

           rc =  prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource(devNum,sourcePtr,size,tileNum,j,i,foundIndex,&numOfPorts);

           if(rc == GT_OK)
           {
                rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, sourcePtr[foundIndex[0]].physicalPortNumber, /*OUT*/&portMapShadowPtr);

                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                currentIndex = prvCpssFalconTxqUtilsGetPnodeIndex(devNum,j,i);

                 rc = prvCpssFalconTxqUtilsSetDmaToPnodeMapping(devNum,portMapShadowPtr->portMap.rxDmaNum,tileNum,currentIndex);

                if (rc != GT_OK)
                {
                   return rc;
                }

                /*PFC mapping*/
                rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum,tileNum,j,i,portMapShadowPtr->portMap.rxDmaNum,
                    CPSS_DXCH_PORT_FC_MODE_DISABLE_E,GT_FALSE);

                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortSet failed for dp %d local port %d \n",j,i);
                }
           }


        }
    }

    targetPtr->size = 72;

    /*Now add A nodes*/

    /*

            1.loop over PDS and local port
            2.Find physical port by (PDS,local port)
            3a.If CPU or local port create A node with 8 queues
            3b.If remote port then look in source all the remote ports that have the same MAC and give then 8 queues - TBD
      */

    /*For mirrored tile A nodes are mirrored in port mode xcl*/
    if(tileNum%2)
    {
       startDp=MAX_DP_IN_TILE(devNum)-1;
    }
    else
    {
        startDp=0;
    }

    currentQnumIterator = 0;

    for(i= startDp;(tileNum%2)?i>=0:(GT_U32)i<MAX_DP_IN_TILE(devNum);(tileNum%2)?i--:i++)
    {
        if(targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
        {
             rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum,tileNum,i,&currentQnumIterator);
             if (rc != GT_OK)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
             }

         }

        firstQueueInDpSet = GT_FALSE;

        for(j=0;j<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;j++)
        {
            rc =  prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource(devNum,sourcePtr,size,tileNum,i,j,foundIndex,&numOfPorts);

            if(rc == GT_OK)
            {

                if(firstQueueInDpSet == GT_FALSE)
                {
                    rc = prvCpssFalconTxqUtilsStartQueueIndexForDpSet(devNum,tileNum,i,currentQnumIterator);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                    firstQueueInDpSet=GT_TRUE;
                }

                for(remotePortsIterator=0;remotePortsIterator<numOfPorts;remotePortsIterator++)
                {
                    switch(sourcePtr[foundIndex[remotePortsIterator]].mappingType)
                    {
                        case CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E:
                        case CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E:

                            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber, /*OUT*/&portMapShadowPtr);

                            if (rc != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                            }


                            rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum,portMapShadowPtr->portMap.rxDmaNum,NULL,&foundIndexInTarget,GT_TRUE);

                            if (rc != GT_OK)
                             {
                                 return rc;
                             }
                            if(targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
                            {
                                if(j== CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC-1)
                                {
                                 if(tileNum%2)
                                 {
                                  rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum,tileNum,i-1,&currentQnumIterator);
                                 }
                                 else
                                 {
                                   rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum,tileNum,i+1,&currentQnumIterator);
                                 }
                                 if (rc != GT_OK)
                                 {
                                      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                                 }
                                  /*Last 8 are reserved for CPU*/
                                  currentQnumIterator -=QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
                                }
                             }

                            aNodeListSize = targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodeListSize;

                            if(aNodeListSize>0||numOfPorts>1)
                            {
                                if(sourcePtr[foundIndex[remotePortsIterator]].mappingType== CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
                                 {
                                        targetPtr->pNodeMappingConfiguration[foundIndexInTarget].isCascade = GT_TRUE;
                                         targetPtr->pNodeMappingConfiguration[foundIndexInTarget].cascadePhysicalPort = sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber;

                                        CPSS_LOG_INFORMATION_MAC("Cascaded port  :%d\n",sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber);
                                        continue; /*This must be cascaded port - no need to map*/
                                 }
                                else /*CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E*/
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                                    "physical port[%d] try to use global DMA[%d] that is already used (Local  port  %d already contain  mapped physical port)",
                                    sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber,
                                    portMapShadowPtr->portMap.rxDmaNum,
                                    j);
                                 }
                            }

                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodeListSize++;

                            /*A nodes are consequitive*/
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].aNodeIndex = currentAnodeIterator;
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].physicalPort = sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber;
                            /*Check that currentQnumIterator+number of mapped queues confirm the limitations*/

                            rc =prvCpssFalconTxqUtilsLastQueueIndexForDpGet(devNum,tileNum,i,&lastAllowedQueueInDp);
                            if(rc!=GT_OK)
                            {
                                return rc;
                            }

                            if(targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
                            {
                                if(j<8)
                                {
                                    /*last 8  queues are reserved for CPU*/
                                    lastAllowedQueueInDp-=QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
                                }
                            }

                            numberOfRequestedQueues = sourcePtr[foundIndex[remotePortsIterator]].txqPortNumber;

                            if(numberOfRequestedQueues<=0)
                            {
                                numberOfRequestedQueues = QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
                            }

                            if(numberOfRequestedQueues>QUEUES_PER_REMOTE_PORT_MAX_Q_NUM_VAL_MAC)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "numberOfRequestedQueues is too big %d ,  for tile %d dp %d local port %d ",numberOfRequestedQueues,tileNum,i,j);
                            }

                            if(currentQnumIterator+numberOfRequestedQueues-1 > lastAllowedQueueInDp)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DP queue limits are exhausted for tile %d dp %d ",tileNum,i);
                            }

                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueFirst= currentQnumIterator;
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].mappingType =  sourcePtr[foundIndex[remotePortsIterator]].mappingType;
                            targetPtr->searchTable.aNodeIndexToPnodeIndex[currentAnodeIterator] = foundIndexInTarget;

                            rc = prvCpssFalconTxqUtilsSetPhysicalNodeToAnodeMapping(devNum,sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber,
                                    tileNum,currentAnodeIterator);

                            if(rc!=GT_OK)
                            {
                                return rc;
                            }

                            /*Queues arrangment need to be TBD also ,need to be taken from source */
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueLast=
                                currentQnumIterator+numberOfRequestedQueues -1;

                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.queueGroupIndex =
                                sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber;
                            for(k = currentQnumIterator; k<targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueLast+1;k++)
                            {
                                targetPtr->searchTable.qNodeIndexToAnodeIndex[k] = currentAnodeIterator;
                            }
                            /*Q nodes are consequitive*/
                            currentQnumIterator = targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueLast+1;
                            currentAnodeIterator++;
                            break;
                        case CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E:

                            if((i<4)&&(targetPtr->mappingMode ==PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC))
                            {
                               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Remote port is mapped on DP %d.Must be lower then 4",i);
                            }

                             rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber, /*OUT*/&portMapShadowPtr);

                            if (rc != GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                            }


                            rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum,portMapShadowPtr->portMap.rxDmaNum,NULL,&foundIndexInTarget,GT_TRUE);


                            if (rc != GT_OK)
                             {
                                 return rc;
                             }

                            aNodeListSize = targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodeListSize;

                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodeListSize ++;

                            if(j== CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC-1)
                            {
                                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Remote port can not be located at local port %d",j);
                            }

                            /*A nodes are consequitive*/
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].aNodeIndex = currentAnodeIterator;
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].physicalPort = sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber;

                            rc =prvCpssFalconTxqUtilsLastQueueIndexForDpGet(devNum,tileNum,i,&lastAllowedQueueInDp);
                            if(rc!=GT_OK)
                            {
                                return rc;
                            }

                            if(targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
                            {
                               /*last 8  queues are reserved for CPU (j<8 this is a remote)*/
                               lastAllowedQueueInDp-=QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
                            }

                            numberOfRequestedQueues = sourcePtr[foundIndex[remotePortsIterator]].txqPortNumber;

                            if(numberOfRequestedQueues<=0)
                            {
                                numberOfRequestedQueues = QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
                            }

                            if(numberOfRequestedQueues>QUEUES_PER_REMOTE_PORT_MAX_Q_NUM_VAL_MAC)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "numberOfRequestedQueues is too big %d ,  for tile %d dp %d local port %d ",numberOfRequestedQueues,tileNum,i,j);
                            }

                            if(currentQnumIterator+numberOfRequestedQueues-1 > lastAllowedQueueInDp)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DP queue limits are exhausted for dp %d ",i);
                            }

                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueFirst= currentQnumIterator;
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E;

                            targetPtr->searchTable.aNodeIndexToPnodeIndex[currentAnodeIterator] = foundIndexInTarget;

                            rc = prvCpssFalconTxqUtilsSetPhysicalNodeToAnodeMapping(devNum,sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber,
                                    tileNum,currentAnodeIterator);

                            if(rc!=GT_OK)
                            {
                                return rc;
                            }

                            /*Queues arrangment need to be TBD also ,need to be taken from source */
                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueLast=
                                currentQnumIterator+numberOfRequestedQueues -1;

                            targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.queueGroupIndex =
                                sourcePtr[foundIndex[remotePortsIterator]].physicalPortNumber;

                            for(k = currentQnumIterator; k<targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueLast+1;k++)
                            {
                                targetPtr->searchTable.qNodeIndexToAnodeIndex[k] = currentAnodeIterator;
                            }


                            /*Q nodes are consequitive*/
                            currentQnumIterator = targetPtr->pNodeMappingConfiguration[foundIndexInTarget].aNodelist[aNodeListSize].queuesData.pdqQueueLast+1;


                            currentAnodeIterator++;

                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                CPSS_LOG_INFORMATION_MAC("Converting to txq data base : Empty slot dp %d local port %d\n",i,j);
            }
        }
    }
    for(i=0;(GT_U32)i<MAX_DP_IN_TILE(devNum);i++)
    {


        if(GT_OK==prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
            (devNum,tileNum,i,CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC-1,&foundIndexInTarget,GT_FALSE))
        {
            targetPtr->pdxPizza.pdxPizzaNumberOfSlices[i] = TXQ_PDX_MAX_SLICE_NUMBER_MAC;

        }
        else
        {
            targetPtr->pdxPizza.pdxPizzaNumberOfSlices[i] = TXQ_PDX_MIN_SLICE_NUMBER_MAC;
        }
    }

    /*Bind all mapped ports to default scheduling profile
    prvCpssFalconTxqUtilsDefaultBindPortsToSchedDb(devNum);*/


    return GT_OK;
}

/**
* @internal prvCpssDxChTxQFalconPizzaArbiterInitPerTile function
* @endinternal
*
* @brief   Initialize the number of silices ta each PDX PDS pizza arbiter,also initialize all the slices to unused
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] pdxNum                   -tile number
* @param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*@param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
*/

GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerTile
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 * pdxPizzaNumberOfSlicesArr
)
{
    GT_U32  i;
    GT_STATUS rc;


    for(i=0;i<MAX_DP_IN_TILE(devNum);i++)
    {
        rc = prvCpssDxChTxQFalconPizzaArbiterInitPerDp(devNum,pdxNum,i,pdxPizzaNumberOfSlicesArr[i]);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }


    }

    return GT_OK;
}
/**
* @internal prvCpssFalconTxqUtilsInitPdxInterfaceTable function
* @endinternal
*
* @brief   Initialize PDX interface table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitPdxInterfaceTable
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION  *   generalCfgPtr;

    generalCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum].general);

    return  prvCpssDxChTxqFalconPdxInterfaceMapSet(devNum,tileNum,&(generalCfgPtr->pdxMapping[0]));
}

/**
* @internal prvCpssFalconTxqUtilsInitPdxQroupMapTable function
* @endinternal
*
* @brief   Initialize PDX DX_QGRPMAP table (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] mappingPtr               - pointer to SW mapping shadow
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitPdxQroupMapTable
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  * mappingPtr

)
{
    GT_U32 portIterator,aNodeListSize,aNodeListIterator,queueGroupIndex;
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC txPortMap = {0};
    GT_STATUS rc;
    GT_U32 physicalPort;
    GT_U32 firstQueueInDp;

    for(portIterator = 0;portIterator<mappingPtr->size;portIterator++)
    {
        aNodeListSize = mappingPtr->pNodeMappingConfiguration[portIterator].aNodeListSize;

        for(aNodeListIterator = 0;aNodeListIterator<aNodeListSize;aNodeListIterator++)
        {
            queueGroupIndex =
                mappingPtr->pNodeMappingConfiguration[portIterator].aNodelist[aNodeListIterator].queuesData.queueGroupIndex;

            physicalPort  = mappingPtr->pNodeMappingConfiguration[portIterator].aNodelist[aNodeListIterator].physicalPort;

            rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,physicalPort,
                &txPortMap.queuePdxIndex ,
                &txPortMap.queuePdsIndex,
                &txPortMap.dpCoreLocalTrgPort,NULL);

            if(rc!=GT_OK)
            {
               return rc;
            }

            /*Find first  Q  in current DP*/

            rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum,txPortMap.queuePdxIndex,txPortMap.queuePdsIndex,&firstQueueInDp);
            if(rc!=GT_OK)
            {
               return rc;
            }

            txPortMap.queueBase =
                mappingPtr->pNodeMappingConfiguration[portIterator].aNodelist[aNodeListIterator].queuesData.pdqQueueFirst -
                firstQueueInDp;
            mappingPtr->pNodeMappingConfiguration[portIterator].aNodelist[aNodeListIterator].queuesData.queueBase = txPortMap.queueBase;
            mappingPtr->pNodeMappingConfiguration[portIterator].aNodelist[aNodeListIterator].queuesData.dp = txPortMap.queuePdsIndex;

            /*Init local PDS/SDQ db*/

            if(mappingPtr->pNodeMappingConfiguration[portIterator].isCascade == GT_FALSE )
            {

                mappingPtr->pNodeMappingConfiguration[portIterator].sdqQueueFirst= txPortMap.queueBase;
                mappingPtr->pNodeMappingConfiguration[portIterator].sdqQueueLast = mappingPtr->pNodeMappingConfiguration[portIterator].aNodelist[aNodeListIterator].queuesData.pdqQueueLast-
                    firstQueueInDp;
            }
            else
            {
                if(aNodeListIterator==0)
                {
                    mappingPtr->pNodeMappingConfiguration[portIterator].sdqQueueFirst= txPortMap.queueBase;
                }
                else
                /*check consequity*/
                 if(mappingPtr->pNodeMappingConfiguration[portIterator].sdqQueueLast!=txPortMap.queueBase-1)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portMapPtr->sdqQueueLast [%d] !=txPortMap.queueBase-1 [%d]\n",mappingPtr->pNodeMappingConfiguration[portIterator].sdqQueueLast,txPortMap.queueBase-1);
                }
                /*adding Qs*/
                mappingPtr->pNodeMappingConfiguration[portIterator].sdqQueueLast = mappingPtr->pNodeMappingConfiguration[portIterator].aNodelist[aNodeListIterator].queuesData.pdqQueueLast-
                    firstQueueInDp;
            }

            /*copy for all PDXs is done inside at prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum*/
             rc = prvCpssDxChTxqFalconPdxQueueGroupMapSet(devNum,queueGroupIndex,txPortMap);

             if(rc!=GT_OK)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "queueGroupIndex %d \n",queueGroupIndex );
             }
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsInitSdq function
* @endinternal
*
* @brief   Initialize SDQ (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
* @param[in] totalTiles                  -Number of tiles in the device (Applicable range 1..4)
* @param[in] mappingPtr               - pointer to SW mapping shadow
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitSdq
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 totalTiles,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  * mappingPtr

)
{
     GT_STATUS              rc = GT_OK;
     GT_U32                 sdqNum =0;

     GT_U32                 localPortNum,foundIndex;
     GT_U32                 i,j;
     PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE range;
     GT_U32 tc;
     GT_BOOL localPortMapped;
     GT_BOOL numberOfMappedQueues;
     GT_U32 localQueueIndex,firstQueueInDp;
     GT_U32 pbFullLimit;


     for(sdqNum = 0;sdqNum<MAX_DP_IN_TILE(devNum);sdqNum++)
     {

        /*set global*/
        rc = prvCpssFalconTxqSdqGlobalConfigEnableSet(devNum,tileNum,sdqNum,GT_FALSE/*aging*/,GT_TRUE/*semi elig*/);

         if(rc!=GT_OK)
         {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqCreditAgingEnableSet\n",rc );
            return rc;
         }


         for(localPortNum=0;localPortNum<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;localPortNum++)
         {
            localPortMapped = GT_FALSE;

            if (prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget(devNum,tileNum,sdqNum,localPortNum,&foundIndex,GT_FALSE)==GT_OK)
            {
                localPortMapped = GT_TRUE;
            }

            if(GT_TRUE == localPortMapped)
            {
                /*enable the port*/
                rc = prvCpssFalconTxqSdqLocalPortEnableSet(devNum,tileNum,sdqNum,localPortNum,GT_TRUE);
                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "localPortNum %d \n",localPortNum );

                }

                i= mappingPtr->pNodeMappingConfiguration[foundIndex].sdqQueueFirst;
                range.lowQueueNumber = i;
                range.hiQueueNumber = mappingPtr->pNodeMappingConfiguration[foundIndex].sdqQueueLast;

                rc = prvCpssFalconTxqSdqLocalPortQueueRangeSet(devNum,tileNum,sdqNum,localPortNum,range);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "localPortNum %d \n",localPortNum );

                }



                for(i=0;i<mappingPtr->pNodeMappingConfiguration[foundIndex].aNodeListSize;i++)
                {
                    numberOfMappedQueues =mappingPtr->pNodeMappingConfiguration[foundIndex].aNodelist[i].queuesData.pdqQueueLast
                        - mappingPtr->pNodeMappingConfiguration[foundIndex].aNodelist[i].queuesData.pdqQueueFirst+1;
                    for(j=mappingPtr->pNodeMappingConfiguration[foundIndex].aNodelist[i].queuesData.pdqQueueFirst;
                            j<=mappingPtr->pNodeMappingConfiguration[foundIndex].aNodelist[i].queuesData.pdqQueueLast;
                            j++)
                     {
                        rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum,tileNum,sdqNum,&firstQueueInDp);
                        if(rc!=GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                        }
                        localQueueIndex = j-firstQueueInDp;
                        /*Default 1 to 1 mapping*/
                        tc = localQueueIndex%numberOfMappedQueues;
                        rc = prvCpssFalconTxqSdqQueueTcSet(devNum,tileNum,sdqNum,localQueueIndex,tc);

                        if(rc!=GT_OK)
                        {
                          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }
                     }
                }
            }
         }

         switch(totalTiles)
         {
            case 1:
             pbFullLimit = 190;
             break;
            case 2:
             pbFullLimit = 381;
             break;
            case 3:
             pbFullLimit = 571;
             break;
            case 4:
             pbFullLimit = 763;
             break;
            default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "totalTiles illigal number %d ",totalTiles);
         }

        rc = prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet(devNum,tileNum,sdqNum,0xBA4,0xBE4,pbFullLimit);
        if(rc!=GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet fail for tile %d dp %d ",tileNum,sdqNum);
        }

     }


     return GT_OK;


}

/**
* @internal prvCpssFalconTxqUtilsInitPsi function
* @endinternal
*
* @brief   Initialize PSI (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS prvCpssFalconTxqUtilsInitPsi
(
    IN GT_U8   devNum,
    IN GT_U32 tileNum
)
{
     GT_STATUS              rc = GT_OK;

     GT_U32                 sdqNum,foundIndex;
     GT_U32                 localPortNum;
     GT_U32                 firstQueueInDp;
     GT_U32                 credit;
     GT_BOOL                dpActive;


      credit = PRV_CPSS_DXCH_TXQ_SIP_6_CREDIT;



        /*set Credit Value to 4K,emulator 2K*/
        rc = prvCpssFalconPsiCreditValueSet(0,tileNum,credit);

         if(rc!=GT_OK)
         {
            cpssOsPrintf("%d = prvCpssFalconPsiCreditValueSet\n",rc );
            return rc;
         }


         for(sdqNum = 0;sdqNum<MAX_DP_IN_TILE(devNum);sdqNum++)
         {
           rc  = prvCpssFalconTxqUtilsDpActiveStateGet(devNum,tileNum,sdqNum,&dpActive);
           if(rc!=GT_OK)
           {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
           }

           /*scip this DP if it does not contain any queues*/
           if(dpActive==GT_FALSE)
           {
              continue;
           }

           rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum,tileNum,sdqNum,&firstQueueInDp);
           if(rc!=GT_OK)
           {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
           }

            for(localPortNum = 0;localPortNum<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;localPortNum++)
            {

                if(prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
                    (devNum,tileNum, sdqNum, localPortNum,&foundIndex,GT_FALSE) == GT_OK)
                {

                    rc = prvCpssFalconPsiMapPortToPdqQueuesSet(devNum,tileNum,foundIndex,firstQueueInDp);

                     if(rc!=GT_OK)
                     {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                     }
                }
              }
         }


     return GT_OK;


}



/**
* @internal prvCpssFalconTxqUtilsBuildPdqTree function
* @endinternal
*
* @brief   Build default schedualing tree
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNumber               -Then number of tile (Applicable range 0..3)
* @param[in] configPtr                - pointer to SW mapping shadow
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsBuildPdqTree
(
    IN GT_U8 devNum,
    IN  GT_U32 tileNumber,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION  * configPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL      firstANode = GT_FALSE;

    PRV_CPSS_PDQ_SCHED_LEVEL_PERIODIC_PARAMS_STC   params[5];


    GT_U32                                     ii,i,j;
    PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC            cParams;
    PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC            bParams;
    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION    * mappingPtr;
    PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC               portParams;
    GT_U32                                      aNodesSize,qNodesSize;
    GT_U32                                      aNodeIndex,aNodeIndexNew = 0,portIndex;
    PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC       aParams;
    GT_U32                             bNodeIndex,cNodeIndex;
    GT_U32                             queueIndex,queueIndexNew,queuePriority;
    PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC     queueParams;





    rc = prvCpssFalconTxqPdqInit(devNum,tileNumber,PRV_CPSS_DXCH_TXQ_SIP_6_CREDIT);


    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "pdqNum %d \n",0 );
    }



        /*****************************************************/
        /* Configure Periodic Scheme    - no shaping                     */
        /*****************************************************/

        /*port level*/
        params[CPSS_TM_LEVEL_P_E].periodicState = GT_FALSE;
        params[CPSS_TM_LEVEL_P_E].shaperDecoupling = GT_FALSE;
        /* c level*/
        params[CPSS_TM_LEVEL_C_E].periodicState = GT_FALSE;
        params[CPSS_TM_LEVEL_C_E].shaperDecoupling = GT_FALSE;
        /* b level*/
        params[CPSS_TM_LEVEL_B_E].periodicState = GT_FALSE;
        params[CPSS_TM_LEVEL_B_E].shaperDecoupling = GT_FALSE;
        /* a level*/
        params[CPSS_TM_LEVEL_A_E].periodicState = GT_TRUE;
        params[CPSS_TM_LEVEL_A_E].shaperDecoupling = GT_FALSE;
        /* q level*/
        params[CPSS_TM_LEVEL_Q_E].periodicState = GT_TRUE;
        params[CPSS_TM_LEVEL_Q_E].shaperDecoupling = GT_FALSE;


        rc = prvCpssFalconTxqPdqPeriodicSchemeConfig(devNum,tileNumber,params);

        if(rc!=GT_OK)
        {
            return rc;
        }






        /*****************************************************/
        /* Configure C parameters                        */
        /*****************************************************/
        cParams.shapingProfilePtr = CPSS_PDQ_SCHED_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        cParams.quantum = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;

        cParams.eligiblePrioFuncId = CPSS_PDQ_SCHED_ELIG_N_PRIO1_E;
        cParams.numOfChildren =1;

         for (ii = 0; ii < 8; ii++)
         cParams.schdModeArr[ii] = CPSS_PDQ_SCHD_MODE_RR_E; /* No DWRR configured*/


        /*****************************************************/
        /* Configure B parameters                        */
        /*****************************************************/
        bParams.shapingProfilePtr = CPSS_PDQ_SCHED_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        bParams.quantum = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;

        bParams.eligiblePrioFuncId =CPSS_PDQ_SCHED_ELIG_N_PRIO1_E;

         for (ii = 0; ii < 8; ii++)
         bParams.schdModeArr[ii] = CPSS_PDQ_SCHD_MODE_DWRR_E; /* No DWRR configured*/





        mappingPtr = &(configPtr->mapping);



         /*create all ports - no shaping*/

        for(i=0;i<mappingPtr->size;i++)
        {

            if(mappingPtr->pNodeMappingConfiguration[i].aNodeListSize ==0)
            {
                /*this local port is not mapped*/

                continue;
            }


            portParams.cirBw = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
            portParams.eirBw = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
            portParams.cbs = CPSS_PDQ_SCHED_INVAL_DATA_CNS; /*KBytes*/
            portParams.ebs = CPSS_PDQ_SCHED_INVAL_DATA_CNS;

            portParams.numOfChildren = 1;/*one C level*/

            for (j = 0; j < 8; j++)
            {  /*the value is already in chunks ,no need to multiply by chunk size*/
                portParams.quantumArr[j] = CPSS_PDQ_SCHED_MIN_PORT_QUANTUM_CNS;
                /*C level scheduling ,so RR*/
                portParams.schdModeArr[j] = CPSS_TM_SCHD_MODE_RR_E;
            }

           portParams.eligiblePrioFuncId = CPSS_PDQ_SCHED_ELIG_N_PRIO1_E;


            rc = prvCpssFalconTxqPdqAsymPortCreate(devNum,tileNumber, i, GT_TRUE/*fixed mapping to C node*/,&portParams);
            if(rc!=GT_OK)
            {
                return rc;
            }

        }

         /*now create all A levels and connect them to port*/


         /*count all A nodes*/
         for(i=0,aNodesSize = 0;i<mappingPtr->size;i++)
         {
            aNodesSize +=  mappingPtr->pNodeMappingConfiguration[i].aNodeListSize;
         }



          for(i=0;i<aNodesSize;i++)
          {
            aNodeIndex = i;

            /*****************************************************/
            /* Configure A parameters                        */
            /*****************************************************/
            aParams.shapingProfilePtr= CPSS_PDQ_SCHED_SHAPING_INFINITE_PROFILE_INDEX_CNS;/*no shaping*/;
            aParams.quantum= CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;
            for (ii = 0; ii < 8; ii++)
                aParams.schdModeArr[ii] = CPSS_TM_SCHD_MODE_DWRR_E; /*  DWRR configured*/

            /* Elig only if more then min and less then max*/
            aParams.eligiblePrioFuncId = DEFAULT_ELIG_FUNC;


            portIndex = mappingPtr->searchTable.aNodeIndexToPnodeIndex[aNodeIndex];



           for(ii=0;ii<mappingPtr->pNodeMappingConfiguration[portIndex].aNodeListSize;ii++)
            {
                     if(mappingPtr->pNodeMappingConfiguration[portIndex].aNodelist[ii].aNodeIndex == aNodeIndex)
                     {
                        break;
                     }
             }

            /*check if need to create or add new*/
           if(ii==0)
            {
               firstANode = GT_TRUE;
            }
            else
            {
               firstANode = GT_FALSE;
            }


            queueIndex = mappingPtr->pNodeMappingConfiguration[portIndex].aNodelist[ii].queuesData.pdqQueueFirst;

            aParams.numOfChildren = mappingPtr->pNodeMappingConfiguration[portIndex].aNodelist[ii].queuesData.pdqQueueLast-queueIndex+1;

            /*Support remote ports*/
            bParams.numOfChildren =mappingPtr->pNodeMappingConfiguration[portIndex].aNodeListSize;

            rc = prvCpssFalconTxqPdqAnodeToPortCreate(devNum,tileNumber,portIndex,&aParams,GT_TRUE,queueIndex,GT_TRUE,&bParams,&cParams,
                        (firstANode== GT_TRUE)?GT_FALSE:GT_TRUE,
                        &aNodeIndexNew,&bNodeIndex,&cNodeIndex);

            if(rc!=GT_OK)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "portIndex %d \n",portIndex );
            }


            if(aNodeIndexNew != aNodeIndex)
            {

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "aNodeIndexNew [%d]  != aNodeIndex [%d]\n",aNodeIndexNew,aNodeIndex);
            }

     }




          /*now create all queue levels and connect them to A level*/


         /*****************************************************/
         /* Configure Queue parameters                        */
         /*****************************************************/
          queueParams.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
          queueParams.quantum = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;

          aNodeIndex=aNodeIndexNew;


          portIndex = mappingPtr->searchTable.aNodeIndexToPnodeIndex[aNodeIndex];

          qNodesSize = mappingPtr->pNodeMappingConfiguration[portIndex].aNodelist[mappingPtr->pNodeMappingConfiguration[portIndex].aNodeListSize-1].queuesData.pdqQueueLast+1;

          for(i=0;i<qNodesSize;i++)
          {

                if(mappingPtr->searchTable.qNodeIndexToAnodeIndex[i] == CPSS_PDQ_SCHED_INVAL_DATA_CNS)
                 {      /*unmapped*/
                        continue;
                 }

                queueIndex = i;

                queuePriority=queueIndex%8;
                queueParams.eligiblePrioFuncId = CPSS_PDQ_SCHED_ELIG_Q_PRIO0_E;

                /*To be alligned to current CPSS init*/
                if(queuePriority<4)
                {
                    queueParams.quantum = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;
                }
                else
                {
                    queueParams.quantum = 5*CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;
                }

                aNodeIndex = mappingPtr->searchTable.qNodeIndexToAnodeIndex[queueIndex];

                if(aNodeIndex!=aNodeIndexNew)
                {
                    aNodeIndexNew = aNodeIndex;
                }

                queueIndexNew = queueIndex;

                rc = prvCpssFalconTxqPdqQueueToAnodeCreate(devNum,tileNumber,aNodeIndex,&queueParams,GT_TRUE,&queueIndexNew);

                if(rc!=GT_OK)
                {
                    return rc;
                }

                if(queueIndexNew != queueIndex)
                {
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"queueIndexNew [%d]  != queueIndex [%d]\n",queueIndexNew,queueIndex);
                }


          }

        /*Disable  DWRR at P level*/

        rc =  prvCpssFalconTxqPdqTreePlevelDwrrEnableSet(devNum,tileNumber,GT_FALSE);

        if(rc !=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"prvCpssFalconTxqPdqTreePlevelDwrrEnableSet failed for tile %d \n",tileNumber);
        }


    return rc;

}




/**
* @internal prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump function
* @endinternal
*
* @brief   Debug function that dump physical port to P node and A node mapping.
*         Also PDQ queue numbers that are mapped to A node are dumped
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*                                      tileNum      - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump
(
    IN  GT_U8 devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 firstPort,
    IN  GT_U32 lastPort
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNode;
    GT_STATUS              rc;
    GT_U32                 i,j,k;
    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  * mappingPtr;
    /*GT_U32                 dpWithCpuPorts[8]={0};
        GT_BOOL             cpuPortExist = GT_FALSE;*/
    GT_U32              currentPortTile=0;
    GT_U32              remote=0;



    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32 aNodeIndex=0,pNodeIndex =0 ,tmp,qNodeFirst=0,qNodeLast=0,globalDmaNum,cascadePort=0;
    GT_BOOL isCascadePort;

    cpssOsPrintf("\nPhysical port to scheduler nodes table - SW shadow");

    cpssOsPrintf("\n+---+------+-------+------------+----------+------+----------------+");
    cpssOsPrintf("\n| # |P node| A node| PDQ Q first|PDQ Q Last|Remote|Cascade port num|");
    cpssOsPrintf("\n+---+------+-------+------------+----------+------+----------------+");

    mappingPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNumber].mapping);

    for(i=firstPort;i<=lastPort;i++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, i, /*OUT*/&portMapShadowPtr);

        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, i,&isCascadePort);
        if(rc != GT_OK)
        {
             return rc;
        }

        if(GT_FALSE == isCascadePort)
        {
            rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,i,&currentPortTile,&aNodeIndex);

            if(rc==GT_NOT_FOUND)
            {
              rc = GT_OK;
            }

            if (rc != GT_OK)
            {
              return rc;
            }
        }

        if(portMapShadowPtr->valid)
        {
            if(GT_FALSE == isCascadePort)
            {
                /*found corresponding P node*/
               if((prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,i,&aNode)== GT_OK)&&
                    (currentPortTile==tileNumber))
               {
                 aNodeIndex = aNode->aNodeIndex;
                 pNodeIndex = mappingPtr->searchTable.aNodeIndexToPnodeIndex[aNode->aNodeIndex];
                 qNodeFirst = aNode->queuesData.pdqQueueFirst;
                 qNodeLast =  aNode->queuesData.pdqQueueLast;

                 if(portMapShadowPtr->portMap.mappingType ==CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
                 {
                    remote = 1;
                    rc =prvCpssFalconTxqUtilsGetCascadePort(devNum,i,GT_TRUE,&cascadePort,NULL);
                    if (rc != GT_OK)
                    {
                      return rc;
                    }
                 }
              }
             }
             else
             {

                rc =prvCpssFalconTxqUtilsGetCascadePort(devNum,i,GT_FALSE,&cascadePort,&pNodeIndex);
                if (rc != GT_OK)
                {
                  return rc;
                }
                currentPortTile=0;
                qNodeFirst = mappingPtr->pNodeMappingConfiguration[pNodeIndex].aNodelist[0].queuesData.pdqQueueFirst;
                aNodeIndex = mappingPtr->pNodeMappingConfiguration[pNodeIndex].aNodeListSize-1;
                qNodeLast=mappingPtr->pNodeMappingConfiguration[pNodeIndex].aNodelist[aNodeIndex].queuesData.pdqQueueLast;
             }
             cpssOsPrintf("\n|%3d|%7d|%6d|%12d|%10d|%6d|%16d|",i,pNodeIndex,isCascadePort?0:aNodeIndex,qNodeFirst,qNodeLast,remote,cascadePort);
           }
            cpssOsPrintf("\n+---+------+-------+------------+----------+------+----------------+");

            cpssOsPrintf("\nDMA to Pnode mapping - SW shadow");

            cpssOsPrintf("\n+---+------+-------+------------+");
            cpssOsPrintf("\n| DP |Port | P node| Global DMA |");
            cpssOsPrintf("\n+---+------+-------+------------+");

           for(k=0;k<8;k++)
           {
                for(j=0;j<9;j++)
                {
                       rc = prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget(devNum,tileNumber,k,j,&tmp,GT_FALSE);

                       if(rc == GT_OK)
                       {
                            rc = prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileNumber,
                                           k,j,&globalDmaNum);

                            if(tmp== pNodeIndex)
                            {
                               cpssOsPrintf("\n|%3d|%6d|%7d|%12d|",k,j,pNodeIndex,globalDmaNum);
                               cpssOsPrintf("\n+---+------+-------+------------+");
                            }

                            /*
                                                    if(j==8)
                                                    {
                                                           dpWithCpuPorts[i] = 1;
                                                           cpuPortExist = GT_TRUE;
                                                    }*/
                       }
                 }

                /*cpssOsPrintf("\n+---+------+-------+------------+");*/
           }

    }





/*
   cpssOsPrintf("\n CPU ports on DPs : %s",cpuPortExist==GT_FALSE?"NA\n":" ");

   if(cpuPortExist == GT_TRUE)
    {

       for(i=0;i<8;i++)
        {
            if(dpWithCpuPorts[i])
            {
                cpssOsPrintf("%d  |",i);
            }
        }

       cpssOsPrintf("\n");
    }
*/
   return GT_OK;


}

/**
* @internal prvCpssFalconTxqUtilsSdqQueueAttributesDump function
* @endinternal
*
* @brief   Debug function that dump queue attributes
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    PP's device number.
* @param[in]  tileNum                    tile number.(APPLICABLE RANGES:0..3)
* @param[in] sdqNum                    global SDQ number (APPLICABLE RANGES:0..31)
* @param[in] startQ                     -queue to start from in the dump
* @param[in] size                     -     number of queues to dump
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsSdqQueueAttributesDump
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
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES  queueAttributes;
    GT_U32 balance;
    GT_U32 eligState;
    GT_BOOL sp;

    cpssOsPrintf("\nQueueAttributes SDQ %d\n",sdqNum);

    cpssOsPrintf("\n+-------------+------+--+--------+--------+--------+--------+--------+--+");
    cpssOsPrintf("\n| queueNumber |enable|tc|  high  |  low   |  neg   |current | eligSt |SP|");
    cpssOsPrintf("\n+-------------+------+--+--------+--------+--------+--------+--------+--+");

    for(i =startQ;i<(startQ+size);i++)
    {
        rc = prvCpssFalconTxqSdqQueueAttributesGet(devNum,tileNumber,sdqNum,i,&queueAttributes);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n",rc );
            return rc;
        }

      rc = prvCpssFalconTxqSdqQCreditBalanceGet(devNum,tileNumber,sdqNum,i,&balance);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n",rc );
            return rc;
        }

       rc = prvCpssFalconTxqSdqEligStateGet(devNum,tileNumber,sdqNum,i,&eligState);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n",rc );
            return rc;
        }

        rc = prvCpssFalconTxqSdqQueueStrictPriorityGet(devNum,tileNumber,sdqNum,i,&sp);
         if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueStrictPriorityGet\n",rc );
            return rc;
        }

        cpssOsPrintf("\n|%13d|%6d|%2d|%8d|%8d|%8d|%8d|%8d|%2d|",i,queueAttributes.enable,queueAttributes.tc,
                queueAttributes.highCreditThreshold,queueAttributes.lowCreditTheshold,queueAttributes.negativeCreditThreshold,balance,eligState,sp);
    }

  cpssOsPrintf("\n+-------------+------+--+--------+--------+--------+--------+--------+--+\n");

    return GT_OK;


}

/**
* @internal prvCpssFalconTxqUtilsPsiConfigDump function
* @endinternal
*
* @brief   Debug function that dump PDQ queue to SDQ/PDS mapping
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] psiNum                   - tile number (APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsPsiConfigDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum
)
{
    GT_U32           i;
    GT_U32           pdqLowQueueNum,localPortNum;
    GT_STATUS        rc;

     cpssOsPrintf("\nPSI sdq to pdq map table \n");

    cpssOsPrintf("\n+-----+--------------+-----------------+");
    cpssOsPrintf("\n| SDQ | localPortNum | queue   offset  |");
    cpssOsPrintf("\n+-----+--------------+-----------------+");

    for(i = 0;i<MAX_DP_IN_TILE(devNum);i++)
    {
        for(localPortNum = 0;localPortNum<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;localPortNum++)
        {
            rc = prvCpssFalconPsiMapPortToPdqQueuesGet(devNum,psiNum,i,localPortNum,&pdqLowQueueNum);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }

            cpssOsPrintf("\n|%5d|%14d|%17d|",i,localPortNum,pdqLowQueueNum);
        }


    }

    cpssOsPrintf("\n+-----+----------------+----------------+-------------+");

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsPdxTablesDump function
* @endinternal
*
* @brief   Debug function that dump PDX routing table and PDX pizza arbiter
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*                                      psiNum       - tile number (APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsPdxTablesDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 pdxNum
)
{
    GT_U32           i,j;
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC txPortMap;
    GT_STATUS        rc;
    GT_U32       activeSliceNum;
    GT_U32       activeSliceMap[40];
    GT_BOOL   sliceValid[40];

    cpssOsPrintf("\nPDX routing table \n");

    cpssOsPrintf("\n+-----------------+-----+------+----------+-------------------+");
    cpssOsPrintf("\n| queueGroupIndex | PDX | PDS  | queueBase| dpCoreLocalTrgPort|");
    cpssOsPrintf("\n+-----------------+-----+------+----------+-------------------+");

    for(i = 0;i<128;i++)
    {
        rc = prvCpssDxChTxqFalconPdxQueueGroupMapGet(devNum,i,&txPortMap);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        cpssOsPrintf("\n|%17d|%5d|%6d|%10d|%19d|",i,
            txPortMap.queuePdxIndex,txPortMap.queuePdsIndex,txPortMap.queueBase,txPortMap.dpCoreLocalTrgPort);
    }

     cpssOsPrintf("\n+-----------------+-----+------+----------+-------------------+\n\n\n");


  cpssOsTimerWkAfter(50);


    cpssOsPrintf("\nPDX Pizza arbiter\n");

    cpssOsPrintf("\n+----+-----+-------+-----+----------+");
    cpssOsPrintf("\n|PDX | PDS | slice |valid|assignment|");
    cpssOsPrintf("\n+----+-----+-------+-----+----------+");



    for(i = 0;i<8;i++)
    {
        rc = prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet(devNum,pdxNum,i,&activeSliceNum,activeSliceMap,sliceValid);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        for(j=0;j<activeSliceNum;j++)
        {
            cpssOsPrintf("\n|%4d|%5d|%7d|%5s|%10c|",pdxNum,i,j,(sliceValid[j]==GT_TRUE)?"true":"false",(sliceValid[j]==GT_TRUE)?'0'+activeSliceMap[j]:' ');
        }

        cpssOsPrintf("\n+----+-----+-------+-----+----------+");



    }



    return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsSdqPortRangeDump function
* @endinternal
*
* @brief   Debug function that dump mapping of local port to queues at SDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     PP's device number.
* @param[in] tileNumber                tile number (APPLICABLE RANGES:0..3)
* @param[in] sdqNum                   -global SDQ number (APPLICABLE RANGES:0..31)
* @param[in] portSpecific               if equal GT_TRUE then print range for specific local port,else print for all ports.
* @param[in] port                   -       local port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsSdqPortRangeDump
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNumber,
    IN  GT_U32  sdqNum,
    IN  GT_BOOL portSpecific,
    IN  GT_U32  port
)

{
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE  range;
    GT_U32           i;
    GT_STATUS        rc;
    GT_BOOL          enable;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES   portAttributes;

    cpssOsPrintf("\nPortRange and BP thresholds in SDQ %d\n",sdqNum);


    cpssOsPrintf("\n+-----------+-------+---+----+-----------+------------+");
    cpssOsPrintf("\n| localPort |enable |low|high|BP high th | BP low th  |");
    cpssOsPrintf("\n+-----------+-------+---+----+-----------+------------+");

    for(i=portSpecific?port:0;i<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;i++)
    {
         rc = prvCpssFalconTxqSdqLocalPortEnableGet(devNum,tileNumber,sdqNum,i,&enable);
         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

         }

         rc = prvCpssFalconTxqSdqLocalPortQueueRangeGet(devNum,tileNumber,sdqNum,i,&range);

         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

         }

         rc = prvCpssFalconTxqSdqPortAttributesGet(devNum,tileNumber,sdqNum,i,&portAttributes);

         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPortAttributesGet  failed for port %d  ",i);
         }

         cpssOsPrintf("\n|%11d|%7d|%3d|%4d|%11d|%12d|",i,enable,range.lowQueueNumber,range.hiQueueNumber,
                portAttributes.portBackPressureHighThreshold,portAttributes.portBackPressureLowThreshold);

         if(GT_TRUE == portSpecific)
           break;

    }

   cpssOsPrintf("\n+-----------+-------+---+----+-----------+------------+");
   cpssOsPrintf("\n");

   return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsGetShapingParameters function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and read shaping parameters from specific node(SW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] burstSizePtr             - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[out] maxRatePtr               -(pointer to) the requested shaping Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsGetShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    OUT    GT_U16    * burstSizePtr,
    OUT    GT_U32    *maxRatePtr
)
{
    GT_U32 tileNum;
    GT_U32 aNodeIndex,qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  * aNodePtr;


    GT_STATUS rc =GT_OK;

    rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,physicalPortNum,&tileNum,&aNodeIndex);

     if(rc!=GT_OK)
      {
          return rc;
      }



     if(perQueue == GT_FALSE)
       {
            rc = prvCpssFalconTxqPdqGetShapingOnNode(devNum,tileNum,aNodeIndex,GT_FALSE,burstSizePtr,maxRatePtr);

            if(rc)
             {
                return rc;
              }
        }
      else
       {
            /*Find the queue index*/
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNum,&aNodePtr);

             if(rc)
             {
                return rc;
              }

            qNodeIndex = aNodePtr->queuesData.pdqQueueFirst+queueNumber;

            rc = prvCpssFalconTxqPdqGetShapingOnNode(devNum,tileNum,qNodeIndex,GT_TRUE,burstSizePtr,maxRatePtr);

        }

    return rc;



}


/**
* @internal prvCpssFalconTxqUtilsSetShapingParameters function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and write shaping parameters to specific node
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
* @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsSetShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN    GT_U16    burstSize,
    INOUT GT_U32    *maxRatePtr
)
{
    GT_U32 tileNum;
    GT_U32 aNodeIndex,qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  * aNodePtr;


    GT_STATUS rc =GT_OK;



    rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,physicalPortNum,&tileNum,&aNodeIndex);

     if(rc!=GT_OK)
      {
          return rc;
      }

     if(perQueue == GT_FALSE)
       {
            rc = prvCpssFalconTxqPdqSetShapingOnNode(devNum,tileNum,aNodeIndex,GT_FALSE,burstSize,maxRatePtr);

            if(rc)
             {
                return rc;
              }
        }
      else
       {
            /*Find the queue index*/
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNum,&aNodePtr);

             if(rc)
             {
                return rc;
              }

            qNodeIndex = aNodePtr->queuesData.pdqQueueFirst+queueNumber;

            rc = prvCpssFalconTxqPdqSetShapingOnNode(devNum,tileNum,qNodeIndex,GT_TRUE,burstSize,maxRatePtr);

            if(rc)
             {
                return rc;
              }
        }

    return rc;



}


/**
* @internal prvCpssFalconTxqUtilsSetShapingEnable function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsSetShapingEnable
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN    GT_BOOL    enable
)
{
    GT_U32 tileNum;
    GT_U32 aNodeIndex,qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  * aNodePtr;


    GT_STATUS rc =GT_OK;

        /*chek that port is mapped*/
     rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,physicalPortNum,&tileNum,&aNodeIndex);

     if(rc!=GT_OK)
      {
          return rc;
      }


     if(perQueue == GT_FALSE)
       {
            rc = prvCpssFalconTxqPdqSetShapingEnable(devNum,tileNum,aNodeIndex,GT_FALSE,enable);

            if(rc)
             {
                return rc;
              }
        }
      else
       {
            /*Find the queue index*/
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNum,&aNodePtr);

             if(rc)
             {
                return rc;
              }

            qNodeIndex = aNodePtr->queuesData.pdqQueueFirst+queueNumber;

             rc = prvCpssFalconTxqPdqSetShapingEnable(devNum,tileNum,qNodeIndex,GT_TRUE,enable);

            if(rc)
             {
                return rc;
              }
        }







    return rc;



}
/**
* @internal prvCpssFalconTxqUtilsGetShapingEnable function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and get Enable/Disable Token Bucket rate shaping on specified port or queue of specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] enablePtr                - GT_TRUE, enable Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsGetShapingEnable
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN  GT_BOOL  *  enablePtr
)
{
    GT_U32 tileNum;
    GT_U32 aNodeIndex,qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  * aNodePtr;


    GT_STATUS rc =GT_OK;


    rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,physicalPortNum,&tileNum,&aNodeIndex);

     if(rc!=GT_OK)
      {
          return rc;
      }



     if(perQueue == GT_FALSE)
       {
            rc = prvCpssFalconTxqPdqGetShapingEnable(devNum,tileNum,aNodeIndex,GT_FALSE,enablePtr);

            if(rc)
             {
                return rc;
              }
        }
      else
       {
            /*Find the queue index*/
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNum,&aNodePtr);

             if(rc)
             {
                return rc;
              }

            qNodeIndex = aNodePtr->queuesData.pdqQueueFirst+queueNumber;

             rc = prvCpssFalconTxqPdqGetShapingEnable(devNum,tileNum,qNodeIndex,GT_TRUE,enablePtr);

            if(rc)
             {
                return rc;
              }
        }






    return rc;



}

/**
* @internal prvCpssFalconTxqUtilsPortTxBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
*                                      portNum  - physical or CPU port number
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/


GT_STATUS prvCpssFalconTxqUtilsPortTxBufNumberGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_U32 * numPtr
)
{
         GT_STATUS rc ;

        GT_U32 tileNum,dpNum,localPortNum;
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;

        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,physicalPortNum,&tileNum,&dpNum,&localPortNum,&mappingType);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        if(mappingType!= CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
         { /*local or CPU */

            rc = prvCpssFalconTxqQfcBufNumberGet(devNum,tileNum,dpNum,localPortNum,GT_TRUE,numPtr);

               if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqQfcBufNumberGet");
                }
         }
        else
         {
            /*we do not have counters for remote port.So return GT_NOT_SUPPORTED*/

             rc = GT_NOT_SUPPORTED;

         }

        return rc;
}

/**
* @internal prvCpssFalconTxqUtilsQueueBufPerTcNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class (0..7)
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsQueueBufPerTcNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8       trafClass,
    OUT GT_U16      *numPtr
)
{
    GT_U32 tileNum,dpNum,localPortNum,queueNum,counter =0;
    GT_STATUS rc ;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr;


    /*Find tile*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localPortNum,NULL);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);

    if(rc !=GT_OK)
    {
        return rc;
    }

    queueNum = aNodePtr->queuesData.queueBase;

    *numPtr=0;

    queueNum = aNodePtr->queuesData.queueBase + trafClass;


    rc = prvCpssFalconTxqQfcBufNumberGet(devNum,tileNum,dpNum,queueNum,GT_FALSE,&counter);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqQfcQueueBufNumberGet");
    }

    *numPtr =(GT_U16)counter;

     return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsQueueEnableSet function
* @endinternal
*
* @brief   Enable/disable queue
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical or CPU port number.
* @param[in] tcQueue                  - traffic class queue on this device (0..7).
* @param[in] enable                   - if GT_TRUE queue  is enabled , otherwise disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsQueueEnableSet
(
     IN GT_U8 devNum,
     IN  GT_PHYSICAL_PORT_NUM portNum,
     IN  GT_U8     tcQueue,
     IN GT_BOOL enable
)
{
        GT_U32 tileNum,dpNum,localPortNum;

        GT_STATUS rc ;

        /*Find tile*/
        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localPortNum,NULL);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }


        rc = prvCpssFalconTxqSdqQueueEnableSet(devNum,tileNum,dpNum,localPortNum,tcQueue,enable);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqSdqQueueEnableSet");
        }

         return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/disable queue status
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] portNum                  - physical or CPU port number.
* @param[in] tcQueue                  - traffic class queue on this device (0..7).
*
* @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsQueueEnableGet
(
     IN GT_U8 devNum,
     IN  GT_PHYSICAL_PORT_NUM portNum,
     IN  GT_U8     tcQueue,
     IN GT_BOOL * enablePtr
)
{
        GT_U32 tileNum,dpNum,localPortNum;

        GT_STATUS rc ;

        /*Find tile*/
        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localPortNum,NULL);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }


        rc = prvCpssFalconTxqSdqQueueEnableGet(devNum,tileNum,dpNum,localPortNum,tcQueue,enablePtr);

       if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqSdqQueueEnableGet");
        }

         return GT_OK;
}


/**
* @internal prvCpssFalconTxqUtilsSetDmaToPnodeMapping function
* @endinternal
 *
* @brief    Write entry to DMA to Pnode data base
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -  device number
* @param[in] dmaNum                  - Global DMA number(0..263).
* @param[in] tileNum                  - traffic class queue on this device (0..7).
* @param[in] pNodeNum                  - Index of P node
*/

static GT_STATUS prvCpssFalconTxqUtilsSetDmaToPnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  dmaNum,
     IN  GT_U32   tileNum,
     IN  GT_U32   pNodeNum
)
{

       TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

       if((dmaNum >= PRV_CPSS_MAX_DMA_NUM_CNS) ||pNodeNum>=CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM)
        {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Fail in prvCpssFalconTxqUtilsSetDmaToPnodeMapping");
        }



       PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].pNodeInd = pNodeNum;
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].tileInd = tileNum;

       return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsGetDmaToPnodeMapping function
* @endinternal
 *
* @brief    Read entry from DMA to Pnode data base
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -  device number
* @param[in] dmaNum                  - Global DMA number(0..263).
* @param[in] tileNum                  - traffic class queue on this device (0..7).
* @param[out] pNodeNum                  - Index of P node
* @param[in] isErrorForLog            - indication the CPSS ERROR should be in the LOG or not

*/

static GT_STATUS prvCpssFalconTxqUtilsGetDmaToPnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  dmaNum,
     OUT  GT_U32   *tileNumPtr,
     OUT GT_U32   * pNodeNumPtr,
    IN GT_BOOL isErrorForLog
)
{
    if(dmaNum >= PRV_CPSS_MAX_DMA_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "dmaNum[%d] >= 'max'[%d]",
            dmaNum,PRV_CPSS_MAX_DMA_NUM_CNS);
    }

    if(tileNumPtr)
    {
        *tileNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].tileInd ;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].pNodeInd == CPSS_PDQ_SCHED_INVAL_DATA_CNS)
    {
        if(GT_FALSE == isErrorForLog)
        {
            return /* not error for the LOG */ GT_NOT_FOUND;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "dmaNum[%d] not hold valid pNodeInd",
                dmaNum);
        }
    }

    * pNodeNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].pNodeInd ;

    return GT_OK;

}



/**
* @internal prvCpssFalconTxqUtilsSetPhysicalNodeToAnodeMapping function
* @endinternal
 *
* @brief    Write entry to physical port to Anode index data base
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -  device number
* @param[in] dmaNum                  - Global DMA number(0..263).
* @param[in] tileNum                  - traffic class queue on this device (0..7).
* @param[in] aNodeNum                  - Index of A node

*/

static GT_STATUS prvCpssFalconTxqUtilsSetPhysicalNodeToAnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  physicalPortNumber,
     IN  GT_U32   tileNum,
     IN  GT_U32   aNodeNum
)
{
       GT_STATUS rc;
       CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

       TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

       rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNumber, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(portMapShadowPtr->valid == GT_FALSE)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

       PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].aNodeInd = aNodeNum;
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].tileInd = tileNum;

       return GT_OK;



}

/*******************************************************************************
* prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping
*
* DESCRIPTION:
*      Read entry from physical port to Anode index data base
*
* APPLICABLE DEVICES:
*        Falcon
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       devNum        -  device number
*       dmaNum       - Global DMA number(0..263).
*       tileNum         - traffic class queue on this device (0..7).
*
* OUTPUTS:
*      aNodeNumPtr   - Index of A node
*      tileNumPtr   - Index of tile
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

GT_STATUS prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  physicalPortNumber,
     OUT   GT_U32   * tileNumPtr,
     OUT   GT_U32   * aNodeNumPtr
)
{


       GT_STATUS rc;
       CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;


        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNumber, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
               return rc;
         }

           if(portMapShadowPtr->valid == GT_FALSE)
           {
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
           }


        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].tileInd  == CPSS_PDQ_SCHED_INVAL_DATA_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Fail in prvCpssFalconTxqUtilsSetDmaToPnodeMapping.Unexpected tile number");
        }


        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].aNodeInd == CPSS_PDQ_SCHED_INVAL_DATA_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Fail in prvCpssFalconTxqUtilsSetDmaToPnodeMapping.Unexpected tile number");
        }

       *aNodeNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].aNodeInd ;

        if(tileNumPtr)
        {
            *tileNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].tileInd;
        }

       return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb function
* @endinternal
 *
* @brief  Initialize physical port to A nodedatabase
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                  -  device number
*/

GT_VOID  prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb
(
    IN GT_U8 devNum
)
{

     GT_U32 i;

    for(i=0;i<PRV_CPSS_MAX_PP_PORTS_NUM_CNS ;i++)
    {
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[i].aNodeInd = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[i].tileInd = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
    }

}

/**
* @internal prvCpssFalconTxqUtilsGetQueueGroupAttributes function
* @endinternal
 *
* @brief  Get attributes of queue group
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] queueGroup           - queue group index
* @param[out] tilePtr                   - tile of the queue group
* @param[out] dpPtr  - dp of the queue group
* @param[out]  queueBaseInDpPtr - local port of the queue group
* @param[out] queueBaseInTilePtr - global queue index (per PDQ)
*/
GT_STATUS  prvCpssFalconTxqUtilsGetQueueGroupAttributes
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroup,
    OUT GT_U32 * tilePtr,
    OUT GT_U32 * dpPtr,
    OUT GT_U32 * localPortPtr,
    OUT GT_U32 * queueBaseInDpPtr,
    OUT GT_U32 * queueBaseInTilePtr
)
{
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC txPortMap;
    GT_STATUS rc;
    GT_U32 pNodeIndex,i;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    rc = prvCpssDxChTxqFalconPdxQueueGroupMapGet(devNum,queueGroup,&txPortMap);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in prvCpssFalconTxqUtilsGetQueueGroupAttributes" );
    }

    *tilePtr = txPortMap.queuePdxIndex;
    *dpPtr = txPortMap.queuePdsIndex;
    *localPortPtr = txPortMap.dpCoreLocalTrgPort;
        * queueBaseInDpPtr = txPortMap.queueBase;

    /*calculate P node index*/

    pNodeIndex = prvCpssFalconTxqUtilsGetPnodeIndex(devNum,*dpPtr, *localPortPtr);

    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[ *tilePtr ]);

    /*find A node*/

    for(i=0;i<tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeIndex].aNodeListSize;i++)
    {
        if(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeIndex].aNodelist[i].queuesData.queueGroupIndex == queueGroup)
        {
            *queueBaseInTilePtr = tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeIndex].aNodelist[i].queuesData.pdqQueueFirst;
            return GT_OK;
        }
    }

   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}




/**
* @internal prvCpssFalconTxqUtilsIsCascadePort function
* @endinternal
 *
* @brief  Check if port has queues mapped at TxQ
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[out] isCascadePtr  - if equal GT_TRUE this port is cascade,GT_FALSE otherwise

*/

GT_STATUS prvCpssFalconTxqUtilsIsCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_BOOL * isCascadePtr
)
{
        GT_STATUS rc;
        /*In order to avoid compiler warning initialize pNode*/
        GT_U32  pNode = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
        GT_U32   tileNum= 0;
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

       rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNum, /*OUT*/&portMapShadowPtr);

         if(rc!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
         }

       if(portMapShadowPtr->valid)
        {

            rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum,portMapShadowPtr->portMap.txDmaNum,&tileNum,&pNode,GT_TRUE);

             if(rc!=GT_OK)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
             }

             tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);

              *isCascadePtr=GT_FALSE;

             if(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNode].isCascade)
             {
                if(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNode].cascadePhysicalPort==physicalPortNum)
                {
                   *isCascadePtr=GT_TRUE;
                }
             }




             return GT_OK;

       }

       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);


}


GT_VOID prvCpssFalconTxqUtilsDefaultBindPortsToSchedDb
(
    IN GT_U8 devNum
)
{

     GT_U32 i;
     GT_U32 wordIndex;
     GT_U32 bitIndex;

    for(i=0;i<PRV_CPSS_MAX_PP_PORTS_NUM_CNS ;i++)
    {
       if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[i].aNodeInd != CPSS_PDQ_SCHED_INVAL_DATA_CNS)
       {
            wordIndex = i/32;
            bitIndex=i%32;

            /*bind  to the default profile*/
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[DEFAULT_SCHED_PROFILE].bindedPortsBmp[wordIndex]|=(1<<bitIndex);
       }
    }

}
/**
* @internal prvCpssFalconTxqUtilsDumpBindPortsToSchedDb function
* @endinternal
 *
* @brief   Print port to schedule profile binding
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
*/
GT_STATUS prvCpssFalconTxqUtilsDumpBindPortsToSchedDb
(
    IN GT_U8 devNum
)
{
    GT_U32 i,j,tmp,portNum,shift,num=0;


    for(i = 0;i<CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM ;i++)
    {
            cpssOsPrintf(" \nProfile %d binded ports: \n",i+1/*Profiles start from 1*/);

            for(j=0;j<32;j++)
            {
                tmp= PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[j];

                if(tmp )
                {
                        shift =0;

                        while(tmp)
                        {
                            if(tmp&0x1)
                            {
                                portNum = j*32+shift;

                                cpssOsPrintf(" %3d ,",portNum);
                                num++;

                                if(num==8)
                                {
                                    cpssOsPrintf(" \n",portNum);
                                    num=0;
                                }

                            }

                            tmp>>=1;
                            shift++;
                        }
                }
            }

    }

     return GT_OK;
}



static GT_STATUS prvCpssFalconTxqUtilsSyncPortToSchedulerProfile
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
)
{

       GT_U32  tileNum,aNodeIndex;
       GT_STATUS rc;
       PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr;
       PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr;
       GT_BOOL                          lowLatencyPresent;


      /*Find A-node index*/

       rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,physicalPortNum,&tileNum,&aNodeIndex);

       if(rc!=GT_OK)
        {
            return rc;
        }

       rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNum,&aNodePtr);

       if(rc!=GT_OK)
       {
            return rc;
       }

       rc = prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber(devNum,physicalPortNum,&pNodePtr);
       if(rc!=GT_OK)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber failed ");
       }

       rc = prvCpssFalconTxqUtilsLowLatencyQueuePresentInPnodeGet(devNum,pNodePtr,&lowLatencyPresent);
       if(rc!=GT_OK)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsLowLatencyQueuePresentInPnodeGet failed ");
       }

       return prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes(devNum,tileNum,aNodePtr,
         &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet]),lowLatencyPresent,
         pNodePtr->aNodelist[0].queuesData.queueBase,
         pNodePtr->aNodelist[0].queuesData.queueBase+pNodePtr->aNodelist[pNodePtr->aNodeListSize-1].queuesData.pdqQueueLast-
         pNodePtr->aNodelist[0].queuesData.pdqQueueFirst);
}


static GT_STATUS prvCpssFalconTxqUtilsSyncBindedPortsToSchedulerProfile
(
    IN GT_U8 devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
    )
{
    GT_U32 j;

    GT_STATUS rc;

    GT_U32 tmp,portNum,shift;

    for(j=0;j<32;j++)
    {
        tmp= PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].bindedPortsBmp[j];

        if(tmp)
        {
            shift =0;

            while(tmp)
            {
                if(tmp&0x1)
                {
                    portNum = j*32+shift;

                    rc = prvCpssFalconTxqUtilsSyncPortToSchedulerProfile(devNum,portNum,profileSet);

                    if(rc !=GT_OK)
                    {
                        return rc;
                    }
                }

                tmp>>=1;
                shift++;
            }
        }
    }

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
* @endinternal
 *
* @brief  Bind a port to scheduler profile set.
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[in] profileSet  -The Profile Set in which the scheduler's parameters are
*                                      associated.
*/
GT_STATUS prvCpssFalconTxqUtilsBindPortToSchedulerProfile
    (
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
    )
{
    GT_U32 wordIndex,i;
    GT_U32 bitIndex;
    GT_STATUS rc;


    /*check if mapped*/

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNum].aNodeInd == CPSS_PDQ_SCHED_INVAL_DATA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Unmapped port number %d\n",physicalPortNum);
    }

    wordIndex = physicalPortNum/32;
    bitIndex=physicalPortNum%32;

    /*First delete from old profile shadow*/
    for(i = 0;i<CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM ;i++)
    {
        if( (PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[wordIndex])&(1<<bitIndex))
        {
            if(i== (GT_U32)profileSet)
            {
                /*The port is already binded to this profile*/
                return GT_OK;
            }

            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[wordIndex]&=~((1<<bitIndex));
            break;
        }
    }



    /*Now update new profile shadow*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].bindedPortsBmp[wordIndex]|=((1<<bitIndex));

    /*Now update HW*/

    rc = prvCpssFalconTxqUtilsSyncPortToSchedulerProfile(devNum,physicalPortNum,profileSet);

    return rc;

}
/**
* @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
* @endinternal
 *
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[out] profileSetPtr  -The Profile Set in which the scheduler's parameters are
*                                      associated.
*/
GT_STATUS prvCpssFalconTxqUtilsPortSchedulerProfileGet
(
  IN GT_U8 devNum,
  IN GT_U32 physicalPortNum,
  OUT CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  * profileSetPtr
)
{
    GT_U32 wordIndex,i;
    GT_U32 bitIndex;
    GT_STATUS rc;


    /*check if mapped*/

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNum].aNodeInd == CPSS_PDQ_SCHED_INVAL_DATA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Unmapped port number %d\n",physicalPortNum);
    }

    wordIndex = physicalPortNum/32;
    bitIndex=physicalPortNum%32;


    for(i = 0;i<CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM ;i++)
    {
        if( (PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[wordIndex])&(1<<bitIndex))
        {
            break;
        }
    }

    if(i == CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM)
    {
        rc = GT_NOT_FOUND;
    }
    else
    {
        rc = GT_OK;
        *profileSetPtr = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E+i;
    }

    return rc;

}


/**
* @internal prvCpssFalconTxqUtilsWrrProfileSet function
* @endinternal
 *
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this Arbitration Group
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*/
GT_STATUS prvCpssFalconTxqUtilsWrrProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  GT_U8                                   wrrWeight,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
)
{


        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].weights[tcQueue] == wrrWeight*CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS)
        {
            return GT_OK;
        }

        /*Update SW shadow*/

        PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].weights[tcQueue] = wrrWeight*CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;

        /*Update HW*/

        return  prvCpssFalconTxqUtilsSyncBindedPortsToSchedulerProfile(devNum,profileSet);

}

/**
* @internal prvCpssFalconTxqUtilsWrrProfileGet function
* @endinternal
 *
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*
* @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this  Arbitration Group
*/
GT_STATUS prvCpssFalconTxqUtilsWrrProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT  GT_U8                                  *wrrWeightPtr
)
{
     *wrrWeightPtr = (GT_U8)PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].weights[tcQueue] /CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;

     return GT_OK;
}
/**
* @internal prvCpssFalconTxqUtilsArbGroupSet function
* @endinternal
 *
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] arbGroup                 - scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*/
GT_STATUS prvCpssFalconTxqUtilsArbGroupSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_Q_ARB_GROUP_ENT            arbGroup,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
)
{
       GT_U32  * wrrGroupBmpPtr  =  & (PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp);

        if(arbGroup == CPSS_PORT_TX_SP_ARB_GROUP_E)
        {
              if((PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp &(1<<tcQueue))== 0)
              {
                 /*Already configured*/
                  return GT_OK;
              }

              /*Move from WRR to SP*/
              PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp&=~(1<<tcQueue);

               /*We have new low latency queue*/
              if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].lowLatencyQueue<tcQueue)
              {
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].lowLatencyQueue = tcQueue;
              }

         }
        else
        {
              if((PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp &(1<<tcQueue))== ((GT_U32)1<<tcQueue))
              {

                     if(arbGroup == CPSS_PORT_TX_WRR_ARB_GROUP_0_E)
                     {
                               if((PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp &(1<<tcQueue))==0)
                                {
                                /*Already configured*/
                                    return GT_OK;
                               }

                               /*Move from WRR1 to WRR0*/
                                PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp&=~(1<<tcQueue);
                     }
                    else
                    {
                             if(((*wrrGroupBmpPtr) &(1<<tcQueue))==1)
                                {
                                /*Already configured*/
                                    return GT_OK;
                               }

                             /*Move from WRR0 to WRR1*/
                              (*wrrGroupBmpPtr)|=(1<<tcQueue);
                    }

              }

              /*Move from SP to WRR*/
              PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp|=(1<<tcQueue);

              if(arbGroup == CPSS_PORT_TX_WRR_ARB_GROUP_0_E)
              {
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp&=~(1<<tcQueue);
              }
              else
              {
                 PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp|=(1<<tcQueue);
              }
        }

             /*Update HW*/

        return  prvCpssFalconTxqUtilsSyncBindedPortsToSchedulerProfile(devNum,profileSet);

}

/**
* @internal prvCpssFalconTxqUtilsArbGroupGet function
* @endinternal
 *
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*/

GT_STATUS prvCpssFalconTxqUtilsArbGroupGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT CPSS_PORT_TX_Q_ARB_GROUP_ENT            *arbGroupPtr
)
{
     GT_U32 wrrGroupBmp ;
     wrrGroupBmp = PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp;

     if((PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp &(1<<tcQueue))== 0)
      {
            *arbGroupPtr = CPSS_PORT_TX_SP_ARB_GROUP_E;
      }
     else if(wrrGroupBmp &(1<<tcQueue))
     {
            *arbGroupPtr = CPSS_PORT_TX_WRR_ARB_GROUP_1_E;
     }
     else
     {
             *arbGroupPtr = CPSS_PORT_TX_WRR_ARB_GROUP_0_E;
     }

      return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsDumpBindPortsToSchedDb function
* @endinternal
 *
* @brief   Print  schedule profile attributes
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
*/
GT_STATUS prvCpssFalconTxqUtilsDumpSchedDbProfiles
(
    IN GT_U8 devNum
)
{

      GT_U32 i,tcQueue;

       cpssOsPrintf("+----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n");
       cpssOsPrintf("|  profile |     TC0   |    TC1    |    TC2    |    TC3    |    TC4    |    TC5    |    TC6    |    TC7    |\n");
       cpssOsPrintf("|          | [Grp/Wgt] | [Grp/Wgt] | [Grp/Wgt] | [Grp/Wgt] | [Grp/Wgt] | [Grp/Wgt] | [Grp/Wgt] | [Grp/Wgt] |\n");
       cpssOsPrintf("+----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n");

      for(i = 0;i<CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM ;i++)
      {
            cpssOsPrintf("|   %2d     ",i+1/*profile start from 1*/);

            for(tcQueue = 0; tcQueue < 8; tcQueue++)
            {
                if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].wrrEnableBmp &( 1<<tcQueue))
                {
                     cpssOsPrintf("|  %2d/%3d   ", PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].wrrGroupBmp&(1<<tcQueue)?1:0,
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].weights[tcQueue]/CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS);
                }
                else
                {
                    cpssOsPrintf("|  SP/NA    ");
                }
            }

          cpssOsPrintf("|\n+----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n");
      }

      return GT_OK;
}

/**
* @internal prvCpssFalconTxqGlobalTxEnableSet function
* @endinternal
 *
* @brief   Enable/Disable transmission of specified device.
*                Set enable/disable on all tiles (1/2/4)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
*/
GT_STATUS   prvCpssFalconTxqGlobalTxEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
        GT_U32 numberOfTiles,i;
        GT_STATUS rc;

        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            case 0:
            case 1:
                numberOfTiles = 1;
                break;
           case 2:
           case 4:
                numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d  ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                break;

        }

        for(i=0;i<numberOfTiles;i++)
        {
                /*set all tiles to same value*/
                rc = prvCpssFalconTxqPdqGlobalTxEnableSet(devNum,i,enable);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqGlobalTxEnableSet failed for tile - %d",i);
                }
        }

        return GT_OK;

}

/**
* @internal prvCpssFalconTxqGlobalTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*                Note this function check the status in all availeble tiles (The status should be the SAME)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS   prvCpssFalconTxqGlobalTxEnableGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  * enablePtr
)
{
        GT_U32 numberOfTiles,i;
        GT_STATUS rc;
        GT_BOOL enablePerTile[4];

        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            case 0:
            case 1:
                numberOfTiles = 1;
                break;
           case 2:
           case 4:
                numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d  ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                break;

        }

        for(i=0;i<numberOfTiles;i++)
        {
                /*read from all tiles */
                rc = prvCpssFalconTxqPdqGlobalTxEnableGet(devNum,i,enablePerTile+i);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGlobalTxEnableGet failed for tile - %d",i);
                }
        }

        /*check that all tiles are synced */
        for(i=1;i<numberOfTiles;i++)
        {
               if(enablePerTile[i]!=enablePerTile[i-1])
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " GlobalTxEnable has different values at tile %d  and  tile - %d",i,i-1);
                }
        }

        *enablePtr = *enablePerTile;

        return GT_OK;

}
/**
* @internal prvCpssDxChTxqFalconDescCounterDump function
* @endinternal
*
* @brief   Print TxQ descriptor counters for specific tile and PDS :
*                1. Per port decriptor count at PDX PAC unit (Port numbering is local to PDS).
*                2. Total PDS decriptor count.
*                3. Per queue decriptor count
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                   - tile number (0..3)
* @param[in] pdsNum                   - pds number  (0..7)
* @param[in] queueStart                 - first queue to dump number  (0..399)
* @param[in] queueEnd                   - last queue to dump number  (0..399)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconDescCounterDump
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd
)
{
        GT_U32 pdxPacDescCount;
        GT_U32 pdsDescCount,pdsDescCacheCount;
        GT_U32  i,total;
        GT_STATUS rc;
        PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC  perQueueDescCount;


        cpssOsPrintf("\nDescriptor counters for PDX %d PDS %d  \n",tileNum,pdsNum);

        cpssOsPrintf("\n+------+--------------------+");
        cpssOsPrintf("\n| Port | PDX PAC desc count |");
        cpssOsPrintf("\n+------+--------------------+");

        for(i=0;i<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;i++)
        {
                rc = prvCpssDxChTxqFalconPdxPacPortDescCounterGet(devNum,tileNum,CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC*pdsNum+i,&pdxPacDescCount);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdxPacPortDescCounterGet  failed for pdx  %d port %d ",tileNum,i);
                }

               cpssOsPrintf("\n|%6d|%20d|",i,pdxPacDescCount);
               cpssOsPrintf("\n+------+--------------------+");

        }

        cpssOsPrintf("\n+----+------------+----------------------------+");
        cpssOsPrintf("\n| PDS| desc total | desc cache(not include PB) |");
        cpssOsPrintf("\n+----+------------+----------------------------+");


        rc = prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet(devNum,tileNum,pdsNum,&pdsDescCount);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet  failed for tile  %d pds %d ",tileNum,pdsNum);
        }

        rc = prvCpssDxChTxqFalconPdsCacheDescCounterGet(devNum,tileNum,pdsNum,&pdsDescCacheCount);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsCacheDescCounterGet  failed for tile  %d pds %d ",tileNum,pdsNum);
        }

        cpssOsPrintf("\n|%4d|%12d|%28d|",pdsNum,pdsDescCount,pdsDescCacheCount);
        cpssOsPrintf("\n+----+------------+----------------------------+\n");



        cpssOsPrintf("\n+----+------------+------------+------------+------------+------------+------------+");
        cpssOsPrintf("\n| Q  |tail counter|frag counter|head counter|frag   index|long  queue |total decs  |");
        cpssOsPrintf("\n+----+------------+------------+------------+------------+------------+------------+");

        for(i=queueStart; (i<=queueEnd) && (i<CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC);i++)
        {
                rc = prvCpssDxChTxqFalconPdsPerQueueCountersGet(devNum,tileNum,pdsNum,i,&perQueueDescCount);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet  failed for tile  %d pds %d ",tileNum,i);
                }

                total = perQueueDescCount.headCounter;

                if(GT_TRUE == perQueueDescCount.longQueue)
                {
                    total +=perQueueDescCount.tailCounter+(16*perQueueDescCount.fragCounter);
                }


               cpssOsPrintf("\n|%4d|%12d|%12d|%12d|%12d|%12d|%12d|",i,perQueueDescCount.tailCounter,
                        perQueueDescCount.fragCounter,perQueueDescCount.headCounter,perQueueDescCount.fragIndex,perQueueDescCount.longQueue,total);

               cpssOsPrintf("\n+----+------------+------------+------------+------------+------------+------------+");

        }

        cpssOsPrintf("\n");

        return GT_OK;
}


GT_STATUS prvCpssDxChTxqFalconDumpSchedErrors
(
    IN GT_U8 devNum
)
{
        GT_U32 numberOfTiles,i;
        GT_STATUS rc;
        GT_U32    errorCounter;
        GT_U32    exceptionCounter;
        GT_U32    errorStatus;


        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            case 0:
            case 1:
                numberOfTiles = 1;
                break;
           case 2:
           case 4:
                numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d  ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                break;

        }

        cpssOsPrintf("\n+------+----------+--------------+-------------+");
        cpssOsPrintf("\n| Tile | Error cnt|Exception cnt |Error status |");
        cpssOsPrintf("\n+------+----------+--------------+-------------+");


        for(i=0;i<numberOfTiles;i++)
        {
             rc = prvCpssFalconTxqPdqGetErrorStatus(devNum,i,&errorCounter,&exceptionCounter,&errorStatus);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetErrorStatus  failed for tile  %d  ",i);
                }

                cpssOsPrintf("\n|%6d|%10d|%14d|%13d|",i,errorCounter,exceptionCounter,errorStatus);
                cpssOsPrintf("\n+------+----------+--------------+-------------+");
        }

        cpssOsPrintf("\n");



 cpssOsPrintf("\n\nError meaning :\n\n");
 cpssOsPrintf("FIFOOvrflowErr (Bit 5)- Tm to TM backpressure FIFO has over flown.Backpressure Xon/Xoff indication lost.\n");
 cpssOsPrintf("TBNegSat (Bit 4) - A Token Bucket level has reached its maximal negative value and is saturated at this value.\n");
 cpssOsPrintf("BPBSat (Bit 3)- Byte Per Burst value (internal parameter used for Port DWRR) is saturated.\n");
 cpssOsPrintf("UncECCErr(Bit 2) - Two or more bits were corrupted in internal memory protected by ECC and could not be corrected\n");
 cpssOsPrintf("CorrECCErr(Bit 1) - One corrupted bit in internal memory was detected and corrected by ECC.\n");
 cpssOsPrintf("ForcedErr(Bit 0) - A forced error was ordered by writing to the Force Error configuration register.\n");

 return GT_OK;

}


GT_STATUS prvCpssDxChTxqDumpQueueStatus
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd
)
{
        GT_U32 i;
        GT_U32 rc;
        GT_32     level;
        GT_U32   dificit;

        cpssOsPrintf("Level - Bucket level field is a signed value in bytes. Min Token Bucket is Conforming if Value greater than 0.\n");
        cpssOsPrintf("MinLvl[bit 22] - Read Error. Set in case read is failed.  \n\n\n");


        cpssOsPrintf("\n+----+-------+-------+");
        cpssOsPrintf("\n| Q  | level |dificit|");
        cpssOsPrintf("\n+----+-------+-------+");

        for(i=queueStart; (i<=queueEnd) && (i<CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM);i++)
        {
            rc = prvCpssFalconTxqPdqGetQueueStatus(devNum,tileNum,i,&level,&dificit);

             if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueStatus  failed for tile  %d  ",i);
            }

             cpssOsPrintf("\n|%4d|%7d|%7d|",i,level,dificit);

            cpssOsPrintf("\n+----+-------+-------+");

        }

        cpssOsPrintf("\n");

        return GT_OK;

}


GT_STATUS prvCpssDxChTxqDumpAlevelStatus
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 alevelStart,
    IN GT_U32 alevelEnd
)
{
        GT_U32 i;
        GT_U32 rc;
        GT_32     level;
        GT_U32   dificit;

        cpssOsPrintf("Level - Bucket level field is a signed value in bytes. Min Token Bucket is Conforming if Value greater than 0.\n");
        cpssOsPrintf("MinLvl[bit 22] - Read Error. Set in case read is failed.  \n\n\n");


        cpssOsPrintf("\n+----+-------+-------+");
        cpssOsPrintf("\n| A  | level |dificit|");
        cpssOsPrintf("\n+----+-------+-------+");

        for(i=alevelStart; (i<=alevelEnd) && (i<CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC);i++)
        {
            rc = prvCpssFalconTxqPdqGetAlevelStatus(devNum,tileNum,i,&level,&dificit);

             if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueStatus  failed for tile  %d  ",i);
            }

             cpssOsPrintf("\n|%4d|%7d|%7d|",i,level,dificit);

            cpssOsPrintf("\n+----+-------+-------+");

        }

        cpssOsPrintf("\n");

        return GT_OK;

}

GT_STATUS prvCpssDxChTxqDumpQueueShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd
)
{
        GT_U32 i;
        GT_U32 rc;
        GT_U32    divExp;
        GT_U32    token;
        GT_U32    res;
        GT_U32    burst;
        GT_BOOL shapingEnabled;
        GT_U32     shaperDec;
        GT_U32     perInter;
        GT_U32     perEn,bw = 0;
        double tmp;



       rc = prvCpssFalconTxqPdqGetPerLevelShapingParameters(devNum,tileNum,GT_TRUE,&shaperDec,&perInter,&perEn);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetPerLevelShapingParameters  failed  ");
        }

        cpssOsPrintf("\nQ level parameters:\n\nPeriodic interval %d\n\nshaperDec %d\n\nperEn %d\n",perInter,shaperDec,perEn);

        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");
        cpssOsPrintf("\n| Q  |shp| divExp|token  |res  | burst |BW in Kbit |");
        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        for(i=queueStart; (i<=queueEnd) && (i<CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM);i++)
        {
            rc = prvCpssFalconTxqPdqGetShapingEnable(devNum,tileNum,i,GT_TRUE,&shapingEnabled);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetShapingEnable  failed for q  %d  ",i);
            }

            rc = prvCpssFalconTxqPdqGetQueueShapingParameters(devNum,tileNum,i,&divExp,&token,&res,&burst);

             if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueShapingParameters  failed for q  %d  ",i);
            }

            tmp =(double) ( token*(1<<res))/(perInter*(1<<divExp));
            bw = (GT_U32)((200000*8)*tmp); /*shaper update freq is 200 Mhz*/


             cpssOsPrintf("\n|%4d|%3c|%7d|%7d|%5d|%7d|%11d|",i,shapingEnabled?'y':'n',divExp,token,res,burst,bw);

             cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        }

        cpssOsPrintf("\n");

        return GT_OK;


}


GT_STATUS prvCpssDxChTxqDumpAnodeShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 aNodeStart,
    IN GT_U32 aNodeEnd
)
{
        GT_U32 i;
        GT_U32 rc;
        GT_U32    divExp;
        GT_U32    token;
        GT_U32    res;
        GT_U32    burst;
        GT_BOOL shapingEnabled;
        GT_U32     shaperDec;
        GT_U32     perInter;
        GT_U32     perEn,bw = 0;
        double tmp;



       rc = prvCpssFalconTxqPdqGetPerLevelShapingParameters(devNum,tileNum,GT_FALSE,&shaperDec,&perInter,&perEn);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetPerLevelShapingParameters  failed  ");
        }

        cpssOsPrintf("\nA level parameters:\n\nPeriodic interval %d\n\nshaperDec %d\n\nperEn %d\n",perInter,shaperDec,perEn);

        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");
        cpssOsPrintf("\n| A  |shp| divExp|token  |res  | burst |BW in Kbit |");
        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        for(i=aNodeStart; (i<=aNodeEnd) && (i<CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC);i++)
        {
            rc = prvCpssFalconTxqPdqGetShapingEnable(devNum,tileNum,i,GT_FALSE,&shapingEnabled);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetShapingEnable  failed for q  %d  ",i);
            }

            rc = prvCpssFalconTxqPdqGetAnodeShapingParameters(devNum,tileNum,i,&divExp,&token,&res,&burst);

             if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueShapingParameters  failed for q  %d  ",i);
            }

            tmp =(double) ( token*(1<<res))/(perInter*(1<<divExp));
            bw = (GT_U32)((200000*8)*tmp); /*shaper update freq is 200 Mhz*/


             cpssOsPrintf("\n|%4d|%3c|%7d|%7d|%5d|%7d|%11d|",i,shapingEnabled?'y':'n',divExp,token,res,burst,bw);

             cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        }

        cpssOsPrintf("\n");

        return GT_OK;


}

/**
 * @internal prvCpssDxChTxqBindQueueToProfileSet function
 * @endinternal
 *
 * @brief Bind queue to queue  profile( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[in] profileIndex           - index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToProfileSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN GT_U32  profileIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr;
    GT_U32 queueNum;
    GT_U32 tileNum;
    GT_U32 pdsNum;

    rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,&tileNum,&queueNum/*just for dummy*/);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping  failed for portNum  %d  ",portNum);
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&anodePtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    queueNum = anodePtr->queuesData.queueBase+ tc;
    pdsNum = anodePtr->queuesData.dp;

    rc = prvCpssDxChTxqFalconPdsQueueProfileMapSet(devNum,tileNum,pdsNum,queueNum,profileIndex);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueProfileMapSet  failed for queueNum  %d  ",queueNum);
    }


    return rc;

}

/**
 * @internal prvCpssDxChTxqBindQueueToProfileGet function
 * @endinternal
 *
 * @brief Get binded  queue  profile index( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[out] profileIndexPtr           - (pointer to)index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    OUT GT_U32  * profileIndexPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr;
    GT_U32 queueNum;
    GT_U32 tileNum;
    GT_U32 pdsNum;

    rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,&tileNum,&queueNum/*just for dummy*/);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping  failed for portNum  %d  ",portNum);
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&anodePtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    queueNum = anodePtr->queuesData.queueBase+ tc;
    pdsNum = anodePtr->queuesData.dp;

    rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum,tileNum,pdsNum,queueNum,profileIndexPtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueProfileMapGet  failed for queueNum  %d  ",queueNum);
    }


    return rc;

}


/**
 * @internal prvCpssDxChTxqSetPortSdqThresholds function
 * @endinternal
 *
 * @brief Set port and queue thresholds (credit allocation requests to PDQ)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */
GT_STATUS prvCpssDxChTxqSetPortSdqThresholds
(
    IN  GT_U8   devNum,
   IN  GT_PHYSICAL_PORT_NUM portNum,
   IN CPSS_PORT_SPEED_ENT speed
)
{
    GT_STATUS rc;
    GT_U32 tileNum;
    GT_U32 i,lastQueue;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pnodePtr;
    GT_U32 dummy = 0,localdpPortNum,dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    GT_BOOL isCascade;
    GT_U32 pNodeNum,aNodeListSize;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }


    /*Threshold set only for local ports ,remote ports thresholds will be set via cascade port*/
    if(mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
     {
             rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,portNum,&isCascade);

            if(rc!=GT_OK)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ",portNum);
             }

            if(isCascade == GT_TRUE)
            {
                    pNodeNum = dpNum + (MAX_DP_IN_TILE(devNum))*localdpPortNum;

                    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[ tileNum]);

                    pnodePtr = &(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeNum]);

                     i = pnodePtr->aNodelist[0].queuesData.queueBase;
                     aNodeListSize = pnodePtr->aNodeListSize;
                     lastQueue = i+(pnodePtr->aNodelist[aNodeListSize-1].queuesData.pdqQueueLast -pnodePtr->aNodelist[0].queuesData.pdqQueueFirst);
            }
            else
            {

                rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,&tileNum,&dummy);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping  failed for portNum  %d  ",portNum);
                }

                rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&anodePtr);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ",portNum);
                }

                i = anodePtr->queuesData.queueBase;
                lastQueue = i+(anodePtr->queuesData.pdqQueueLast -anodePtr->queuesData.pdqQueueFirst);

             }



            /*set queue thresholds */

            for(;i<=lastQueue;i++)
            {
                    rc = prvCpssFalconTxqSdqQueueAttributesSet(devNum,tileNum,dpNum,i,speed);

                     if(rc!=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueAttributesSet  failed for q  %d  ",i);
                    }
            }

             /*set port BP thresholds */

            rc = prvCpssFalconTxqSdqPortAttributesSet(devNum,tileNum,dpNum,localdpPortNum,speed);

             if(rc!=GT_OK)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueAttributesSet  failed for q  %d  ",i);
             }
        }

    return GT_OK;

}

/**
 * @internal prvCpssDxChTxqInitPdsLongQueueProfiles function
 * @endinternal
 *
 * @brief Init long queue and length adjust profiles (per all the possible speeds)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE: CPSS copy all the profiles to all the DP cores in all tiles
 */

GT_STATUS prvCpssDxChTxqInitPdsLongQueueProfiles
(
    IN  GT_U8   devNum
)
{
        static const CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC queuePdsProfiles[ ] =
            {
            {GT_TRUE ,39,43},/*0 - 50*/
            {GT_TRUE ,21,24},/*1- 10*/
            {GT_TRUE ,28,31},/*2 - 25*/
            {GT_TRUE ,61,68},/*3 - 100*/
            {GT_TRUE ,106,117},/*4 - 200*/
             {GT_TRUE ,195,215} /*5 -400*/};

        GT_U32 i;
        GT_STATUS rc;
        CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profile;

        profile.lengthAdjustParameters.lengthAdjustEnable = GT_TRUE;
        profile.lengthAdjustParameters.lengthAdjustSubstruct = GT_FALSE;
        profile.lengthAdjustParameters.lengthAdjustByteCount = PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_LENGTH_ADJUST_BYTES_MAC;

        for(i=0;i<sizeof(queuePdsProfiles)/sizeof(queuePdsProfiles[0]);i++)
        {
                profile.longQueueParameters = queuePdsProfiles[i];
                rc = cpssDxChPortTxQueueProfileSet(devNum,i,profile);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChPortTxQueueProfileSet  failed for profile  %d  ",i);
                }
        }

        return GT_OK;

}

/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Bind port queues to PDS profile  (depend on port speed)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */

GT_STATUS prvCpssDxChTxqBindPortQueuesToPdsProfile
(
   IN  GT_U8   devNum,
   IN  GT_PHYSICAL_PORT_NUM portNum,
   IN CPSS_PORT_SPEED_ENT speed
)
{
    GT_STATUS rc;
    GT_U32 tileNum;
    GT_U32 i,lastQueue;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pnodePtr;
    GT_U32 dummy = 0,localdpPortNum,dpNum,profile = PRV_TXQ_PDS_10G_PROFILE_MAC;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    GT_BOOL isCascade;
    GT_U32 pNodeNum,aNodeListSize;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }


    /*Threshold set only for local ports ,remote ports thresholds will be set via cascade port*/
    if(mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
     {
             rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,portNum,&isCascade);

            if(rc!=GT_OK)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ",portNum);
             }

            if(isCascade == GT_TRUE)
            {
                    pNodeNum = dpNum + (MAX_DP_IN_TILE(devNum))*localdpPortNum;

                    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[ tileNum]);

                    pnodePtr = &(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeNum]);

                     i = pnodePtr->aNodelist[0].queuesData.queueBase;
                     aNodeListSize = pnodePtr->aNodeListSize;
                     lastQueue = i+(pnodePtr->aNodelist[aNodeListSize-1].queuesData.pdqQueueLast -pnodePtr->aNodelist[0].queuesData.pdqQueueFirst);
            }
            else
            {

                rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,&tileNum,&dummy);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping  failed for portNum  %d  ",portNum);
                }

                rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&anodePtr);

                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ",portNum);
                }

                i = anodePtr->queuesData.queueBase;
                lastQueue = i+(anodePtr->queuesData.pdqQueueLast -anodePtr->queuesData.pdqQueueFirst);

             }

           switch(speed)
        {
                case CPSS_PORT_SPEED_10000_E:
                    profile =  PRV_TXQ_PDS_10G_PROFILE_MAC;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    profile =  PRV_TXQ_PDS_50G_PROFILE_MAC;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                   profile =  PRV_TXQ_PDS_100G_PROFILE_MAC;
                    break;
                case CPSS_PORT_SPEED_200G_E:
                    profile =  PRV_TXQ_PDS_200G_PROFILE_MAC;
                    break;
               case CPSS_PORT_SPEED_400G_E:
                    profile =  PRV_TXQ_PDS_400G_PROFILE_MAC;
                    break;

               default :
                   rc = GT_FAIL;
                  break;
        }

        if(rc == GT_OK)
        {

            /*set queue thresholds */
            for(;i<=lastQueue;i++)
            {
                    rc = prvCpssDxChTxqFalconPdsQueueProfileMapSet(devNum,tileNum,dpNum,i,profile);

                     if(rc!=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueAttributesSet  failed for q  %d  ",i);
                    }
            }
        }


        }

    return rc;

}

/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Dump queue to profile PDS binding
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] tileNumber           -tile number to dump
 * @param[in] pdsNum                      - pds number to dump
  * @param[in] size                      - amount of queues to dump
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note
 */

GT_STATUS prvCpssFalconTxqUtilsPdqQueueProfilesBindDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 pdsNum,
    IN  GT_U32 size
)
{
    GT_U32           i,profile = 0;
    GT_STATUS        rc;


    cpssOsPrintf("\nQueueAttributes PDS %d\n",pdsNum);

    cpssOsPrintf("\n+-------------+--------+");
    cpssOsPrintf("\n| queueNumber |profile |");
    cpssOsPrintf("\n+-------------+--------+");

    for(i =0;i<size;i++)
    {
        rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum,tileNumber,pdsNum,i,&profile);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsQueueProfileMapGet\n",rc );
            return rc;
        }

        cpssOsPrintf("\n|%13d|%8d|",i,profile);
    }

   cpssOsPrintf("\n+-------------+--------+");

   return GT_OK;


}

/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Dump  PDS queue profile attributes
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] tileNumber           -tile number to dump
 * @param[in] pdsNum                      - pds number to dump
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note
 */

GT_STATUS prvCpssFalconTxqUtilsPdqQueueProfilesAttributesDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 pdsNum
)
{
    GT_U32           i;
    GT_STATUS        rc;
    CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profile;


    cpssOsPrintf("\n PDS queue profile tile %d  pds %d\n",tileNumber,pdsNum);

    cpssOsPrintf("\n+----------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n| profile  | longQueueEnable |   headEmptyLimit| longQueueLimit  |lengthAdjEnable  |lengthAdjSubstrct|lengthAdjByteCnt |");
    cpssOsPrintf("\n+----------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+");

    for(i =0;i<PDS_PROFILE_MAX_MAC;i++)
    {
        rc = prvCpssDxChTxqFalconPdsProfileGet(devNum,tileNumber,pdsNum,i,&profile);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsProfileGet\n",rc );
            return rc;
        }

        cpssOsPrintf("\n|%10d|%17d|%17d|%17d|%17d|%17d|%17d|",i,profile.longQueueParameters.longQueueEnable,
            profile.longQueueParameters.headEmptyLimit,
            profile.longQueueParameters.longQueueLimit,
            profile.lengthAdjustParameters.lengthAdjustEnable,
            profile.lengthAdjustParameters.lengthAdjustSubstruct,
            profile.lengthAdjustParameters.lengthAdjustByteCount);
    }

    cpssOsPrintf("\n+----------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n");

   return GT_OK;


}


/**
 * @internal prvCpssDxChTxqSetPlevelDwrr function
 * @endinternal
 *
 * @brief The function set correct weight at DWRR depending on port speed.
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      If the port is local port then P level weight is updated,if the port is remote port then A level weight is updated.
 */

GT_STATUS prvCpssDxChTxqSetDwrr
(
   IN  GT_U8   devNum,
   IN  GT_PHYSICAL_PORT_NUM portNum,
   IN CPSS_PORT_SPEED_ENT speed
)
{
    GT_STATUS rc;
    GT_U32 tileNum;

    GT_U32 localdpPortNum,dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    GT_U32 aNodeNum,quantum = 0;
#ifdef    CPSS_LOG_ENABLE
    GT_U32 pNodeNum;
#endif

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }


    /*only remote ports support DWRR*/
    if(mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        #ifdef    CPSS_LOG_ENABLE
        pNodeNum = dpNum + (MAX_DP_IN_TILE(devNum))*localdpPortNum;
        #endif
        /*Configure A level for remote ports  */
        rc = prvCpssFalconTxqPdqConvertSpeedToQuantum(speed,GT_TRUE,&quantum);

        if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqConvertSpeedToQuantum  failed for speed (enum)  %d  ",speed);
         }

        /*Find A level index*/

         rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,&tileNum,&aNodeNum);

         if(rc!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping  failed for portNum  %d  ",portNum);
         }

         /*Now we got the tile,A level  index and quantum.Let's set it*/

         rc = prvCpssFalconTxqPdqSetAlevelQuantum(devNum,tileNum,aNodeNum,quantum);

         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqSetBlevelQuantum  failed for p node  %d  ",pNodeNum);
         }
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping function
 * @endinternal
 *
 * @brief The function dump         local_port_%p_source_port_config table
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 * @param[in] tileNum                    -tile number
 * @param[in] qfcNum                   - qfc number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 */

GT_STATUS prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum,
    IN GT_U32 localPortStart,
    IN GT_U32 localPortEnd,
    OUT GT_U32 *dmaPtr
)
{
        GT_U32 i;
        GT_STATUS rc;
        GT_U32   dmaPort=0;
        CPSS_DXCH_PORT_FC_MODE_ENT  portType;


        cpssOsPrintf("Tile %d  qfcNum %d\n",tileNum,qfcNum);



        cpssOsPrintf("\n+----+-------+--------+");
        cpssOsPrintf("\n|Port|DMA    |portType|");
        cpssOsPrintf("\n+----+-------+--------+");

        for(i=localPortStart;  i<=localPortEnd;i++)
        {
            rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum,tileNum,qfcNum,i,&dmaPort,&portType);

             if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortGet  failed for tile  %d  ",i);
            }

             cpssOsPrintf("\n|%4d|%7d|%7d|",i,dmaPort,portType);

            cpssOsPrintf("\n+----+-------+--------+");

        }

        cpssOsPrintf("\n");

        *dmaPtr = dmaPort;

        return GT_OK;

}

/**
 * @internal prvCpssDxChTxqPfccTableInit function
 * @endinternal
 *
 * @brief The function initialize PFCC CFG table and Global_pfcc_CFG register. Only master tile is configured
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 */

GT_STATUS prvCpssDxChTxqPfccTableInit
(
    IN GT_U8 devNum
)
{
       GT_U32 tableSize,i;
       PRV_CPSS_PFCC_TILE_INIT_STC  initDb;
       GT_STATUS rc;
       PRV_CPSS_PFCC_CFG_ENTRY_STC  bubble={0};

       GT_U32 masterTile = PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC;

        /*First decide on table size*/

      /*currently waisting (maxDP-2) entries because only 2 CPU NW ports are availeble.Since we have 32 spare entries it is ok*/
      tableSize = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;
      tableSize+=8;/*Global TC - always last 16 till 8*/

      if(prvCpssDxchFalconCiderVersionGet()>PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
      {
        tableSize+=8;/*Global TC HR - always last  8*/
      }


      initDb.pfccLastEntryIndex = tableSize-1;
      initDb.isMaster = GT_TRUE;
      initDb.pfccEnable = GT_TRUE;


       rc = prvCpssFalconTxqPfccUnitCfgSet(devNum,masterTile,&initDb);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccUnitCfgSet  failed for tile  %d  ",masterTile);
        }

        bubble.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE;
        bubble.numberOfBubbles = 1;

        for(i = 0;i<tableSize;i++)
        {
            /*Now set all entries to bubbles of size 1*/
            rc = prvCpssFalconTxqPfccCfgTableEntrySet(devNum,masterTile,i,&bubble);

             if(rc!=GT_OK)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntrySet  failed for tile  %d  ",masterTile);
             }
         }


      return GT_OK;
}


/**
 * @internal prvCpssDxChTxqQfcUnitsInit function
 * @endinternal
 *
 * @brief Enable local PFC generation at all QFCs.PFC generation will be controlled at PFCC
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 */
GT_STATUS prvCpssDxChTxqQfcUnitsInit
(
    IN GT_U8 devNum
)
{

    GT_U32 numberOfTiles,i,j;
    GT_STATUS rc;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
     {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;

     }

  for(i=0;i<numberOfTiles;i++)
  {
        for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
        {

            rc = prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet(devNum,i,j,GT_TRUE);

            if(rc!=GT_OK)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPbLimitSet fail");
            }
            /*HR counting is enabled by default in HW.So no need to enable in initialization*/
        }
  }

      return GT_OK;
}

/**
* @internal prvCpssFalconTxqPfcValidGet function
* @endinternal
*
* @brief   This function check if PFC is configured per port or port/TC
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                 data path[0..7]
* @param[in] localPort                         local port number[0..8]
* @param[in] tcBmpPtr                           (pointer to) traffic class that is set for flow control,0x0 mean no flow control is set,0xFF mean flow control on port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfcValidGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    OUT  GT_U32                 *tcBmpPtr
)
{
    GT_STATUS                    rc;
    CPSS_DXCH_PFC_THRESHOLD_STC  thresholdCfg;
    GT_BOOL                      enable;
    GT_U32                       i;

    *tcBmpPtr=0;

    rc = prvCpssFalconTxqQfcPortPfcThresholdGet(devNum,tileNum,dpNum,localPort,&enable,&thresholdCfg,NULL);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortPfcThresholdGet  failed for localPort  %d  ",localPort);
    }
    if(enable==GT_TRUE)
    {
        *tcBmpPtr=0xFF;
        return GT_OK;
    }
    /*Check port/TC*/
    for(i=0;i<8;i++)
    {
        rc = prvCpssFalconTxqQfcPortTcPfcThresholdGet(devNum,tileNum,dpNum,localPort,i,&enable,&thresholdCfg);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortTcPfcThresholdGet  failed for localPort  %d  ",localPort);
        }

        if(enable==GT_TRUE)
        {
            *tcBmpPtr|=1<<i;
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssFalconTxqUtilsNumberOfConsumedLanesGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  *numberOfConsumedLanesPtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32       activeSliceNum;
    GT_U32       activeSliceMap[TXQ_PDX_MAX_SLICE_NUMBER_MAC];
    GT_BOOL      sliceValid[TXQ_PDX_MAX_SLICE_NUMBER_MAC];

    *numberOfConsumedLanesPtr = 0;


   /*TBD mapping check*/

    rc = prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet(devNum,tileNum,dpNum,&activeSliceNum,activeSliceMap,sliceValid);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet  failed for dpNum  %d  ",dpNum);
    }

    /*CPU port*/
    if(localPort ==CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC-1)
    {
        if(activeSliceNum!=TXQ_PDX_MAX_SLICE_NUMBER_MAC)
        {
            /*no CPU port mapped on this DP*/
            return GT_OK;
        }
        if(sliceValid[TXQ_PDX_MAX_SLICE_NUMBER_MAC-1]==GT_TRUE)
        {
            *numberOfConsumedLanesPtr = 1;
            return GT_OK;
        }
    }

    /*Read only first 8 entries due to duplication of pizza*/

    for(i=0;i<8;i++)
    {
        if((sliceValid[i]==GT_TRUE)&&(activeSliceMap[i]==localPort))
        {
            (*numberOfConsumedLanesPtr)++;
        }
    }

    return GT_OK;

}

static GT_STATUS prvCpssFalconTxqPffcTableSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  tcBmp,
    IN  GT_U32                  numberOfConsumedLanes,
    IN  GT_BOOL                 enable
)
{

    GT_STATUS rc;
    GT_U32 i;
    GT_U32  startIndex;
    PRV_CPSS_PFCC_CFG_ENTRY_STC      entry;

    startIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*localPort+dpNum+tileNum*(MAX_DP_IN_TILE(devNum));

    if(enable == GT_TRUE)
    {
        /*Get DMA*/
        rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum, tileNum,dpNum,localPort, &entry.portDma,NULL);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortGet  failed for localPort  %d  ",localPort);
        }

        entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT;
        entry.pfcMessageTrigger = GT_TRUE;
        entry.tcBitVecEn = tcBmp;

    }
    else
    {
        entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE;
        entry.numberOfBubbles = 1;
    }

    for(i=0;i<numberOfConsumedLanes;i++)
    {
        rc= prvCpssFalconTxqPfccCfgTableEntrySet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,startIndex,&entry);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for localPort  %d  ",localPort);
        }
        /*Prepare for next iteration*/
        localPort++;
        startIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*(localPort)+dpNum+tileNum*(MAX_DP_IN_TILE(devNum));
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqPffcTableSyncSet function
* @endinternal
*
* @brief   This function read from PDX pizza and configure PFCC table according to number of lanes consumed by port
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                 data path[0..7]
* @param[in] localPort                         local port number[0..8]
* @param[in] tcBmp                           traffic class that is set for flow control,0xFF mean flow control on port(relevant only if enable is set to GT_TRUE)
* @param[in] enable                           enable/disable entry at PFCC table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPffcTableSyncSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  tcBmp,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    GT_U32    numberOfConsumedLanes;

    rc = prvCpssFalconTxqUtilsNumberOfConsumedLanesGet(devNum,tileNum,dpNum,localPort,&numberOfConsumedLanes);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for localPort  %d  ",localPort);
    }

    rc = prvCpssFalconTxqPffcTableSet(devNum,tileNum,dpNum,localPort,tcBmp,numberOfConsumedLanes,enable);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPffcTableSet  failed for localPort  %d  ",localPort);
    }

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqPffcTableSyncSet function
* @endinternal
*
* @brief   This function  PFCC table according to given configuration of consumed lanes
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                data path[0..7]
* @param[in] localPort                        local port number[0..8]
* @param[in] portPizzaSlices-           desired PDX pizza configuration
* @param[in] tcBmp                            traffic class that is set for flow control,0xFF mean flow control on port(relevant only if enable is set to GT_TRUE)
* @param[in] enable                           enable/disable entry at PFCC table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPffcTableExplicitSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  portPizzaSlices,
    IN  GT_U32                  tcBmp,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32                           numberOfConsumedLanes;

    numberOfConsumedLanes= 0;

    /*CPU port*/
    if(localPort ==CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC-1)
    {
        numberOfConsumedLanes = 1;
    }
    else
    {
        /*Read only first 8 entries due to duplication of pizza*/
        for(i=0;i<8;i++)
        {
            if((portPizzaSlices&1<<i))
            {
                numberOfConsumedLanes++;
            }
        }
    }


    rc = prvCpssFalconTxqPffcTableSet(devNum,tileNum,dpNum,localPort,tcBmp,numberOfConsumedLanes,enable);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPffcTableSet  failed for localPort  %d  ",localPort);
    }

    return GT_OK;

}


/**
* @internal prvCpssFalconTxqUtilsFlowControlInit function
* @endinternal
*
* @brief   This function  initialize PFC quanta and PFC threshold for all mapped ports
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         PP's device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsFlowControlInit
(
    IN GT_U8 devNum
)
{
    GT_U32 i,j;
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    for(i=0;i<PRV_CPSS_MAX_PP_PORTS_NUM_CNS ;i++)
    {
       rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, i, /*OUT*/&portMapShadowPtr);

       if (rc != GT_OK)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
       }

       if(portMapShadowPtr->valid == GT_TRUE)
       {
            if(portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                for(j=0;j<8;j++)
                {
                  rc = prvCpssDxChPortSip6PfcPauseQuantaSet(devNum,i,j,PRV_CPSS_DXCH_FALCON_TXQ_MAX_QUANTA_MAC);

                  if (rc != GT_OK)
                  {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcPauseQuantaSet failed for port %d tc % d\n",i,j);
                  }

                  rc = prvCpssDxChPortSip6PfcQuantaThreshSet(devNum,i,j,PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_THRESHOLD_MAC);

                  if (rc != GT_OK)
                  {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcQuantaThreshSet failed for port %d tc % d\n",i,j);
                  }
                }
            }
       }

    }

     rc =prvCpssFalconTxqUtilsFlowControlInitAvailebleBufferSize(devNum);
     if (rc != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsFlowControlInitBufferSize failed.\n");
    }

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsQueueTcSet function
* @endinternal
*
* @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] queueNumber                                              local queue offset [0..15]
* @param[in] tcForPfcResponce                       -      Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsQueueTcSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queueNumber,
    IN GT_U32                  tc
)
{
    GT_STATUS rc;
    GT_U32                           tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE   *anodePtr;
    GT_U32                           lastValidQueue;

    /*Find  local SDQ queue number*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType!=CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "PFC responce configured not on local port");
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&anodePtr);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    /*Check range*/
    lastValidQueue = anodePtr->queuesData.pdqQueueLast-anodePtr->queuesData.pdqQueueFirst;
    if(queueNumber>lastValidQueue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Wrong queue number %d",queueNumber);
    }

    /*TC validity is checked inside prvCpssFalconTxqSdqQueueTcSet*/
    rc = prvCpssFalconTxqSdqQueueTcSet(devNum,tileNum,dpNum,anodePtr->queuesData.queueBase+queueNumber,tc);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueTcSet  failed for portNum  %d  ",portNum);
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqUtilsQueueTcGet function
* @endinternal
*
* @brief   Gets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] queueNumber           local queue offset [0..15]
* @param[in] tcPtr                      -        (pointer to)Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsQueueTcGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queueNumber,
    IN GT_U32                  *tcPtr
)
{
    GT_STATUS rc;
    GT_U32                           tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE   *anodePtr;
    GT_U32                           lastValidQueue;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES queueAttributes;

    /*Find  local SDQ queue number*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType!=CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "PFC responce configured not on local port");
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&anodePtr);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    /*Check range*/
    lastValidQueue = anodePtr->queuesData.pdqQueueLast-anodePtr->queuesData.pdqQueueFirst;
    if(queueNumber>lastValidQueue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Wrong queue number %d",queueNumber);
    }

    rc = prvCpssFalconTxqSdqQueueAttributesGet(devNum,tileNum,dpNum,anodePtr->queuesData.queueBase+queueNumber,
        &queueAttributes);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueAttributesGet  failed for portNum  %d  ",portNum);
    }

    *tcPtr = queueAttributes.tc;

    return rc;
}


/**
* @internal prvCpssFalconTxqUtilsPfcDisable function
* @endinternal
*
* @brief   Read PFC  enable status for both generation and responce and then disable PFC on RX/TX at
*               TxQ and MAC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
* @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPfcDisable
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
)
{
    GT_STATUS rc;
    GT_U32                           tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;

    CPSS_NULL_PTR_CHECK_MAC(pfcRespBitMapPtr);
    CPSS_NULL_PTR_CHECK_MAC(pfcGenerationModePtr);

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }


    /*1.Read PFC responce bitmap*/
    rc = prvCpssFalconTxqSdqPfcControlEnableBitmapGet(devNum,tileNum,dpNum,localdpPortNum,pfcRespBitMapPtr);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapGet  failed for localdpPortNum  %d  ",localdpPortNum);
    }

    /*2.Read PFC generation mode*/
    rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum,tileNum,dpNum,localdpPortNum,NULL,pfcGenerationModePtr);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortGet failed for localdpPortNum %d",localdpPortNum);
    }

    /*3.Disable PFC responce at SDQ*/
    rc = prvCpssFalconTxqSdqPfcControlEnableBitmapSet(devNum,tileNum,dpNum,localdpPortNum,0);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapGet  failed for localdpPortNum  %d  ",localdpPortNum);
    }
    /*4.Disable PFC responce at MAC*/
    rc = prvCpssDxChPortSip6PfcPauseIgnoreEnableSet(devNum,portNum,GT_TRUE);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ",portNum);
    }

    /*5.Disable PFC generation at QFC*/
    rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum,tileNum,dpNum,localdpPortNum,0,CPSS_DXCH_PORT_FC_MODE_DISABLE_E,GT_TRUE);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortGet failed for localdpPortNum %d",localdpPortNum);
    }

    return GT_OK;

}


/**
* @internal prvCpssFalconTxqUtilsPfcEnable function
* @endinternal
*
* @brief   Set PFC  enable status for both generation and responce  at
*               TxQ and MAC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
* @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPfcEnable
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
)
{
    GT_STATUS rc;
    GT_U32                           tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;

    CPSS_NULL_PTR_CHECK_MAC(pfcRespBitMapPtr);
    CPSS_NULL_PTR_CHECK_MAC(pfcGenerationModePtr);

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    /*1.Write PFC responce bitmap*/
    rc = prvCpssFalconTxqSdqPfcControlEnableBitmapSet(devNum,tileNum,dpNum,localdpPortNum,*pfcRespBitMapPtr);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapSet  failed for localdpPortNum  %d  ",localdpPortNum);
    }

    /*2.Write PFC generation mode*/
     rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum,tileNum,dpNum,localdpPortNum,0,*pfcGenerationModePtr,GT_TRUE);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortSet failed for localdpPortNum %d",localdpPortNum);
    }

    /*4.Enable PFC responce at MAC*/
    rc = prvCpssDxChPortSip6PfcPauseIgnoreEnableSet(devNum,portNum,GT_FALSE);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ",portNum);
    }

    return GT_OK;

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
* @param[in] portNum                                                         physical port number
* @param[in] enable                                                             Enable/disable "credit ignore" mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPortFlushSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    GT_U32                           tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;


    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType!=CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        return GT_OK;
    }

    rc = prvCpssFalconTxqSdqLocalPortFlushSet(devNum,tileNum,dpNum,localdpPortNum,enable);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapSet  failed for localdpPortNum  %d  ",localdpPortNum);
    }


    return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsGetCascadePort function
* @endinternal
*
* @brief  Get Cascade port related to remote port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] remotePortNum                                             physical remote port number
* @param[out] casCadePortNumPtr                                  (pointer to) cascade port
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS prvCpssFalconTxqUtilsGetCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 remotePortNum,
    IN GT_BOOL remote,
    OUT GT_U32 *casCadePortNumPtr,
    OUT GT_U32 * pNodeIndPtr
)
{
        GT_STATUS rc;
        /*In order to avoid compiler warning initialize pNode*/
        GT_U32  pNode = CPSS_PDQ_SCHED_INVAL_DATA_CNS;
        GT_U32   tileNum= 0;
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

       rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, remotePortNum, /*OUT*/&portMapShadowPtr);

       if(rc!=GT_OK)
       {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
       }

       if(portMapShadowPtr->valid)
        {

           if(remote&&portMapShadowPtr->portMap.mappingType!=CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
           {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Port %d is not remote",remotePortNum);
           }
            rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum,portMapShadowPtr->portMap.txDmaNum,&tileNum,&pNode,GT_TRUE);
            if(rc!=GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
             tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[tileNum]);
             if(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNode].isCascade)
             {
                *casCadePortNumPtr = tileConfigsPtr->mapping.pNodeMappingConfiguration[pNode].cascadePhysicalPort;
             }
             else
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "The remote port %d has no cascade port",remotePortNum);
             }

             if(pNodeIndPtr)
             {
                *pNodeIndPtr= pNode;
             }

             return GT_OK;

       }

       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);


}

/**
* @internal prvCpssFalconTxqUtilsPortEnableSet function
* @endinternal
*
* @brief  Set port to enable in SDQ.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[in] enable                                                             Enable/disable "credit ignore" mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPortEnableSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    GT_U32                           tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;


    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType!=CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        return GT_OK;
    }

    rc = prvCpssFalconTxqSdqLocalPortEnableSet(devNum,tileNum,dpNum,localdpPortNum,enable);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapSet  failed for localdpPortNum  %d  ",localdpPortNum);
    }


    return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsPortEnableSet function
* @endinternal
*
* @brief  Flush queues of remote ports.Wait until the queues are empty.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsRemotePortFlush
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32 numberOfMappedQueues,i,queueIterator;
    GT_U32 prevDescNum,tileNum,dpNum,localPortNum;
    GT_U32 queueDescNum = (GT_U32) -1; /* number of not treated TXQ descriptors */
    PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC perQueueDescCount;
    GT_U32                                     pfcRespBitMap;
    CPSS_DXCH_PORT_FC_MODE_ENT                 pfcGenerationMode;
    GT_U32                                     cascadePort = 0/*avoid warning*/;

    /*Find the pointer to A node*/
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " A node is not found for  physical port %d  ",portNum);
    }

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localPortNum,NULL);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssFalconTxqUtilsGetCascadePort(devNum,portNum,GT_TRUE,&cascadePort,NULL);
    if (rc != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsGetCascadePort fail");
    }

    /*Disable PFC on cascade*/
    rc = prvCpssFalconTxqUtilsPfcDisable(devNum,cascadePort,&pfcRespBitMap,&pfcGenerationMode);
    if (rc != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPfcDisable fail");
    }

    numberOfMappedQueues = aNodePtr->queuesData.pdqQueueLast-aNodePtr->queuesData.pdqQueueFirst+1;

    for(i=0;i<numberOfMappedQueues;i++)
    {
        queueIterator = aNodePtr->queuesData.queueBase+i;

        do
        {
            prevDescNum = queueDescNum;
            rc = prvCpssDxChTxqFalconPdsPerQueueCountersGet(devNum,tileNum,dpNum,queueIterator,&perQueueDescCount);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet  failed for tile  %d pds %d ",tileNum,i);
            }

            queueDescNum = perQueueDescCount.headCounter;

            if(GT_TRUE == perQueueDescCount.longQueue)
            {
                queueDescNum +=perQueueDescCount.tailCounter+(16*perQueueDescCount.fragCounter);
            }

            if(0 == queueDescNum)
            {
                break;
            }
            else
            {
                cpssOsTimerWkAfter(10);
            }
        } while (queueDescNum < prevDescNum);

        if(0 == queueDescNum)
        {
            /*move to next Q*/
            i++;
        }
        else
        {
            /*Failure*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Can not flush queue %d port %d ",i,portNum);
        }
    }

    /*Restore  PFC state*/
    rc = prvCpssFalconTxqUtilsPfcEnable(devNum,cascadePort,&pfcRespBitMap,&pfcGenerationMode);
    if (rc != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPfcEnable fail");
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqUtilsInitTailDrop function
* @endinternal
*
* @brief  Initialize PREQ tail drop parameters to default.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsInitTailDrop
(
    IN GT_U8                                devNum
)
{
   GT_U32 numberOfTiles;
   GT_STATUS rc;
   CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS   tailDropWredProfileParams;
   CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet;
   GT_U8 trafficClass;
   CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;

   switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
   {
       case 0:
       case 1:
           numberOfTiles = 1;
           break;
      case 2:
      case 4:
           numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
           break;
       default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
           break;

   }
    rc = cpssDxChPortTxTailDropGlobalParamsSet(devNum,PRV_CPSS_TD_DEFAULT_RESOURCE_MODE_MAC,
        numberOfTiles*PRV_CPSS_TD_PB_SIZE_PER_TILE_MAC,/*globalAvailableBuffers*/
        numberOfTiles*PRV_CPSS_TD_PB_SIZE_PER_TILE_MAC,/*pool0AvailableBuffers*/
        numberOfTiles*PRV_CPSS_TD_PB_SIZE_PER_TILE_MAC);/*pool1AvailableBuffers*/
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropGlobalParamsSet failed ");
    }

    rc = cpssDxChPortTxMcastAvailableBuffersSet(devNum,numberOfTiles*PRV_CPSS_TD_PB_SIZE_PER_TILE_MAC);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxMcastAvailableBuffersSet failed ");
    }

    rc =cpssDxChPortTxGlobalDescLimitSet(devNum,(GT_U32)(PRV_CPSS_TD_GLOBAL_LIMIT_FACTOR_MAC*PRV_CPSS_TD_PB_SIZE_PER_TILE_MAC*numberOfTiles));
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxGlobalDescLimitSet failed ");
    }

    rc = cpssDxChPortTxMcastBuffersPriorityLimitSet(devNum,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E,0,CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxGlobalDescLimitSet failed ");
    }

    rc = cpssDxChPortTxMcastBuffersPriorityLimitSet(devNum,CPSS_PORT_TX_DROP_MCAST_PRIORITY_HI_E,0,CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxGlobalDescLimitSet failed ");
    }

    tailDropWredProfileParams.dp0WredAttributes.guaranteedLimit =0;
    tailDropWredProfileParams.dp0WredAttributes.probability = CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E;
    tailDropWredProfileParams.dp0WredAttributes.wredSize =0;

    tailDropWredProfileParams.dp1WredAttributes = tailDropWredProfileParams.dp0WredAttributes;
    tailDropWredProfileParams.dp2WredAttributes = tailDropWredProfileParams.dp0WredAttributes;

    tailDropProfileParams.tcMaxBuffNum = 0xfffff;
    tailDropProfileParams.dp0QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
    tailDropProfileParams.dp1QueueAlpha = tailDropProfileParams.dp0QueueAlpha;
    tailDropProfileParams.dp2QueueAlpha = tailDropProfileParams.dp0QueueAlpha;

     for(profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
         profileSet <= CPSS_PORT_TX_DROP_PROFILE_16_E; profileSet++)
    {
        rc = cpssDxChPortTxTailDropWredProfileSet(devNum,profileSet,&(tailDropWredProfileParams.dp0WredAttributes));
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
        }

        rc = cpssDxChPortTxTailDropProfileSet(devNum,profileSet,CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E,0,0);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropProfileSet failed ");
        }

        for(trafficClass = 0; trafficClass < TC_NUM_CNS; trafficClass++)
        {
            rc = cpssDxChPortTx4TcTailDropWredProfileSet(devNum,profileSet,trafficClass,&tailDropWredProfileParams);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
            }

            rc = cpssDxChPortTx4TcTailDropProfileSet(devNum,profileSet,trafficClass,&tailDropProfileParams);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
            }
        }
    }

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqUtilsDevClose function
* @endinternal
*
* @brief  Release dynamic memory allocation for TxQ scheduler
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsDevClose
(
    IN GT_U8                                devNum
)
{
    GT_STATUS rc;
    GT_U32    i,numberOfTiles;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
   {
       case 0:
       case 1:
           numberOfTiles = 1;
           break;
      case 2:
      case 4:
           numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
           break;
       default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
           break;

   }

    for(i=0;i<numberOfTiles;i++)
    {
        rc = prvCpssFalconTxqPdqClose(devNum,i);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPdqClose failed for tile %d",i);
        }
    }

    return GT_OK;
}



