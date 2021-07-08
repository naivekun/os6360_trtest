/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/generic/private/utils/prvCpssTimeRtUtils.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>

#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <gtOs/gtOsTimer.h>


#define PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(x) x = x

#define BAD_VALUE (GT_U32)(~0)

GT_STATUS appDemoDxPossiblePhysPortNumGet
(
    IN GT_U8 devNum,
    OUT GT_U32 *portNumPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    return cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E,portNumPtr);
}


GT_CHAR * CPSS_MAPPING_2_STR
(
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm
)
{
    typedef struct
    {
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm;
        GT_CHAR                        *mapStr;
    }APPDEMO_MAPPING_2_STR_STC;


    static APPDEMO_MAPPING_2_STR_STC prv_mappingTypeStr[] =
    {
             { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,           "ETHERNET" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               "CPU-SDMA" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,           "ILKN-CHL" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,   "REMOTE-P" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,                "--------" }
    };
    GT_U32 i;
    for (i = 0 ; prv_mappingTypeStr[i].mapEnm != CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E; i++)
    {
        if (prv_mappingTypeStr[i].mapEnm == mapEnm)
        {
            return prv_mappingTypeStr[i].mapStr;
        }
    }
    return "--------";
}

GT_CHAR * CPSS_SCHEDULER_PROFILE_2_STR
(
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileId
)
{
    static APPDEMO_GT_U32_2_STR_STC prv_profileIdTypeStr[] =
    {
          { CPSS_PORT_TX_SCHEDULER_PROFILE_1_E,   " 1" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_2_E,   " 2" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_3_E,   " 3" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_4_E,   " 4" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_5_E,   " 5" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_6_E,   " 6" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_7_E,   " 7" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_8_E,   " 8" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_9_E,   " 9" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_10_E,  "10" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_11_E,  "11" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_12_E,  "12" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_13_E,  "13" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_14_E,  "14" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_15_E,  "15" }
         ,{ CPSS_PORT_TX_SCHEDULER_PROFILE_16_E,  "16" }
         ,{ BAD_VALUE,                            (GT_CHAR *)NULL }
    };
    return u32_2_STR(profileId,&prv_profileIdTypeStr[0]);
}

/*-------------------------------------------------------------*/

/**
* @internal prvCpssDxChMacUnitsPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChMacUnitsPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* value to write */
    GT_STATUS   rc;         /* return code */
    GT_U32      portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    value = BOOL2BIT_MAC(enable);

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortsEnableDisable function
* @endinternal
*
* @brief   Execute multiple iterations of MAC Units Port Enable toggling.
*
* @param[in] devNum                   - device number
* @param[in] iterationsNum            - number of iterations
* @param[in] sleep                    -   time in milliseconds between iterations. 0 - no sleep
* @param[in] numOfPorts               - quantity of ports to use in iterations
*                                      ... - numbers of ports to configure
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoPortsEnableDisable
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 iterationsNum,
    IN  GT_U32                 sleep,
    IN  GT_U32                 numOfPorts,
    ...
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      i;                  /* interator */
    GT_PHYSICAL_PORT_NUM portNum;   /* port number */
    va_list     ap;                 /* arguments list pointer */
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap; /* port mapping structure */
    GT_U32      iteration;

    va_start(ap, numOfPorts);
    for(i = 1; i <= numOfPorts; i++)
    {
        portNum = va_arg(ap, GT_U32);
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMap);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Wrong devNum [%d] or portNum [%d]\n",devNum, portNum);
            va_end(ap);
            return rc;
        }
        if(portMap.valid != GT_TRUE)
        {
            cpssOsPrintf("Port is not mapped: devNum [%d] portNum [%d]\n",devNum, portNum);
            va_end(ap);
            return GT_BAD_PARAM;
        }

        for (iteration = 0; iteration < iterationsNum; iteration++)
        {
            rc = prvCpssDxChMacUnitsPortEnableSet(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                cpssOsPrintf("prvCpssDxChMacUnitsPortEnableSet Disable: failure devNum [%d] portNum [%d]\n",devNum, portNum);
                va_end(ap);
                return rc;
            }

            if (sleep)
            {
                osTimerWkAfter(sleep);
            }

            rc = prvCpssDxChMacUnitsPortEnableSet(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                cpssOsPrintf("prvCpssDxChMacUnitsPortEnableSet Enable: failure devNum [%d] portNum [%d]\n",devNum, portNum);
                va_end(ap);
                return rc;
            }
        }
    }
    va_end(ap);

    return GT_OK;
}


