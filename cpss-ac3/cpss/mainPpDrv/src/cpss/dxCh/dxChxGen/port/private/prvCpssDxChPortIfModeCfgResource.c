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
* @file prvCpssDxChPortIfModeCfgResource.c
*
* @brief CPSS implementation for port resource configuration.
*
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlue.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>

#define INVALID_PORT_CNS  0xFFFF
/**
* @internal prvCpssDxChPortMappingCPUPortGet function
* @endinternal
*
* @brief   Function checks and returns the number of SDMA CPU ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuPortNumPtr            -  array with the physical port bumbers of the SDMA CPU ports
* @param[out] numOfCpuPorts            - (pointer to) the number of SDMA CPU ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMappingCPUPortGet
(
     IN  GT_U8                devNum,
     OUT GT_PHYSICAL_PORT_NUM *cpuPortNumPtr, /*array of size CPSS_MAX_CPU_PORTS_CNS*/
     OUT GT_U32               *numOfCpuPorts
)
{
    GT_STATUS rc;
    GT_U32 maxPortNum;
    GT_U32 index;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cpuPortNumPtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *numOfCpuPorts = 1;
        cpuPortNumPtr[0] = CPSS_CPU_PORT_NUM_CNS;
        return GT_OK;

    }
    maxPortNum = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    index = 0;

    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        /* find local physical port number of CPU port */
        rc =  prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_TRUE == portMapShadowPtr->valid)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                cpuPortNumPtr[index] = portNum;
                index++;
            }
        }
    }
    *numOfCpuPorts = index;
    return GT_OK;
}



 /*
//    +------------------------------------------------------------------------------------------------------------------------------+
//    |                             Port resources                                                                                   |
//    +----+------+----------+-------+--------------+--------------------+-----+-----------------------------+-----------+-----------+
//    |    |      |          |       |              |                    |RXDMA|      TXDMA SCDMA            | TX-FIFO   |Eth-TX-FIFO|
//    |    |      |          |       |              |                    |-----|-----+-----+-----+-----------|-----+-----|-----+-----|
//    | #  | Port | map type | Speed |    IF        | rxdma txq txdma tm |  IF | TxQ |Burst|Burst| TX-FIFO   | Out | Pay | Out | Pay |
//    |    |      |          |       |              |                    |Width|Descr| All | Full|-----+-----|Going| Load|Going| Load|
//    |    |      |          |       |              |                    |     |     | Most|Thrsh| Hdr | Pay | Bus |Thrsh| Bus |Thrsh|
//    |    |      |          |       |              |                    |     |     | Full|     |Thrsh|Load |Width|     |Width|     |
//    |    |      |          |       |              |                    |     |     |Thrsh|     |     |Thrsh|     |     |     |     |
//    +----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//    | 53 |   63 | CPU-SDMA | 1G    | ------------ |    72  63    72 -1 | 64b |   2 |   2 |   2 |   2 |   4 |  1B |   2 |  -- |  -- |  As 1G
//    | 53 |   63 | CPU-SDMA | 1G    | ------------ |    72  63    72 -1 | 64b |   2 |   0 |   0 |   2 |   4 |  8B |   1 |  -- |  -- | Default
//    +----+------+----------+-------+--------------+--------------------+-----+-----------------------------+-----------+-----------+

CPU  TxFIFO out-going-bus (DMA 72) shall be configured as 8byte

Regarding the TxDMA “Burst Almost Full Thr” and “Burst Full Thr” the configuration is irrelevant if we aren’t enabling the feature
(/Cider/EBU/BobK/BobK {Current}/Switching Core/TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 1/ Enable Burst Limit SCDMA %p).
*/
typedef GT_STATUS (*prvCpssDxChBcat2PortResourcesInit_FUN)
(
    IN    GT_U8                   devNum
);

typedef GT_STATUS (*prvCpssDxChBcat2PortResourcesConfig_FUN)
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         allocate
);

typedef struct
{
    CPSS_PP_FAMILY_TYPE_ENT                  devFamily;
    CPSS_PP_SUB_FAMILY_TYPE_ENT              devSubFamily;
    prvCpssDxChBcat2PortResourcesInit_FUN    initFun;
    prvCpssDxChBcat2PortResourcesConfig_FUN  configFun;
}PRV_CPSS_DXCH_RESOURCE_INIT_STC;


