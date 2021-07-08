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
* @file prvCpssDxChTxqDebugUtils.c
*
* @brief CPSS SIP6 TXQ debug functions
*
* @version   1
********************************************************************************
*/


/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPfcc.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPsi.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPdx.h>


#define PRV_CPSS_TXQ_DEBUG_PORT_MAPPING_CNS  0x1
#define PRV_CPSS_TXQ_DEBUG_PORT_STATUS_CNS   0x2
#define PRV_CPSS_TXQ_DEBUG_PORT_SHAPING_CNS  0x4
#define PRV_CPSS_TXQ_DEBUG_PORT_SCHED_CNS    0x8




extern GT_STATUS prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum,
    IN GT_U32 localPortStart,
    IN GT_U32 localPortEnd,
    OUT GT_U32 *dmaPtr
);

extern GT_STATUS prvCpssDxChTxqDumpAnodeShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 aNodeStart,
    IN GT_U32 aNodeEnd
);


extern GT_STATUS prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE ** outPtr
);

extern GT_STATUS prvCpssFalconTxqUtilsNumberOfConsumedLanesGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  *numberOfConsumedLanesPtr
);

extern GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);

extern GT_STATUS prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping
(
     IN  GT_U8 devNum,
     IN  GT_U32  physicalPortNumber,
     OUT   GT_U32   * tileNumPtr,
     OUT   GT_U32   * aNodeNumPtr
);

extern GT_U32 prvCpssFalconTxqUtilsGetPnodeIndex
(
    GT_U32   devNum,
    GT_U32   dpNum,
    GT_U32   localdpPortNum
);

extern GT_STATUS prvCpssDxChTxqDumpQueueShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd
);