GT_STATUS asicUserScenRegListPrint
(
    APPDEMO_UNIT_x_REG_x_VALUE_STC *regListptr
)
{
    GT_U32 rc;
    GT_U8 dev = 0;
    GT_U32 regIdx;
    GT_U32 data;
    static GT_CHAR name[200];
    GT_BOOL tableNameDef = GT_FALSE;
    GT_BOOL regNameDef = GT_FALSE;


    cpssOsPrintf("\n+-----------------------------------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                                           Diff Register dump                                                                            |");
    cpssOsPrintf("\n+------+---------------------------------------------+------------+------------+------------+---------------------------------------------+");
    cpssOsPrintf("\n| xls# |       unitName                              |  regAddr   |   dataHW   |   dataSW   | RegName                                     |");
    cpssOsPrintf("\n+------+---------------------------------------------+------------+------------+----------------------------------------------------------+");

    for (regIdx = 0 ; regListptr[regIdx].regAddr != BAD_VALUE ; regIdx++)
    {
        rc = prvCpssDrvHwPpReadRegister(dev,regListptr[regIdx].regAddr, /*OUT*/&data);
        if (rc != GT_OK)
        {
            return rc;
        }
        tableNameDef = GT_FALSE;
        regNameDef = GT_FALSE;

        if (data != regListptr[regIdx].data)
        {
            if (regListptr[regIdx].regName != NULL)
            {
                if (regListptr[regIdx].regName[0] != ' ')
                {
                    cpssOsSprintf(name,"REG : %s",regListptr[regIdx].regName);
                    tableNameDef = GT_TRUE;
                }
            }
            if (regListptr[regIdx].tableName != NULL)
            {
                if (regListptr[regIdx].tableName[0] != ' ')
                {
                    cpssOsSprintf(name,"TBL : %s",regListptr[regIdx].tableName);
                    regNameDef = GT_TRUE;
                }
            }
            if (tableNameDef == GT_TRUE && regNameDef == GT_TRUE)
            {
                cpssOsSprintf(name,"BOTH are defined : REG %s : TBL : %s",regListptr[regIdx].regName, regListptr[regIdx].tableName);
            }


            cpssOsPrintf("\n| %4d | %43s | 0x%08x | 0x%08x | 0x%08x | %-40s",regListptr[regIdx].idxInExcel
                                                    ,regListptr[regIdx].unitName
                                                    ,regListptr[regIdx].regAddr
                                                    ,regListptr[regIdx].data
                                                    ,data,
                                                    name);
        }
    }
    cpssOsPrintf("\n+-----+---------------------------------------+------------+------------+----------------------------------------------------------+");
    return GT_OK;
}