/* this function created just to have single IF with BC2 in table */
GT_STATUS prvCpssDxChCaelumPortResourcesConfig_Dummy
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         allocate
)
{
    allocate = allocate;
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChTxPortSpeedPizzaResourcesSet(devNum, portNum, ifMode, speed, GT_TRUE/* config*/);
    }
    return prvCpssDxChCaelumPortResourcesConfig(devNum, portNum, ifMode, speed);
}



PRV_CPSS_DXCH_RESOURCE_INIT_STC prv_devResourcesInitList[] =
{
      {CPSS_PP_FAMILY_DXCH_BOBCAT2_E,  CPSS_PP_SUB_FAMILY_NONE_E,          prvCpssDxChBcat2PortResourcesInit,   prvCpssDxChBcat2PortResourcesConfig        }
     ,{CPSS_PP_FAMILY_DXCH_BOBCAT2_E,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,  prvCpssDxChCaelumPortResourcesInit,  prvCpssDxChCaelumPortResourcesConfig_Dummy }
     ,{CPSS_PP_FAMILY_DXCH_ALDRIN_E,   CPSS_PP_SUB_FAMILY_NONE_E,          prvCpssDxChCaelumPortResourcesInit,  prvCpssDxChCaelumPortResourcesConfig_Dummy }
     ,{CPSS_PP_FAMILY_DXCH_AC3X_E,     CPSS_PP_SUB_FAMILY_NONE_E,          prvCpssDxChCaelumPortResourcesInit,  prvCpssDxChCaelumPortResourcesConfig_Dummy }
     ,{CPSS_PP_FAMILY_DXCH_BOBCAT3_E,  CPSS_PP_SUB_FAMILY_NONE_E,          prvCpssDxChCaelumPortResourcesInit,  prvCpssDxChCaelumPortResourcesConfig_Dummy }
     ,{CPSS_PP_FAMILY_DXCH_ALDRIN2_E,  CPSS_PP_SUB_FAMILY_NONE_E,          prvCpssDxChCaelumPortResourcesInit,  prvCpssDxChCaelumPortResourcesConfig_Dummy }
     ,{CPSS_PP_FAMILY_DXCH_FALCON_E,   CPSS_PP_SUB_FAMILY_NONE_E,          prvCpssDxChFalconPortResourcesInit,  prvCpssDxChCaelumPortResourcesConfig_Dummy }
     ,{CPSS_MAX_FAMILY,                CPSS_BAD_SUB_FAMILY,                NULL,                                NULL                                       }
};


/**
* @internal prvCpssDxChPortResourcesInit function
* @endinternal
*
* @brief   Initialize data structure for port resource allocation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesInit
(
    IN  GT_U8   devNum
)
{
    GT_STATUS rc = GT_OK; /*return code*/
    GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32 i, numOfCpuPorts;
    CPSS_PORT_SPEED_ENT cpuSpeed;
    PRV_CPSS_DXCH_RESOURCE_INIT_STC *devResourcesInitListPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /* --------------------------------------*/
    /* search for appropriate device device  */
    /* --------------------------------------*/
    for (i = 0, devResourcesInitListPtr = NULL; prv_devResourcesInitList[i].devFamily != CPSS_MAX_FAMILY; i++)
    {
        if (prv_devResourcesInitList[i].devFamily    == PRV_CPSS_PP_MAC(devNum)->devFamily &&
            prv_devResourcesInitList[i].devSubFamily ==  PRV_CPSS_PP_MAC(devNum)->devSubFamily)
        {
            devResourcesInitListPtr = &prv_devResourcesInitList[i];
            break;
        }
    }
    if (devResourcesInitListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    /* --------------------------------------*/
    /* process with device                   */
    /* --------------------------------------*/
    rc = devResourcesInitListPtr->initFun(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChPortMappingCPUPortGet(devNum,/*OUT*/cpuPortNumArr,&numOfCpuPorts);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0; i < numOfCpuPorts; i++)
    {

         rc = prvCpssDxChPortDynamicPizzaArbiterWScpuPortSpeedGet(devNum,/*OUT*/&cpuSpeed);
         if (rc != GT_OK)
         {
             return rc;
         }
         /* configure cpu port resources only if it mapped. It should be done also in Fast Boot*/
         /* cause cpu port resources could be not part of eeprom */
         rc = devResourcesInitListPtr->configFun(devNum, cpuPortNumArr[i],
                                                    CPSS_PORT_INTERFACE_MODE_NA_E,
                                                    cpuSpeed, GT_TRUE);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
         }
    }
    return GT_OK;
}