/**
* @internal  prvCpssDxChTxqDebugDumpPfccTable  function
* @endinternal
*
* @brief   Dump PFCC unit configuration and PFCC table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number.
* @param[in] tileNum              Tile number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChTxqDebugDumpPfccTable
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   tileNum
)
{   GT_STATUS                    rc = GT_OK;
    GT_U32                       i,numberOfTiles;
    PRV_CPSS_PFCC_CFG_ENTRY_STC  entry;
    PRV_CPSS_PFCC_TILE_INIT_STC  pfccDb;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

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

    if(tileNum>=numberOfTiles)
    {

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Tile number bigger or equal[ %d ] then number of tiles  number of tiles [%d ]",tileNum,PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    }

    rc = prvCpssFalconTxqPfccUnitCfgGet(devNum,tileNum,&pfccDb);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccUnitCfgGet fail for tile %d",tileNum);
    }

    cpssOsPrintf("\n pfccEnable = %d",pfccDb.pfccEnable);
    cpssOsPrintf("\n isMaster = %d",pfccDb.isMaster);
    cpssOsPrintf("\n pfccLastEntryIndex = %d",pfccDb.pfccLastEntryIndex);

    cpssOsPrintf("\n+-----+-------+------+----+-------------------+---------------+----------------+");
    cpssOsPrintf("\n|Index|  Type | DMA  | TC | Number Of Bubbles | TC bit vector | Pfc msg trigger|");
    cpssOsPrintf("\n+-----+-------+------+----+-------------------+---------------+----------------+");

    for(i = 0;i<=pfccDb.pfccLastEntryIndex;i++)
    {
      rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,tileNum,i,&entry);

      if(rc!=GT_OK)
      {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntryGet fail for entry %d",i);
      }

      switch(entry.entryType)
      {
        case PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT:
          cpssOsPrintf("\n|%5d|  Port |%6d| NA |        NA         |%.15x|%16d|",i,entry.portDma,entry.tcBitVecEn,entry.pfcMessageTrigger);
          break;
        case PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC:
          cpssOsPrintf("\n|%5d|   TC  |  NA  |%4d|        NA         |     NA        |       NA       |",i,entry.globalTc);
          break;
        case PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR:
          cpssOsPrintf("\n|%5d| HR-TC |  NA  |%4d|        NA         |     NA        |       NA       |",i,entry.globalTc);
          break;
        default:
          cpssOsPrintf("\n|%5d| Bubble|  NA  | NA |        %5d      |     NA        |       NA       |",i,entry.numberOfBubbles);
          break;
      }
      cpssOsPrintf("\n+-----+-------+------+----+-------------------+---------------+----------------+");
      cpssOsTimerWkAfter(50);

    }

    cpssOsPrintf("\n");

    return rc;
}

/**
* @internal prvCpssDxChTxqDebugDumpFlowControlConfig function
* @endinternal
*
* @brief   Dump flow control configuration of specific port (txQ and L1)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcModeEnablePtr -(Pointer to)If equals GT_TRUE,the Core generates and processes PFC control frames according
*  to the Priority Flow Control Interface signals. If equals GT_FALSE (0 - Reset Value), the Core operates in legacy Pause Frame
*  mode and generates and processes standard Pause Frames.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChTxqDebugDumpFlowControlConfig
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;
    CPSS_DXCH_PORT_FC_MODE_ENT fcMode;
    GT_BOOL fcuTxEnable,fcuRxEnable,pfcMode,pauseFwd,pauseIgn;
    GT_U32 quanta,i;
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 rxPauseStatus,tcPaused;
    GT_U32 macStatus;
    GT_U32 regAddr;


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    rc = prvCpssDxChPortSip6FlowControlModeGet(devNum,portNum,&fcMode);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6FlowControlModeGet  failed for portNum  %d  ",portNum);
    }

    rc = prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet(devNum,portNum,&fcuTxEnable);

    if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet  failed for portNum  %d  ",portNum);
     }

    rc = prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet(devNum,portNum,&fcuRxEnable);

    if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet  failed for portNum  %d  ",portNum);
     }

    rc = prvCpssDxChPortSip6PfcModeEnableGet(devNum,portNum,&pfcMode);

     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcModeEnableGet  failed for portNum  %d  ",portNum);
     }

     rc = prvCpssDxChPortSip6PfcPauseFwdEnableGet(devNum,portNum,&pauseFwd);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcPauseFwdEnableGet  failed for portNum  %d  ",portNum);
     }

     rc = prvCpssDxChPortSip6PfcPauseIgnoreEnableGet(devNum,portNum,&pauseIgn);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcPauseIgnoreEnableGet  failed for portNum  %d  ",portNum);
     }

    cpssOsPrintf("\n+---+-----------+--------+--------+------------+------------------+------------------+");
    cpssOsPrintf("\n| # |TXQ FC type| FCU TX | FCU RX |COM CONF PFC|COM CONF PAUSE FWD|COM CONF PAUSE IGN|");
    cpssOsPrintf("\n+---+-----------+--------+--------+------------+------------------+------------------+");

    cpssOsPrintf("\n|%3d|%11s|%8d|%8d|%12d|%18d|%18d|",portNum,
        fcMode==CPSS_DXCH_PORT_FC_MODE_802_3X_E?"FC":(fcMode==CPSS_DXCH_PORT_FC_MODE_PFC_E?"PFC":"DISABLE"),
        fcuTxEnable?1:0,fcuRxEnable?1:0,pfcMode?1:0,pauseFwd?1:0,pauseIgn?1:0);
     cpssOsPrintf("\n+---+-----------+--------+--------+------------+------------------+------------------+");
    cpssOsPrintf("\n");

    cpssOsPrintf("Quanta - MAC TYPE %s\n",
        (macType == PRV_CPSS_PORT_MTI_64_E)?"MTI 100":(macType == PRV_CPSS_PORT_MTI_400_E)?"MTI 400":"Unknown");

    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+");
    cpssOsPrintf("\n|TC0   |TC1   |TC2   |TC3   |TC4   |TC5   |TC6   |TC7   |");
    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+\n");

    for(i=0;i<8;i++)
    {
     rc = prvCpssDxChPortSip6PfcPauseQuantaGet(devNum,portNum,i,&quanta);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcModeEnableGet  failed for portNum  %d  ",portNum);
     }
     cpssOsPrintf("| %5x",quanta);
    }
    cpssOsPrintf("|\n+------+------+------+------+------+------+------+------+\n");

    cpssOsPrintf("Threshold - MAC TYPE %s\n",
        (macType == PRV_CPSS_PORT_MTI_64_E)?"MTI 100":(macType == PRV_CPSS_PORT_MTI_400_E)?"MTI 400":"Unknown");

    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+");
    cpssOsPrintf("\n|TC0   |TC1   |TC2   |TC3   |TC4   |TC5   |TC6   |TC7   |");
    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+\n");

    for(i=0;i<8;i++)
    {
     rc = prvCpssDxChPortSip6PfcQuantaThreshGet(devNum,portNum,i,&quanta);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcQuantaThreshGet  failed for portNum  %d  ",portNum);
     }
     cpssOsPrintf("| %5x",quanta);
    }
    cpssOsPrintf("|\n+------+------+------+------+------+------+------+------+\n");

    cpssOsPrintf("Rx Pause Status - MAC TYPE %s\n",
        (macType == PRV_CPSS_PORT_MTI_64_E)?"MTI 100":(macType == PRV_CPSS_PORT_MTI_400_E)?"MTI 400":"Unknown");

    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+");
    cpssOsPrintf("\n|TC0   |TC1   |TC2   |TC3   |TC4   |TC5   |TC6   |TC7   |");
    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+\n");

    rc = prvCpssDxChPortSip6PfcRxPauseStatusGet(devNum,portNum,&rxPauseStatus);
    if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcQuantaThreshGet  failed for portNum  %d  ",portNum);
     }

    for(i=0;i<8;i++)
    {
     tcPaused=(rxPauseStatus>>i)&1;
     cpssOsPrintf("| %5x",tcPaused);
    }
    cpssOsPrintf("|\n+------+------+------+------+------+------+------+------+\n");


    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.status;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.status;
    }

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&macStatus);

    if(rc==GT_OK)
    {
        cpssOsPrintf("MAC status is 0x%x\n",macStatus);
    }

    return rc;
}

/**
* @internal  prvCpssDxChTxqDebugDumpQfcConfig  function
* @endinternal
*
* @brief   Dump QFC  unit configuration
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number.
* @param[in] tileNum              Tile number
* @param[in] qfcNum              Local QFC number [0..7]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChTxqDebugDumpQfcConfig
(
  IN  GT_U8                    devNum,
  IN  GT_U32                   tileNum,
  IN  GT_U32                   qfcNum
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  qfcCfg;
    GT_STATUS                                      rc;
    GT_U32                                         numberOfTiles;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

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

    if(tileNum>=numberOfTiles)
    {

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Tile number bigger or equal[ %d ] then number of tiles  number of tiles [%d ]",tileNum,PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    }

    rc = prvCpssFalconTxqQfcGlobalPfcCfgGet(devNum,tileNum,qfcNum,&qfcCfg);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcGlobalPfcCfgGet  failed for qfc  %d  ",qfcNum);
    }

    cpssOsPrintf("\n globalPfcEnable             = %d",qfcCfg.globalPfcEnable);
    cpssOsPrintf("\n ingressAgregatorTcPfcBitmap = %d",qfcCfg.ingressAgregatorTcPfcBitmap);
    cpssOsPrintf("\n pbAvailableBuffers          = %d",qfcCfg.pbAvailableBuffers);
    cpssOsPrintf("\n pbCongestionPfcEnable       = %d",qfcCfg.pbCongestionPfcEnable);
    cpssOsPrintf("\n pdxCongestionPfcEnable      = %d",qfcCfg.pdxCongestionPfcEnable);
    cpssOsPrintf("\n hrCountingEnable            = %d\n",qfcCfg.hrCountingEnable);

    return GT_OK;

}

/**
* @internal  prvCpssFalconTxqDebugDumpPortInfo  function
* @endinternal
*
* @brief   Dump port configuration
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
* @param[in] portNum              physical port number
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static GT_STATUS prvCpssFalconTxqDebugDumpPortMapping
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  printDebug,
    IN  GT_BOOL                 isCascade,
    OUT GT_U32                  *tileNumPtr,
    OUT GT_U32                  *dpNumPtr,
    OUT GT_U32                  *localdpPortNumPtr
)
{
    GT_STATUS rc;
    GT_U32 tileNum,numberOfConsumedLanes,aNodeIndex=0,pNodeIndex;
    GT_U32 localdpPortNum,dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }


    rc = prvCpssFalconTxqUtilsNumberOfConsumedLanesGet(devNum,tileNum,dpNum,localdpPortNum,&numberOfConsumedLanes);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(GT_FALSE == isCascade)
    {
        rc = prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping(devNum,portNum,NULL,&aNodeIndex);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetPhysicalNodeToAnodeMapping  failed for portNum  %d  ",portNum);
        }
    }

    pNodeIndex = prvCpssFalconTxqUtilsGetPnodeIndex(devNum,dpNum,localdpPortNum);

    if(printDebug)
    {

        cpssOsPrintf("\n+---------------+------+----+------------+----------------------+------+------+");
        cpssOsPrintf("\n| physical port | tile | DP | local port | numberOfConsumedLanes|P node|A node|");
        cpssOsPrintf("\n+---------------+------+----+------------+----------------------+------+------+");
        cpssOsPrintf("\n|%15d|%6d|%4d|%12d|%22d|%6d|%6d|",portNum,tileNum,dpNum,localdpPortNum,numberOfConsumedLanes,pNodeIndex,isCascade?0:aNodeIndex);
        cpssOsPrintf("\n+---------------+------+----+------------+----------------------+------+------+");
        cpssOsPrintf("\n");
    }

    *tileNumPtr = tileNum;
    *dpNumPtr = dpNum;
    *localdpPortNumPtr = localdpPortNum;

    return GT_OK;


}

/**
* @internal  prvCpssFalconTxqTailDropPoolMappingValidityCheck  function
* @endinternal
*
* @brief   Validate tc4Pfc  SW shadow.Check that  <pool id>  field from EGF_QAG_TC_DP_MAPPER table
*      is synced to pfcc tc to pool vector.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqTailDropPoolMappingValidityCheck
(
    IN  GT_U8                   devNum
)
{
    GT_U32 poolId;
    GT_U32 tc4Pfc;
    GT_STATUS rc;
    GT_U32 i,value;
    GT_U32 pfccTcMapping,pfccPoolId,tmp;
    GT_BOOL headRoom;


    rc = prvCpssFalconTxqPfccTcMapVectorGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,&pfccTcMapping);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcMapVectorGet failed  ");
    }

    tmp=pfccTcMapping;

    for(i=0;i<4;i++)
    {
       cpssOsPrintf("PFC mapping POOL_%d vector 0x%x\n",i,tmp&0xFF);
       tmp>>=8;
    }

    for(i=0;i<_4K;i++)
    {
        rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E,i,&value);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChReadTableEntry EGF_QAG_TC_DP_MAPPER  failed  for  index %d  ",i);
        }

        tc4Pfc = U32_GET_FIELD_MAC(value,6,3);
        poolId = U32_GET_FIELD_MAC(value,9,1);

        if(pfccTcMapping&(1<<tc4Pfc))
        {
            pfccPoolId =0;
        }
        else if(pfccTcMapping&(1<<(tc4Pfc+8)))
        {
            pfccPoolId =1;
        }
        else if(pfccTcMapping&(1<<(tc4Pfc+16)))
        {
            pfccPoolId =2;
        }
        else if(pfccTcMapping&(1<<(tc4Pfc+24)))
        {
            pfccPoolId =3;
        }
        else
        {
            pfccPoolId =4;
        }

        if(pfccPoolId>0)
        {
            pfccPoolId =1;
        }

        if(poolId!=pfccPoolId)
        {
            cpssOsPrintf("FAIL for index %d for Vector 0x%x tc4Pfc %d poolId %d\n",i,pfccTcMapping,tc4Pfc,poolId);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqTailDropPoolMappingValidityCheck failed  for  index %d  ",i);
        }
    }


    /*Check  coherency of TC_to_pool_CFG and pool_CFG*/
    for(i=0;i<8;i++)
    {
        rc =prvCpssFalconTxqPfccTcResourceModeGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,(GT_U8)i,&tmp,&headRoom);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcMapVectorGet failed  ");
        }

        if(pfccTcMapping&(1<<i))
        {
            pfccPoolId =0;
        }
        else if(pfccTcMapping&(1<<(i+8)))
        {
            pfccPoolId =1;
        }
        else if(pfccTcMapping&(1<<(i+16)))
        {
            pfccPoolId =2;
        }
        else if(pfccTcMapping&(1<<(i+24)))
        {
            pfccPoolId =3;
        }
        else
        {
            pfccPoolId =4;
        }

        if(tmp!=PRV_TC_MAP_PB_MAC)
        {
            cpssOsPrintf("TC %d DBA POOL %d  PFC MAP %d HR %d\n",i,tmp,pfccPoolId,headRoom);

            if(tmp!=pfccPoolId)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " Coherency of TC_to_pool_CFG and pool_CFG failed for tc %d ",i);
            }
        }
        else
        {
            cpssOsPrintf("TC %d DBA POOL PB PFC MAP %d HR %d\n",i,pfccPoolId,headRoom);
        }
    }

    tmp =0;
    /*Check that all Tcs are mapped*/
    for(i=0;i<4;i++)
    {
        tmp |= (pfccTcMapping&0xFF);
        pfccTcMapping>>=8;
    }

    if(tmp!=0xFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "pfccTcMapping vector validation failed.Unmapped TC found 0x%x",tmp);
    }

    cpssOsPrintf("ALL PASS  \n");

    return GT_OK;
}