GT_STATUS appDemoBobKPortPizzaArbiterResourcesUnitStatisticsGet
(
    IN GT_U8 dev,
    IN GT_U32 localNumOnUnit,
    IN CPSS_DXCH_PA_UNIT_ENT unit,
    IN CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr,
    OUT GT_U32 *totalPtr,
    OUT GT_U32 *sliceNptr,
    OUT GT_U32   *minDistPtr,
    OUT GT_U32   *maxDistPtr
)
{
    GT_U32 unitIdx;
    GT_BOOL unitFound;
    GT_U32  i;
    GT_U32  sliceN;
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  *unitStatePtr;

     #define BAD_VALUE (GT_U32)(~0)
    *minDistPtr = BAD_VALUE;
    *maxDistPtr = BAD_VALUE;

    unitFound = GT_FALSE;
    unitStatePtr = NULL;
    for (unitIdx = 0 ; pizzaDeviceStatePtr->devState.bobK.unitList[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        if (pizzaDeviceStatePtr->devState.bobK.unitList[unitIdx] == unit)
        {
            unitFound = GT_TRUE;
            unitStatePtr = &pizzaDeviceStatePtr->devState.bobK.unitState[unitIdx];
            break;
        }
    }
    if (unitFound != GT_TRUE)
    {
        return GT_BAD_PARAM;
    }
    /* count slice N*/
    sliceN = 0;
    for (i = 0 ; i < unitStatePtr->totalConfiguredSlices; i++)
    {
        if (unitStatePtr->slice_enable[i] == GT_TRUE)
        {
            if (unitStatePtr->slice_occupied_by[i] == localNumOnUnit)
            {
                sliceN++;
            }
        }
    }
    *totalPtr  = unitStatePtr->totalConfiguredSlices;
    *sliceNptr = sliceN;

    if (sliceN > 1)
    {
        /* find first slice */
        GT_U32 firstSliceIdx;
        GT_U32 lastSliceIdx;
        GT_U32 dist;
        GT_U32 minDist;
        GT_U32 maxDist;
        for (firstSliceIdx = 0 ; firstSliceIdx < unitStatePtr->totalConfiguredSlices; firstSliceIdx++)
        {
            if (unitStatePtr->slice_enable[firstSliceIdx] == GT_TRUE)
            {
                if (unitStatePtr->slice_occupied_by[firstSliceIdx] == localNumOnUnit)
                {
                    break;
                }
            }
        }
        lastSliceIdx = firstSliceIdx;
        minDist = unitStatePtr->totalConfiguredSlices;
        maxDist = 0;
        for (i = firstSliceIdx+1 ; i < unitStatePtr->totalConfiguredSlices; i++)
        {
            if (unitStatePtr->slice_enable[i] == GT_TRUE)
            {
                if (unitStatePtr->slice_occupied_by[i] == localNumOnUnit)
                {
                    dist = i - lastSliceIdx;
                    if (dist < minDist)
                    {
                        minDist = dist;
                    }
                    if (dist > maxDist)
                    {
                        maxDist = dist;
                    }
                    lastSliceIdx = i;
                }
            }
        }
        *minDistPtr = minDist;
        *maxDistPtr = maxDist;
    }
    return GT_OK;
}



/*----------------------------------------------------------------------------------------------------------------------------------*
 *   MIB
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/GOP/<CG_IP>CG Top/<CG IP REGS 1.0> CG/Units/CG Port %a Pipe %t/MAC/MAC_RegFile/aFramesTransmittedOK
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/GOP/<CG_IP>CG Top/<CG IP REGS 1.0> CG/Units/CG Port %a Pipe %t/MAC/MAC_RegFile/aFramesReceivedOK
 *
 *  TxFIFO
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/<TXFIFO_IP> TxFIFO IP TLU/Units/TxFIFO %a Pipe %b/TxFIFO Debug/Informative Debug/Incoming Headers Counter Lsb
 *
 *  TxDMA
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Debug/Informative Debug/Incoming Descriptors Counter Lsb
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Debug/Informative Debug/Outgoing Descriptors Counter Lsb
 *
 *   TxQ
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/<TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Units/TXQ_IP_queue/Peripheral Access/Egress MIB Counters/Set <%n> Outgoing Unicast Packet Counter
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/<TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Units/TXQ_IP_queue/Peripheral Access/Egress MIB Counters/Set <%n> Tail Dropped Packet Counter
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/<TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Units/TXQ_IP_queue/Peripheral Access/Egress MIB Counters/Set <%n> Bridge Egress Filtered Packet Counter
 *
 *   RxDMA
 *      /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/<RxDMA_IP> RxDMA IP TLU/Units/RxDMA %a Pipe %b/Debug/Counters And Counters Statuses/Counter Total Number Of Outgoing Packets From Rxdma Lsb
 *
 *-------------------------------------------------------------------------------------------------------------------------------------
 *  Input :
 *      port-In
 *      port-loopback
 *
 * --->  Mac-In          ---> RxDma-In  ---> TXQ-lb       --->  TxDMA-Lb     ---> TxFifo-Lb  --> Mac-Lb -----------+
 *      Mib-aFramesRxOK    Cnt-Tx(per DP)  Cnt-Unicast          Cnt-Rx(perDP)     Cnt-Rx    Mib-aFramesTxOK        |
 *                                         Cnt-TailDrop         Cnt-Tx(perDP)                                      |
 *                                         Cnt-Flt                                                                 |
 *                                                                                                                 |
 * <---  Mac-In          <--- TxFifo-In <--- TxDma-in    <---     TxQ-In     <--- RxDma-In  <-- Mac-Lb <-----------+
 *      Mib-aFramesTxOK                     Cnt-Rx(perDP)                        Tx(per DP)     Mib-aFramesRxOK
 *                                          Cnt-Tx(perDP)
 *
 *
 *----------------------------------------------------------------------------------------------------------------------------------------*/
#if 0
typedef struct
{
    GT_U32 mibMac_rx;        /* v */
    GT_U32 cntRxDma_tx;
}PortRxRoad_STC;
typedef struct
{
    GT_U32 cntTxQ_unicast;
    GT_U32 cntTxQ_tailDrop;
    GT_U32 cntTxQ_flt;
    GT_U32 cntTxDma_rx;
    GT_U32 cntTxDma_tx;
    GT_U32 cntTxFifo_rx;
    GT_U32 mibMac_tx;       /* v  */
}PortTxRoad_STC;

typedef struct
{
    PortRxRoad_STC rxRoad;
    PortTxRoad_STC txRoad;
}PortPacketRoad_STC;

GT_VOID prvCpssPortDebug
(
    IN  GT_U8 devNum,
    IN  GT_U32 portNum,
    OUT PortPacketRoad_STC *portCntPtr;
)
{
    GT_STATUS rc;
    GT_U32 regAddr_STATN_CONFIG;
    GT_U32 regAddr_mib_tx;
    GT_U32 regAddr_mib_rx;
    GT_U32 regValue;

    regAddr_STATN_CONFIG = 0x1000 * portNum + 0x10340478;
    regAddr_mib_tx = 0x1000 * portNum + 0x10340480;
    regAddr_mib_rx = 0x1000 * portNum + 0x10340488;

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr_STATN_CONFIG, 0x30);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr_mib_tx, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    portCntPtr->txRoad.mibMac_tx = regValue;


    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr_mib_rx, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    portCntPtr->rxRoad.mibMac_rx = regValue;


    return;

}