GT_BOOL g_doNotCheckCredits = GT_FALSE;

/*--------------------------------------------------------------------
 *
 *--------------------------------------------------------------------
 */
GT_STATUS prvCpssDxChPortBcat2CreditsCheckSet(GT_BOOL val)
{
    switch (val)
    {
        case GT_FALSE:
            g_doNotCheckCredits = GT_TRUE;
        break;
        default:
        {
            g_doNotCheckCredits = GT_FALSE;
        }
    }
    cpssOsPrintf("\nCredits control = %d\n",!g_doNotCheckCredits);
    return GT_OK;
}


static PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  * prvCpssDxChPortResourcesConfigDbGet
(
    GT_U8 devNum
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    groupResorcesStatusPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus);
    return groupResorcesStatusPtr;
}

/**
* @internal prvCpssDxChPortResourcesConfigDbInit function
* @endinternal
*
* @brief   Resource data structure initialization
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] maxDescCredits           - physical device number
* @param[in] maxHeaderCredits         - physical device number
* @param[in] maxPayloadCredits        - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbInit
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  maxDescCredits,
    IN  GT_U32  maxHeaderCredits,
    IN  GT_U32  maxPayloadCredits
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath)
    {
        if(dpIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if (dpIndex >= MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);

    groupResorcesStatusPtr->usedDescCredits[dpIndex]         = 0;
    groupResorcesStatusPtr->maxDescCredits[dpIndex]          = maxDescCredits;
    groupResorcesStatusPtr->usedHeaderCredits[dpIndex]       = 0;
    groupResorcesStatusPtr->maxHeaderCredits[dpIndex]        = maxHeaderCredits;
    groupResorcesStatusPtr->usedPayloadCredits[dpIndex]      = 0;
    groupResorcesStatusPtr->maxPayloadCredits[dpIndex]       = maxPayloadCredits;
    groupResorcesStatusPtr->trafficManagerCumBWMbps          = 0;
    groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex] = 0;
    groupResorcesStatusPtr->coreOverallSpeedSummaryTemp[dpIndex] = 0;

    return GT_OK;
}


/**
* @internal prvCpssDxChPortResourcesConfigDbAvailabilityCheck function
* @endinternal
*
* @brief   Resource data structure limitations checks
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*                                      txQDescCredits      - physical device number
* @param[in] txFifoHeaderCredits      - physical device number
* @param[in] txFifoPayloadCredits     - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbAvailabilityCheck
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    if (dpIndex >= MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);

    if (g_doNotCheckCredits == GT_FALSE)  /* check credits */
    {
        if (txQDescrCredits + groupResorcesStatusPtr->usedDescCredits[dpIndex] > groupResorcesStatusPtr->maxDescCredits[dpIndex])
        {
            cpssOsPrintf("\n--> ERROR : TXQ credit: number (%d) of credit exceeds limit (%d)\n",
                                      groupResorcesStatusPtr->usedDescCredits[dpIndex] + txQDescrCredits,
                                      groupResorcesStatusPtr->maxDescCredits[dpIndex]);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (txFifoHeaderCredits + groupResorcesStatusPtr->usedHeaderCredits[dpIndex] > groupResorcesStatusPtr->maxHeaderCredits[dpIndex])
        {
            cpssOsPrintf("\n--> ERROR : TxFIFO headers credits: number (%d) of credit exceeds limit (%d)\n",
                                    groupResorcesStatusPtr->usedHeaderCredits[dpIndex] + txFifoHeaderCredits,
                                    groupResorcesStatusPtr->maxHeaderCredits[dpIndex]);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (txFifoPayloadCredits + groupResorcesStatusPtr->usedPayloadCredits[dpIndex] > groupResorcesStatusPtr->maxPayloadCredits[dpIndex])
        {
            cpssOsPrintf("\n--> ERROR : TxFIFO payload credits: number (%d) of credit exceeds limit (%d)\n",
                                    groupResorcesStatusPtr->usedPayloadCredits[dpIndex] + txFifoPayloadCredits,
                                    groupResorcesStatusPtr->maxPayloadCredits[dpIndex]);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChPortResourcesConfigDbAdd function
* @endinternal
*
* @brief   Resource data structure DB add operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      txQDescCredits      - descriptor credit
* @param[in] txFifoHeaderCredits      - header credit
* @param[in] txFifoPayloadCredits     - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbAdd
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    if (dpIndex >= MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);

    groupResorcesStatusPtr->usedDescCredits        [dpIndex] += txQDescrCredits;
    groupResorcesStatusPtr->usedHeaderCredits      [dpIndex] += txFifoHeaderCredits;
    groupResorcesStatusPtr->usedPayloadCredits     [dpIndex] += txFifoPayloadCredits;

    return GT_OK;
}


/**
* @internal prvCpssDxChPortResourcesConfigDbDelete function
* @endinternal
*
* @brief   Resource data structure DB delete operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      txQDescCredits      - descriptor credit
* @param[in] txFifoHeaderCredits      - header credit
* @param[in] txFifoPayloadCredits     - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbDelete
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    if (dpIndex >= MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);

    groupResorcesStatusPtr->usedDescCredits        [dpIndex] -= txQDescrCredits;
    groupResorcesStatusPtr->usedHeaderCredits      [dpIndex] -= txFifoHeaderCredits;
    groupResorcesStatusPtr->usedPayloadCredits     [dpIndex] -= txFifoPayloadCredits;

    return GT_OK;
}


GT_STATUS prvCpssDxChPortResourcesConfigDbCoreOverallSpeedAdd
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  bandwidthMbps
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    if (dpIndex >= MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);
    groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex] += bandwidthMbps;
    return GT_OK;
}

GT_STATUS prvCpssDxChPortResourcesConfigDbCoreOverallSpeedDelete
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  bandwidthMbps
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    if (dpIndex >= MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);

    if (groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex] < bandwidthMbps)
    {
        #ifdef GM_USED
            /* for GM_USED : for some reason the prvCpssDxChPortCaelumModeSpeedSet skips calling to :
                rc = prvCpssDxChCaelumPortResourcesConfig(devNum, portNum, ifMode, speed);
                ...
                so the prvCpssDxChPortResourcesConfigDbCoreOverallSpeedAdd is not called for all ports
                (and called only for 'CPU port')

                ... but the 'remove' is called also for the GM_USED .
            */

            /* do not fail the operation ! */
            return GT_OK;
        #else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        #endif
    }
    groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex] -= bandwidthMbps;
    return GT_OK;

}