/**
* @internal  prvCpssFalconTxqDebugDumpPort  function
* @endinternal
*
* @brief   Debug function for port diagnostic
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
* @param[in] portNum               physical number.
* @param[in] debugMode         desired debug information bitmap
*                                                     [0x1 -mapping ,0x2 - status ,0x3 shaping configuration ,0x4 scheduling configuration]
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugDumpPort
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  debugMode
)
{
    GT_STATUS rc;
    GT_U32    tileNum;
    GT_U32    dpNum;
    GT_U32    localPortNum,dpQueueOffset;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr=NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pnodePtr;
    GT_U32    queueStart,queueEnd,i;
    GT_U32    activeSliceMap[40];
    GT_BOOL   sliceValid[40];
    GT_U32    activeSliceNum;
    GT_U32    pNodeIndex,txDma,aNodeListSize,pNodeNum;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE range;
    GT_BOOL isCascadePort = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort);
    if(rc != GT_OK)
    {
         return rc;
    }

    if(GT_FALSE == isCascadePort)
    {
        rc =prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber failed  ");
        }
     }

    rc =prvCpssFalconTxqDebugDumpPortMapping(devNum,portNum,(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_MAPPING_CNS)?GT_TRUE:GT_FALSE,isCascadePort,
        &tileNum,&dpNum,&localPortNum);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqDebugDumpPortMapping failed  ");
    }
    rc = prvCpssFalconPsiMapPortToPdqQueuesGet(devNum,tileNum,dpNum,localPortNum,&dpQueueOffset);
    if(rc!=GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    }
    if(GT_FALSE == isCascadePort)
    {
        if(aNodePtr==NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "NULL pointer\n");
        }
        queueStart = aNodePtr->queuesData.pdqQueueFirst- dpQueueOffset;
        queueEnd   = aNodePtr->queuesData.pdqQueueLast- dpQueueOffset;
    }
    else
    {
        pNodeNum = dpNum + (MAX_DP_IN_TILE(devNum))*localPortNum;
        tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[ tileNum]);
        pnodePtr = &(tileConfigsPtr->mapping.pNodeMappingConfiguration[pNodeNum]);

        queueStart = pnodePtr->aNodelist[0].queuesData.queueBase;
        aNodeListSize = pnodePtr->aNodeListSize;
        queueEnd = queueStart+(pnodePtr->aNodelist[aNodeListSize-1].queuesData.pdqQueueLast -pnodePtr->aNodelist[0].queuesData.pdqQueueFirst);

    }
            /*Mapping*/
    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_MAPPING_CNS)
    {

        cpssOsPrintf("\n");
        rc = prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump(devNum,tileNum,portNum,portNum);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump failed  ");
        }
        cpssOsPrintf("\n");




        cpssOsPrintf("\nPSI configurations\n");
        cpssOsPrintf("\n+-----+--------------+-----------------+---------------------+--------------------+");
        cpssOsPrintf("\n| SDQ | localPortNum | DP queue offset |Internal offset start|Internal offset stop|");
        cpssOsPrintf("\n+-----+--------------+-----------------+---------------------+--------------------+");
        cpssOsPrintf("\n|%5d|%14d|%17d|%20d|%21d|",dpNum,localPortNum,dpQueueOffset,queueStart,queueEnd);

        cpssOsPrintf("\n+-----+--------------+-----------------+---------------------+--------------------+\n");

        rc = prvCpssFalconTxqUtilsSdqPortRangeDump(devNum,tileNum,dpNum,GT_TRUE,localPortNum);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        rc = prvCpssFalconTxqSdqSelectListGet(devNum,tileNum,dpNum,localPortNum,&range);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        cpssOsPrintf("\nSelect List\n");
        cpssOsPrintf("\n+-----+--------------+---------------+---------------+----------------+-------------+");
        cpssOsPrintf("\n| SDQ | localPortNum | prio0HighLimit| prio0LowLimit | prio1HighLimit |prio1LowLimit|");
        cpssOsPrintf("\n+-----+--------------+---------------+---------------+----------------+-------------+");
        cpssOsPrintf("\n|%5d|%14d|%15d|%15d|%16d|%13d|",dpNum,localPortNum,range.prio0HighLimit,range.prio0LowLimit,
            range.prio1HighLimit,range.prio1LowLimit);
        cpssOsPrintf("\n+-----+--------------+---------------+---------------+----------------+-------------+\n");


        rc =prvCpssFalconTxqUtilsSdqQueueAttributesDump(devNum,tileNum,dpNum,queueStart,
            queueEnd-queueStart+1);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        cpssOsPrintf("\nQFC configurations\n");
        rc =prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping(devNum,tileNum,dpNum,localPortNum,localPortNum,&txDma);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_STATUS_CNS)
    {
        /*Queue status*/

        rc = prvCpssFalconTxqSdqQueueStatusDump(devNum,tileNum,dpNum,queueStart,
            queueEnd-queueStart+1);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc =prvCpssDxChTxqFalconDescCounterDump(devNum,tileNum,dpNum,queueStart,queueEnd);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        cpssOsPrintf("\nPDX Pizza arbiter\n");

        cpssOsPrintf("\n+----+-----+-------+-----+----------+");
        cpssOsPrintf("\n|PDX | PDS | slice |valid|assignment|");
        cpssOsPrintf("\n+----+-----+-------+-----+----------+");

        rc = prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet(devNum,tileNum,dpNum,&activeSliceNum,activeSliceMap,sliceValid);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        for(i=0;i<activeSliceNum;i++)
        {
            cpssOsPrintf("\n|%4d|%5d|%7d|%5s|%10c|",tileNum,dpNum,i,(sliceValid[i]==GT_TRUE)?"true":"false",(sliceValid[i]==GT_TRUE)?'0'+activeSliceMap[i]:' ');
        }

        cpssOsPrintf("\n+----+-----+-------+-----+----------+");

        if (prvCpssDxChPortRemotePortCheck(devNum, portNum) == GT_FALSE)
        {
            /*Flow control*/
            rc = prvCpssDxChTxqDebugDumpFlowControlConfig(devNum,portNum);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }
       }
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_SHAPING_CNS)
    {
        /*no shaping for cascade*/
        if(GT_FALSE == isCascadePort)
        {

            /*Shaping*/
            rc = prvCpssDxChTxqDumpAnodeShapingParameters(devNum,tileNum,aNodePtr->aNodeIndex,aNodePtr->aNodeIndex);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }

            rc = prvCpssDxChTxqDumpQueueShapingParameters(devNum,tileNum,aNodePtr->queuesData.pdqQueueFirst,
                aNodePtr->queuesData.pdqQueueLast);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }
        }
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_SCHED_CNS)
    {
        /*Scheduling*/
        pNodeIndex = prvCpssFalconTxqUtilsGetPnodeIndex(devNum,dpNum,localPortNum);

        cpssOsPrintf("\nDump scheduling PDQ configurations\n");
        rc = prvCpssFalconTxqPdqDumpPortHw(devNum,tileNum,pNodeIndex);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }
    }

    return GT_OK;
}