GT_VOID prvCpssDebug
(
    GT_U8 devNum,
    GT_U32 portNumIn,
    GT_U32 portNumLb
)
{

    typedef struct
    {
        GT_U32 in_mibMac_rx;
        GT_U32 in_cntRxDma_tx;
        GT_U32 lb_cntTxQ_unicast;
        GT_U32 lb_cntTxQ_tailDrop;
        GT_U32 lb_cntTxQ_flt;
        GT_U32 lb_cntTxDma_rx;
        GT_U32 lb_cntTxDma_tx;
        GT_U32 lb_cntTxFifo_rx;
        GT_U32 lb_mibMac_tx;
        GT_U32 lb_mibMac_rx;
        GT_U32 lb_cntRxDma_tx;
        GT_U32 in_cntTxQ_unicast;
        GT_U32 in_cntTxQ_tailDrop;
        GT_U32 in_cntTxQ_flt;
        GT_U32 in_cntTxDma_rx;
        GT_U32 in_cntTxDma_tx;
        GT_U32 in_cntTxFifo_rx;
        GT_U32 in_mibMac_tx;
    }CollectedData_STC;

    GT_STATUS rc;
    GT_U32 regAddr_STATN_CONFIG;
    GT_U32 regAddr_mib_tx;
    GT_U32 regAddr_mib_rx;
    GT_U32 regValue;

    regAddr_STATN_CONFIG = 0x1000 * portNumIn + 0x10340478;
    regAddr_mib_tx = 0x1000 * portNumIn + 0x10340480;
    regAddr_mib_rx = 0x1000 * portNumIn + 0x10340488;

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr_STATN_CONFIG, 0x30);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr_mib_tx, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    cpssOsPrintf("MIB_TX_PORT_IN_%d = 0x%x\n", portNumIn, regValue);
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr_mib_rx, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    cpssOsPrintf("MIB_RX_PORT_IN_%d = 0x%x\n", portNumIn, regValue);

    /******************************************************************************************************************/

    regAddr_STATN_CONFIG = 0x1000 * portNumLb + 0x10340478;
    regAddr_mib_tx = 0x1000 * portNumLb + 0x10340480;
    regAddr_mib_rx = 0x1000 * portNumLb + 0x10340488;

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr_STATN_CONFIG, 0x30);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr_mib_tx, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    cpssOsPrintf("MIB_TX_PORT_LB_%d = 0x%x\n", portNumLb, regValue);

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr_mib_rx, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("HW access errror\n\n");
    }
    cpssOsPrintf("MIB_RX_PORT_LB_%d = 0x%x\n", portNumLb, regValue);

    /******************************************************************************************************************/

    return;
}

#endif