GT_STATUS prvCpssDxChPortResourcesConfigDbTMBWSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps,
    IN GT_U32   dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits

)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    if (dpIndex >= MAX_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);

    groupResorcesStatusPtr->trafficManagerCumBWMbps = tmBandwidthMbps;
    groupResorcesStatusPtr->usedDescCredits[dpIndex]    += txQDescrCredits;
    groupResorcesStatusPtr->usedHeaderCredits[dpIndex]  += txFifoHeaderCredits;
    groupResorcesStatusPtr->usedPayloadCredits[dpIndex] += txFifoPayloadCredits;
    return GT_OK;
}

GT_STATUS prvCpssDxChPortResourcesConfigDbTMBWGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  *tmBandwidthMbpsPtr
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC  *groupResorcesStatusPtr;

    groupResorcesStatusPtr = prvCpssDxChPortResourcesConfigDbGet(devNum);

    CPSS_NULL_PTR_CHECK_MAC(tmBandwidthMbpsPtr);
    *tmBandwidthMbpsPtr = groupResorcesStatusPtr->trafficManagerCumBWMbps;

    return GT_OK;
}

GT_STATUS prvCpssDxChPort_Bcat2A0_TM_ResourcesConfig
(
    IN GT_U8       devNum
)
{
    GT_STATUS   rc;
    GT_U32      regAddr, value;
    GT_U32      fieldOffset;
    GT_U32      fieldSize;

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
       (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E))
    {
        /* nothing to do, there is no static init configuration for TM,
           it will be configured dynamically per add/delete port */

        return GT_OK;
    }

    /* TM port 73 Outgoing Bus %p Width and Shifter %p Threshold */

    if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum) == GT_FALSE)
    {
        value = 5;
    }
    else
    {
        value = 4;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TX_FIFO_MAC(devNum).
                                txFIFOShiftersConfig.SCDMAShiftersConf[73];
    rc = prvCpssHwPpPortGroupSetRegField(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            regAddr, 0, 6,
                                            /* 64B and port speed >= 100G */
                                            (6 | (value << 3)));
    if(rc != GT_OK)
    {
        return rc;
    }

    /* 'scdma_73_payload_threshold' */
    regAddr = PRV_DXCH_REG1_UNIT_TX_FIFO_MAC(devNum).
                                txFIFOGlobalConfig.SCDMAPayloadThreshold[73];
    rc = prvCpssHwPpPortGroupSetRegField(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            regAddr, 0, 7, 0x50);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum) == GT_FALSE)
    {
        fieldOffset = 16;
        fieldSize = 14;
        value = (0<<4) | 0xa;
    }
    else
    {
        fieldOffset = 16;
        fieldSize = 13;
        value = (0<<3) | 0x6;
    }

    /* 'scdma_73_speed', 'rate_limit_threshold_scdma73'  */
    /* rate limit was 4 but is changed to 0 */
    regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).
                                        txDMAPerSCDMAConfigs.SCDMAConfigs[73];
    rc = prvCpssHwPpPortGroupSetRegField(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                regAddr, fieldOffset, fieldSize, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum) == GT_FALSE)
    {
        fieldOffset = 0;
        fieldSize = 9;
    }
    else
    {
        fieldOffset = 17;
        fieldSize = 8;
    }

    /* 'desc_credits_scdma73' */
    regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).
                    txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1[73];
    rc = prvCpssHwPpPortGroupSetRegField(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                regAddr, fieldOffset, fieldSize, 0x3b);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* 'txfifo_payload_counter_threshold_scdma73', 'txfifo_header_counter_threshold_scdma73' */
    regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).
                    txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[73];
    rc = prvCpssHwPpPortGroupSetRegField(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            regAddr, 0, 20, ((0xfa << 10) | 0xfa));
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum) == GT_FALSE)
    {
        fieldOffset = 16;
        fieldSize = 16;
    }
    else
    {
        fieldOffset = 15;
        fieldSize = 16;
    }

    /* 'burst_almost_full_threshold_scdma_73' */
    regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).
                    txDMAPerSCDMAConfigs.burstLimiterSCDMA[73];
    rc = prvCpssHwPpPortGroupSetRegField(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        /* Burst Almost Full Threshold + enable burst limit */
                                            regAddr, fieldOffset, fieldSize, 0x001f);
    return rc;
}



/**
* @internal prvCpssDxChPortApHighLevelConfigurationSet function
* @endinternal
*
* @brief   Allocate/release high level port data path resources (pizza etc.) for
*         port where AP enabled, while physical port interface configuration made
*         by HWS in co-processor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portCreate               - GT_TRUE - port create;
*                                      GT_FALSE - port delete;
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortApHighLevelConfigurationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         portCreate,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc; /* return code */
    GT_U32    portMacMap; /* number of mac mapped to this physical port */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    CPSS_PORT_INTERFACE_MODE_ENT    *portIfModePtr;
    CPSS_PORT_SPEED_ENT             *portSpeedPtr;
    GT_BOOL                         isCgUnitInUse;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacMap);

    portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
    portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));

    isCgUnitInUse = GT_FALSE;
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChIsCgUnitInUse(devNum, portNum, ifMode, speed, &isCgUnitInUse);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    if(!portCreate)
    {
        if(CPSS_PP_FAMILY_DXCH_XCAT3_E != PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            if((CPSS_PP_FAMILY_DXCH_BOBCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
               && (CPSS_PP_SUB_FAMILY_NONE_E == PRV_CPSS_PP_MAC(devNum)->devSubFamily))
            {
                rc = prvCpssDxChBcat2PortResourcesConfig(devNum, portNum, ifMode, speed, GT_FALSE);
            }
            else
            {
                rc = prvCpssDxChCaelumPortResourcesRelease(devNum, portNum);
            }
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Set PTP interface width */
            rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum,CPSS_PORT_SPEED_10_E);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChPortPizzaArbiterIfDelete(devNum, portNum);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChPortBcat2InterlakenTxDmaEnableSet(devNum, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portType = PRV_CPSS_PORT_XG_E;

            if(isCgUnitInUse == GT_TRUE)
            {
                /* update port DB with the default port type value in order to
                    prevent using CG MAC when it is disabled  */
                /* init CG port register database */
                rc = prvCpssDxChCgPortDbInvalidate(devNum, portMacMap, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCgPortDbInvalidate");
                }
            }
        }

        /* save new interface mode in DB */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        return GT_OK;
    }

    if(CPSS_PP_FAMILY_DXCH_XCAT3_E != PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        if(isCgUnitInUse == GT_TRUE)
        {
            /* init CG port register database */
            rc = prvCpssDxChCgPortDbInvalidate(devNum, portMacMap, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        rc = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, portNum,
            ((speed < CPSS_PORT_SPEED_1000_E) ? CPSS_PORT_SPEED_1000_E : speed));
        if (rc != GT_OK)
        {
            return rc;
        }

        if((CPSS_PP_FAMILY_DXCH_BOBCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
           && (CPSS_PP_SUB_FAMILY_NONE_E == PRV_CPSS_PP_MAC(devNum)->devSubFamily))
        {
            rc = prvCpssDxChBcat2PortResourcesConfig(devNum, portNum, ifMode, speed, GT_TRUE);
        }
        else
        {
            rc = prvCpssDxChCaelumPortResourcesConfig(devNum, portNum, ifMode, speed);
        }
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set FCA interface width */
        rc = prvCpssDxChPortBcat2FcaBusWidthSet(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set PTP interface width */
        rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(portMapShadowPtr->portMap.trafficManagerEn)
        {
            /* Port speed calibration value for TM Flow Control */
            #if defined (INCLUDE_TM)
                rc = prvCpssDxChTmGlueFlowControlPortSpeedSet(devNum,
                                                  portMapShadowPtr->portMap.tmPortIdx,
                                                              speed);
            #else
                rc = GT_NOT_SUPPORTED;
            #endif /*#if defined (INCLUDE_TM)*/
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
        rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, portNum, ifMode);
        if (rc != GT_OK)
        {
            return rc;
        }

#ifndef GM_USED
        /* Configures type of the port connected to the LED */
        rc = prvCpssDxChLedPortTypeConfig(devNum, portNum,portCreate);
        if (rc != GT_OK)
        {
            return rc;
        }
#endif
    }
    else
    {
        rc = prvCpssDxChPortXcat3FcaBusWidthSet(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* GE_ONLY ports can't come here Set Count External FC En enable not needed */

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
        rc = prvCpssDxChHwXcat3RegAddrPortMacUpdate(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* save new interface mode in DB */
    *portIfModePtr = ifMode;
    *portSpeedPtr = speed;

    return GT_OK;
}
/**
* @internal prvCpssDxChPortApPortModeAckSet function
* @endinternal
*
* @brief   Send Ack to Service CPU indicate pizza resources allocate
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*                                      mode       - mode in CPSS format
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS prvCpssDxChPortApPortModeAckSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  phyPortNum; /* port number in local core */
    GT_U32                  portGroup;  /* local core number */
    MV_HWS_PORT_STANDARD    hwPortMode;


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,speed,&hwPortMode);

    if(rc != GT_OK)
    {
        return rc;
    }

    if (hwPortMode == _40GBase_KR)
    {
        hwPortMode = _40GBase_KR4;
    }

    rc = mvHwsApPortCtrlSysAck(devNum,portGroup,phyPortNum,hwPortMode);

    return rc;

}



